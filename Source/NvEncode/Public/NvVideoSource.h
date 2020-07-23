#pragma once

#include <CoreMinimal.h>
#include <Components/SceneCaptureComponent2D.h>
#include <Camera/PlayerCameraManager.h>
#include "NvVideoSource.generated.h"

UENUM(BlueprintType)
enum class ECameraMatchFOV : uint8 {
	None		UMETA(DisplayName = "No matching"),
	MatchFOV	UMETA(DisplayName = "Match FOV"),
	Auto		UMETA(DisplayName = "Auto-detect"),
};

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
		ECameraMatchFOV matchCameraFOV = ECameraMatchFOV::Auto;

private:
	float GetViewportAspectRatio();
	mutable FViewport* _viewportCache;

	bool _bMatchFOV = false;
	APlayerCameraManager* _cameraManager = nullptr;

	float _currentHFOV = -1.0f;
};
