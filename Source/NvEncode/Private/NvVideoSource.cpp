#include "NvVideoSource.h"
#include "NvEncodeManager.h"
#include "NvEncodePlugin.h"
#include <Engine/Engine.h>
#include <Engine/World.h>
#include <GameFramework/GameModeBase.h>
#include <Engine/GameViewportClient.h>

UNvVideoSource::UNvVideoSource(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	CaptureSource = SCS_SceneColorHDRNoAlpha;
}

void UNvVideoSource::BeginPlay() {
	Super::BeginPlay();
	if(bMatchCameraFOV && GetWorld()->GetFirstPlayerController()) {
		_cameraManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
		check(GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport);
		_viewportCache = GEngine->GameViewport->Viewport;
		check(_viewportCache->GetSizeXY().X != 0);
	}
	if(bStartAsDefaultSource) SetAsCurrentSource();
}

void UNvVideoSource::SetAsCurrentSource() {
	const auto gamemode = GetWorld()->GetAuthGameMode();
	checkf(gamemode != nullptr, L"Cannot init NvVideoSource: cannot get gamemode");
	auto encodeManager = gamemode->FindComponentByClass<UNvEncodeManager>();
	if(encodeManager) {
		encodeManager->SetVideoSource(this);
	} else {
		NvEncode::LogMessageOnScreen(L"VideoSource error: missing NvEncodeManager on current GameMode");
	}
}

void UNvVideoSource::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(!bMatchCameraFOV || !_cameraManager) return;
	float HalfFOV = _cameraManager->GetFOVAngle() * 0.5f;
	FOVAngle = FMath::RadiansToDegrees(FMath::Atan(FMath::Tan(FMath::DegreesToRadians(HalfFOV)) / GetViewportAspectRatio())) * 2.f;
}

float UNvVideoSource::GetViewportAspectRatio() {
	const auto size = _viewportCache->GetSizeXY();
	return (float)size.X / size.Y;
}

