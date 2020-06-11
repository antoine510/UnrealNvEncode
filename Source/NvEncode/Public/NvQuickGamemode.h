#pragma once

#include "BP/USBGSatLibBP.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/EngineTypes.h"
#include "NvQuickGamemode.generated.h"

UCLASS()
class NVENCODE_API ANvQuickGamemode : public AGameModeBase {
	GENERATED_UCLASS_BODY()
public:

	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type reason) override;

	UFUNCTION(BlueprintImplementableEvent)
		void OnInitGame();

	UFUNCTION(BlueprintImplementableEvent)
		void OnInitStarburst();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnStarburstInitialized(bool success);

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly)
		USBGSatLibBP* SatLibBP;

};
