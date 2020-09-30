#include "NvEncodeManager.h"
#include <Engine/World.h>
#include <TimerManager.h>
#include <chrono>

using namespace StarburstGaming;

UNvEncodeManager::UNvEncodeManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UNvEncodeManager::BeginPlay() {
	Super::BeginPlay();
}

void UNvEncodeManager::BeginDestroy() {
	delete _encoder;
	_encoder = nullptr;
	Super::BeginDestroy();
}

void UNvEncodeManager::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	if(_waitingForFrame > 0) {
		_encoder->EncodeFrame();
		_waitingForFrame--;
	}
	int64 data;
	int size;
	if(_encoder && _encoder->GetFrame(data, size)) {
		FSBGBuffer buf(data, size);
		_videoFrameElt->SetBuffer(buf);
		USBGNebulaBP::SendComet(FramesOutTopicName, _videoFrameComet);
		// Start Frame processing time
		_processTimeBuffer[_processTimeIndex] = std::chrono::duration_cast<std::chrono::duration<uint64, std::milli>>(std::chrono::system_clock::now().time_since_epoch()).count();
		_processTimeIndex = _processTimeBuffer.GetNextIndex(_processTimeIndex);
		if(_processTimeIndex == 0) _processTimeBufferValid = true;	// We looped back, the buffer is full
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UNvEncodeManager::Startup(int width, int height, ETextureRenderTargetFormat format) {
	USBGCometBP::LoadComet(VideoFrameCometPath, _videoFrameComet);
	_videoFrameComet->Get("Frame", _videoFrameElt);
	USBGTopicBP::FCometReceivedDelegate OnFrameSlotReadyReceived;
	OnFrameSlotReadyReceived.BindUFunction(this, L"OnFrameSlotReady");
	USBGTopicBP::SubscribeTopic(FrameSlotReadyTopicName, 
								ESBGTopicStorageType::QUEUE, 
								OnFrameSlotReadyReceived, 
								_frameSlotReadyTopic);

	_renderTarget = NewObject<UTextureRenderTarget2D>();
	check(_renderTarget);
	_renderTarget->RenderTargetFormat = format;
	_renderTarget->ClearColor = FLinearColor(0.f, 0.f, 0.f);
	_renderTarget->bAutoGenerateMips = false;
	_renderTarget->InitAutoFormat(width, height);
	_renderTarget->UpdateResourceImmediate(true);

	if(VideoSource) VideoSource->TextureTarget = _renderTarget;

	_encoder = new NvEncoderUnreal(width, height, width * height * 2, format);
	_encoder->SetInputRenderTarget(_renderTarget);

	UWorld* world = GetWorld();
	if(world) {
		auto& tm = world->GetTimerManager();
		tm.SetTimer(_averageRateTimer, this, &UNvEncodeManager::updateAverageFrameRate, 1.0f, true);
		tm.SetTimer(_sendFrameTimer, this, &UNvEncodeManager::SendFrame, 1.f / MaximumFrameRate, true);
	} else {
		NvEncode::LogMessageOnScreen("Cannot get World at Startup");
	}
}

void UNvEncodeManager::OnFrameSlotReady(USBGCometBP* comet) {
	ReadyFrameSlots++;
	// End Frame processing time
}

void UNvEncodeManager::SetVideoSource(UNvVideoSource* source) {
	if(VideoSource != nullptr && VideoSource != source)
		NvEncode::LogMessageOnScreen("Switching video source");
	VideoSource = source;
	VideoSource->TextureTarget = _renderTarget;
}

void UNvEncodeManager::SendFrame() {
	if(ReadyFrameSlots <= 0) return;
	if(!IsValid(VideoSource)) {
		NvEncode::LogMessageOnScreen(L"Cannot send frame: no video source. Set one as default.");
		return;
	}
	VideoSource->CaptureSceneDeferred();
	ReadyFrameSlots--;
	_waitingForFrame++;
}

void UNvEncodeManager::updateAverageFrameRate() {
	UWorld* world = GetWorld();
	checkf(world != nullptr, L"No world!!");
	auto& tm = world->GetTimerManager();

	float cycleTimeMillis = 1000.f / MaximumFrameRate;	// Default is as fast as possible
	float remaining = tm.GetTimerRemaining(_sendFrameTimer);
	if(_processTimeBufferValid) {	// We can smooth the cycle time to spread out frames evenly
		auto oldestTime = _processTimeBuffer[_processTimeIndex];	// Index is one-past-current so oldest is @ index
		auto newestTime = _processTimeBuffer[_processTimeBuffer.GetPreviousIndex(_processTimeIndex)];	// Newest is previous value in buffer

		// Bias avgMillisPerCycle down 5ms to get back to higher framerate gradually if possible
		float avgMillisPerCycle = (float)(newestTime - oldestTime) / (_processTimeBufferSize - 1) - 5.f;	// Intervals = Timepoints - 1
		if(avgMillisPerCycle > cycleTimeMillis) cycleTimeMillis = avgMillisPerCycle;
	}

	tm.SetTimer(_sendFrameTimer, this, &UNvEncodeManager::SendFrame, cycleTimeMillis / 1000.f, true, remaining);
	CurrentFrameRate = 1000.f / cycleTimeMillis;
}
