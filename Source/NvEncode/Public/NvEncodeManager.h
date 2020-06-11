#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "NvVideoSource.h"
#include "NvEncodeManager.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NVENCODE_API UNvEncodeManager : public UActorComponent {
	GENERATED_UCLASS_BODY()
public:

	UFUNCTION(BlueprintCallable)
		void SetVideoSource(UNvVideoSource* source);

	// Read-only video source. Set using SetVideoSource
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
		UNvVideoSource* VideoSource = nullptr;

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly)
		UTextureRenderTarget2D* RenderTarget;

private:

};
