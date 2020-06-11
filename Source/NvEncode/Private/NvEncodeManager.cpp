#include "NvEncodeManager.h"
#include "NvEncodePlugin.h"

UNvEncodeManager::UNvEncodeManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UNvEncodeManager::SetVideoSource(UNvVideoSource* source) {
	if(VideoSource != nullptr) NvEncode::LogMessageOnScreen("Switching video source");
	VideoSource = source;
	VideoSource->TextureTarget = RenderTarget;
}
