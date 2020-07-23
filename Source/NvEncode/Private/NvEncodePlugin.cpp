#include "NvEncodePlugin.h"
#include <Engine/Engine.h>
#include <Engine/TextureRenderTarget2D.h>
#include <Engine/Texture2D.h>

FLogCategoryNvEncodeLog NvEncodeLog;

void NvEncode::LogMessage(const FString& msg) {
	UE_LOG(NvEncodeLog, Log, TEXT("%s"), *msg);
}

void NvEncode::LogMessageOnScreen(const FString& msg) {
	LogMessage(msg);
	if(GEngine != nullptr) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, msg);
}

void NvEncode::LogMessage(const char* msg) {
	UE_LOG(NvEncodeLog, Log, TEXT("%s"), UTF8_TO_TCHAR(msg));
}

void NvEncode::LogMessageOnScreen(const char* msg) {
	LogMessage(msg);
	if(GEngine != nullptr) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, UTF8_TO_TCHAR(msg));
}


NvEncoderUnreal::NvEncoderUnreal(int width_, int height_, int bitrate_) : width(width_), height(height_) {
	if(width < 128 || width > 4096 || (width & (width - 1)) != 0) {
		NvEncode::LogMessageOnScreen(FString(L"Invalid encoder width: ") + FString::FromInt(width));
		return;
	}
	if(height < 128 || height > 4096 || (height & (height - 1)) != 0) {
		NvEncode::LogMessageOnScreen(FString(L"Invalid encoder height: ") + FString::FromInt(height));
		return;
	}

	CUdevice encodeDevice = 0;
	cuInit(0);
	cuDeviceGet(&encodeDevice, 0);
	cuCtxCreate(&_context, 0, encodeDevice);
	try {
		_encoder = new NvEncoderCuda(_context, width, height, NV_ENC_BUFFER_FORMAT_NV12, 0);
	} catch(NVENCException& e) {
		NvEncode::LogMessageOnScreen(("NvEncoderD3D11 creation error: " + e.getErrorString()).c_str());
		return;
	}

	_inBuffer = (uint8_t*)calloc(1, _encoder->GetFrameSize());


	NV_ENC_INITIALIZE_PARAMS initParams = {NV_ENC_INITIALIZE_PARAMS_VER};
	NV_ENC_CONFIG encodeConfig = {NV_ENC_CONFIG_VER};
	initParams.encodeConfig = &encodeConfig;
	_encoder->CreateDefaultEncoderParams(&initParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID);

	encodeConfig.gopLength = NVENC_INFINITE_GOPLENGTH;
	encodeConfig.frameIntervalP = 1;
	encodeConfig.encodeCodecConfig.h264Config.idrPeriod = NVENC_INFINITE_GOPLENGTH;
	encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;

	encodeConfig.rcParams.averageBitRate = bitrate_;
	encodeConfig.rcParams.vbvBufferSize = (encodeConfig.rcParams.averageBitRate * initParams.frameRateDen / initParams.frameRateNum) * 5;
	encodeConfig.rcParams.maxBitRate = encodeConfig.rcParams.averageBitRate;
	encodeConfig.rcParams.vbvInitialDelay = encodeConfig.rcParams.vbvBufferSize;

	_encoder->CreateEncoder(&initParams);
}

NvEncoderUnreal::~NvEncoderUnreal() {
	_input.store(nullptr, std::memory_order_release);
	DestroyEncoder();
}

bool NvEncoderUnreal::SetInputTexture(UTexture2D* input) {
	if(input->GetSizeX() != width || input->GetSizeY() != height) {
		NvEncode::LogMessageOnScreen(L"Invalid input texture dimensions");
		return false;
	}
	return true;
}

bool NvEncoderUnreal::SetInputRenderTarget(UTextureRenderTarget2D* input) {
	if(input->SizeX != width || input->SizeY != height) {
		NvEncode::LogMessageOnScreen(L"Invalid input render target dimensions");
		return false;
	}
	_input.store(input, std::memory_order_relaxed);
	return true;
}

bool NvEncoderUnreal::GetFrame(int64& data, int& size) const {
	int sz = _encodedSize.exchange(0, std::memory_order_acquire);
	if(sz > 0) {
		data = (int64)_encodedData.load(std::memory_order_relaxed);
		size = sz;
	}
	return sz > 0;
}

void NvEncoderUnreal::EncodeFrame() {
	auto* enc = _encoder;
	if(enc == nullptr) return;

	// Copy texture to local and encode frame
	ENQUEUE_RENDER_COMMAND(EncodeFrameRC)([this, enc](FRHICommandListImmediate& RHICmdList) {
		if(!_input.load(std::memory_order_acquire)) return;
		auto _in = ((FTextureRenderTarget2DResource*)_input.load(std::memory_order_relaxed)->GetRenderTargetResource())->GetTextureRHI();
		uint32 Stride = 0;
		uint8_t* TexData = (uint8_t*)RHICmdList.LockTexture2D(_in, 0, RLM_ReadOnly, Stride, false, false);
		//NV12 expected but R8 source, have to pad
		memcpy(_inBuffer, TexData, width * height);
		RHICmdList.UnlockTexture2D(_in, 0, false, false);

		NV_ENC_PIC_PARAMS picParams = {NV_ENC_PIC_PARAMS_VER};
		picParams.encodePicFlags = 0;

		_vPackets.clear();

		const NvEncInputFrame* encInput = enc->GetNextInputFrame();
		enc->CopyToDeviceFrame(_context, (void*)_inBuffer,
							   0,
							   (CUdeviceptr)encInput->inputPtr,
							   encInput->pitch,
							   enc->GetEncodeWidth(),
							   enc->GetEncodeHeight(),
							   CU_MEMORYTYPE_HOST,
							   encInput->bufferFormat,
							   encInput->chromaOffsets,
							   encInput->numChromaPlanes);
		enc->EncodeFrame(_vPackets, &picParams);

		if(_vPackets.size() != 1) {
			NvEncode::LogMessage(L"ERROR: Encoded " + FString::FromInt(_vPackets.size()) + L" packets!");
		} else {
			_encodedData.store((void*)_vPackets[0].data(), std::memory_order_relaxed);
			_encodedSize.store(static_cast<int>(_vPackets[0].size()), std::memory_order_release);
		}
	});
}

void NvEncoderUnreal::DestroyEncoder() {
	if(_encoder != nullptr) {
		_encoder->EndEncode(_vPackets);
	}

	delete _encoder;
	_encoder = nullptr;

	if(_context != nullptr) {
		if(cuCtxDestroy(_context) != CUDA_SUCCESS) {
			NvEncode::LogMessage(L"Could not destroy CUDA context");
		}
		_context = nullptr;
	}
}
