#include "NvEncodePlugin.h"
#include <Engine/Engine.h>

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


UNvEncoder::UNvEncoder(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

bool UNvEncoder::CreateEncoder(int width, int height, int bitrate, UNvEncoder*& encoder) {
	if(width < 128 || width > 4096 || (width & (width - 1)) != 0) {
		NvEncode::LogMessageOnScreen(FString(L"Invalid encoder width: ") + FString::FromInt(width));
		return false;
	}
	if(height < 128 || height > 4096 || (height & (height - 1)) != 0) {
		NvEncode::LogMessageOnScreen(FString(L"Invalid encoder height: ") + FString::FromInt(height));
		return false;
	}

	/*if(GDynamicRHI->GetName() != FString(L"D3D11")) {
		NvEncode::LogMessageOnScreen(L"NvEncoder only supports D3D11");
		return false;
	}*/
	encoder = NewObject<UNvEncoder>();
	encoder->width = width;
	encoder->height = height;

	CUdevice encodeDevice = 0;
	cuInit(0);
	cuDeviceGet(&encodeDevice, 0);
	cuCtxCreate(&encoder->_context, 0, encodeDevice);
	try {
		encoder->_encoder = new NvEncoderCuda(encoder->_context, width, height, NV_ENC_BUFFER_FORMAT_NV12, 0);
	} catch(NVENCException& e) {
		NvEncode::LogMessageOnScreen(("NvEncoderD3D11 creation error: " + e.getErrorString()).c_str());
		return false;
	}

	encoder->_inBuffer = (uint8_t*)calloc(1, encoder->_encoder->GetFrameSize());


	NV_ENC_INITIALIZE_PARAMS initParams = {NV_ENC_INITIALIZE_PARAMS_VER};
	NV_ENC_CONFIG encodeConfig = {NV_ENC_CONFIG_VER};
	initParams.encodeConfig = &encodeConfig;
	encoder->_encoder->CreateDefaultEncoderParams(&initParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID);

	encodeConfig.gopLength = NVENC_INFINITE_GOPLENGTH;
	encodeConfig.frameIntervalP = 1;
	encodeConfig.encodeCodecConfig.h264Config.idrPeriod = NVENC_INFINITE_GOPLENGTH;
	encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;

	encodeConfig.rcParams.averageBitRate = bitrate;
	encodeConfig.rcParams.vbvBufferSize = (encodeConfig.rcParams.averageBitRate * initParams.frameRateDen / initParams.frameRateNum) * 5;
	encodeConfig.rcParams.maxBitRate = encodeConfig.rcParams.averageBitRate;
	encodeConfig.rcParams.vbvInitialDelay = encodeConfig.rcParams.vbvBufferSize;

	encoder->_encoder->CreateEncoder(&initParams);

	/*ENQUEUE_RENDER_COMMAND(CreateEncoderRC)([&enc = encoder->_encoder, width, height, bitrate](FRHICommandListImmediate& RHICmdList) {

		NV_ENC_INITIALIZE_PARAMS initParams = {NV_ENC_INITIALIZE_PARAMS_VER};
		NV_ENC_CONFIG encodeConfig = {NV_ENC_CONFIG_VER};
		initParams.encodeConfig = &encodeConfig;
		enc->CreateDefaultEncoderParams(&initParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID);

		encodeConfig.gopLength = NVENC_INFINITE_GOPLENGTH;
		encodeConfig.frameIntervalP = 1;
		encodeConfig.encodeCodecConfig.h264Config.idrPeriod = NVENC_INFINITE_GOPLENGTH;
		encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;

		encodeConfig.rcParams.averageBitRate = bitrate;
		encodeConfig.rcParams.vbvBufferSize = (encodeConfig.rcParams.averageBitRate * initParams.frameRateDen / initParams.frameRateNum) * 5;
		encodeConfig.rcParams.maxBitRate = encodeConfig.rcParams.averageBitRate;
		encodeConfig.rcParams.vbvInitialDelay = encodeConfig.rcParams.vbvBufferSize;

		enc->CreateEncoder(&initParams);
	});*/

	return true;
}

bool UNvEncoder::SetInputTexture(UTexture2D* input) {
	if(input->GetSizeX() != width || input->GetSizeY() != height) {
		NvEncode::LogMessageOnScreen(L"Invalid input texture dimensions");
		return false;
	}
	/*ENQUEUE_RENDER_COMMAND(SetInputTextureRC)([this, input](FRHICommandListImmediate& RHICmdList) {
		_input = ((FTexture2DResource*)input->Resource)->GetTexture2DRHI();
	});*/
	return true;
}

bool UNvEncoder::SetInputRenderTarget(UTextureRenderTarget2D* input) {
	if(input->SizeX != width || input->SizeY != height) {
		NvEncode::LogMessageOnScreen(L"Invalid input render target dimensions");
		return false;
	}
	_input = input;
	/*ENQUEUE_RENDER_COMMAND(SetInputRenderTargetRC)([this, input](FRHICommandListImmediate& RHICmdList) {
		_input = ((FTextureRenderTarget2DResource*)input->GetRenderTargetResource())->GetTextureRHI();
	});*/
	return true;
}

bool UNvEncoder::EncodeFrame(const FEncodeFinishedDelegate& encodeDone) {
	auto* enc = _encoder;//_encoder.load(std::memory_order_acquire);
	if(enc == nullptr) return false;

	_delegate = encodeDone;

	// Copy texture to local and encode frame
	ENQUEUE_RENDER_COMMAND(EncodeFrameRC)([this, enc](FRHICommandListImmediate& RHICmdList) {
		auto _in = ((FTextureRenderTarget2DResource*)_input->GetRenderTargetResource())->GetTextureRHI();
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

	return true;
}

void UNvEncoder::DestroyEncoder() {
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
