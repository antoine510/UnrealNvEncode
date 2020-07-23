#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "BP/USBGCometBP.h"
#include "BP/USBGNebulaBP.h"
#include "NvVideoSource.h"
#include "NvEncodePlugin.h"
#include "NvEncodeManager.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NVENCODE_API UNvEncodeManager : public UActorComponent {
	GENERATED_UCLASS_BODY()
public:

	void BeginPlay() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void BeginDestroy() override;

	UFUNCTION(BlueprintCallable)
		void Startup(int width, int height);

	UFUNCTION(BlueprintCallable)
		void SetVideoSource(UNvVideoSource* source);

	UFUNCTION()
		void OnFrameSlotReady(USBGCometBP* comet);

	UFUNCTION(BlueprintCallable)
		void SendFrame();

	UPROPERTY(EditAnywhere)
		FString VideoFrameCometPath = L"SBG/VideoFrame";

	UPROPERTY(EditAnywhere)
		FString FramesOutTopicName = L"videoFrame";

	UPROPERTY(EditAnywhere)
		FString FrameSlotReadyTopicName = L"frameSlotReady";

	// Read-only video source. Set using SetVideoSource
	UPROPERTY(BlueprintReadOnly)
		UNvVideoSource* VideoSource = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		float MaximumFrameRate = 30.f;

	UPROPERTY(BlueprintReadOnly)
		float CurrentFrameRate;

	UPROPERTY(BlueprintReadOnly)
		int ReadyFrameSlots;

private:
	void updateAverageFrameRate();

	UPROPERTY()
		USBGCometBP* _videoFrameComet;
	UPROPERTY()
		USBGCometElementBP* _videoFrameElt;
	UPROPERTY()
		USBGTopicBP* _frameSlotReadyTopic;

	UPROPERTY()
		UTextureRenderTarget2D* _renderTarget;
	NvEncoderUnreal* _encoder = nullptr;

	FTimerHandle _sendFrameTimer;
	FTimerHandle _averageRateTimer;

	// Average cycle time processing variables
	static constexpr uint32 _processTimeBufferSize = 16u;
	TCircularBuffer<uint64> _processTimeBuffer{_processTimeBufferSize};
	uint32 _processTimeIndex = 0;
	bool _processTimeBufferValid = false;

	int _waitingForFrame = 0;
};
