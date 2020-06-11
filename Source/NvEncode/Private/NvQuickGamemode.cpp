#include "NvQuickGamemode.h"
#include "BP/USBGEngineBP.h"

ANvQuickGamemode::ANvQuickGamemode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	SatLibBP = CreateDefaultSubobject<USBGSatLibBP>(L"SatLibBP");
}

void ANvQuickGamemode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) {
	Super::InitGame(MapName, Options, ErrorMessage);
	
	OnInitGame();
}

void ANvQuickGamemode::BeginPlay() {
	//Init starburst
	if(IsValid(SatLibBP)) {
		OnInitStarburst();
		Super::BeginPlay();
	} else {
		Super::BeginPlay();
		OnStarburstInitialized(false);
	}
}

void ANvQuickGamemode::EndPlay(EEndPlayReason::Type reason) {
	USBGSessionBP* res = nullptr;
	USBGEngineBP::GetCurrentSession(res);
	// Only stop session if we are really quitting
	if(res && reason != EEndPlayReason::LevelTransition && reason != EEndPlayReason::RemovedFromWorld) {
		res->StopSession();
	}
	Super::EndPlay(reason);
}

