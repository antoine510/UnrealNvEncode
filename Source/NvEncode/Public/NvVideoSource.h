#pragma once

#include <CoreMinimal.h>
#include <Components/SceneCaptureComponent2D.h>
#include <Camera/PlayerCameraManager.h>
#include "NvVideoSource.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NVENCODE_API UNvVideoSource : public USceneCaptureComponent2D {
	GENERATED_UCLASS_BODY()
public:

	void BeginPlay() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UFUNCTION(BlueprintCallable)
		void SetAsCurrentSource();

	UPROPERTY(EditAnywhere)
		bool bStartAsDefaultSource = true;

	UPROPERTY(EditAnywhere)
		bool bMatchCameraFOV = false;

private:
	float GetViewportAspectRatio();
	mutable FViewport* _viewportCache;

	APlayerCameraManager* _cameraManager = nullptr;

	float _currentHFOV = -1.0f;
};
