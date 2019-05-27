#include "NvEncodePCH.h"
#include "NvEncodePlugin.h"
#include <Engine.h>

#include <Runtime/RHI/Public/RHICommandList.h>

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

bool UNvEncoder::InitializeEncoder(int width, int height, int bitrate) {
	if(GDynamicRHI->GetName() != FString(L"D3D11")) {
		NvEncode::LogMessageOnScreen(L"NvEncoder only supports D3D11");
		return false;
	}

	_encoder = new NvEncoderCustom(width, height);
	if(!_encoder) {
		NvEncode::LogMessageOnScreen(L"NvEncoderD3D11 creation error");
		return false;
	}

	NV_ENC_INITIALIZE_PARAMS initParams = {NV_ENC_INITIALIZE_PARAMS_VER};
	NV_ENC_CONFIG encodeConfig = {NV_ENC_CONFIG_VER};
	initParams.encodeConfig = &encodeConfig;
	_encoder->CreateDefaultEncoderParams(&initParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID);

	encodeConfig.gopLength = NVENC_INFINITE_GOPLENGTH;
	encodeConfig.frameIntervalP = 1;
	encodeConfig.encodeCodecConfig.h264Config.idrPeriod = NVENC_INFINITE_GOPLENGTH;
	encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;

	encodeConfig.rcParams.averageBitRate = bitrate;
	encodeConfig.rcParams.vbvBufferSize = (encodeConfig.rcParams.averageBitRate * initParams.frameRateDen / initParams.frameRateNum) * 5;
	encodeConfig.rcParams.maxBitRate = encodeConfig.rcParams.averageBitRate;
	encodeConfig.rcParams.vbvInitialDelay = encodeConfig.rcParams.vbvBufferSize;

	_encoder->CreateEncoder(&initParams);

	return true;
}

/*void UNvEncoder::CreateInputTexture() {

}*/

void UNvEncoder::SetInputTexture(UTexture2D* input) {
	_encoder->SetInputTexture((ID3D11Texture2D*)(((FTexture2DResource*)input->Resource)->GetTexture2DRHI().GetReference()));
}

void UNvEncoder::SetInputRenderTarget(UTextureRenderTarget2D* input) {
	_encoder->SetInputTexture((ID3D11Texture2D*)(((FTextureRenderTarget2DResource*)input->GetRenderTargetResource())->GetTextureRHI().GetReference()));
}

void UNvEncoder::EncodeFrame(int64& result, int& size) {

	// Copy texture to local and encode frame
	NV_ENC_PIC_PARAMS picParams = {NV_ENC_PIC_PARAMS_VER};
	picParams.encodePicFlags = 0;

	_vPackets.clear();
	_encoder->EncodeFrame(_vPackets, &picParams);

	if(_vPackets.size() != 1) {
		result = 0;
		size = 0;
	} else {
		result = reinterpret_cast<int64>(_vPackets[0].data());
		size = static_cast<int>(_vPackets[0].size());
	}
}

void UNvEncoder::DestroyEncoder() {
	if(_encoder != nullptr) _encoder->EndEncode(_vPackets);

	delete _encoder;
	_encoder = nullptr;
}
