#pragma once

#include "NvEncode.h"
#include "USBGCommon.h"

#include "NvEncoder/NvEncoderCustom.h"

#include <vector>
#include "Kismet/BlueprintFunctionLibrary.h"

#include "NvEncodePlugin.generated.h"

extern NVENCODE_API struct FLogCategoryNvEncodeLog : public FLogCategory<ELogVerbosity::Log, ELogVerbosity::All> {
	FLogCategoryNvEncodeLog() : FLogCategory(TEXT("NvEncode Log")) {}
} NvEncodeLog;

namespace NvEncode {

void NVENCODE_API LogMessage(const FString& msg);
void NVENCODE_API LogMessageOnScreen(const FString& msg);
void NVENCODE_API LogMessage(const char* msg);
void NVENCODE_API LogMessageOnScreen(const char* msg);


}

UCLASS(BlueprintType)
class NVENCODE_API UNvEncoder : public UObject {
	GENERATED_UCLASS_BODY()
public:

	void BeginDestroy() override { DestroyEncoder(); Super::BeginDestroy(); }

	UFUNCTION(BlueprintCallable)
		bool InitializeEncoder(int width, int height, int bitrate);

	UFUNCTION(BlueprintCallable)
		void SetInputTexture(UTexture2D* input);

	UFUNCTION(BlueprintCallable)
		void SetInputRenderTarget(UTextureRenderTarget2D* input);

	UFUNCTION(BlueprintCallable)
		void EncodeFrame(int64& result, int& size);

	UFUNCTION(BlueprintCallable)
		void DestroyEncoder();

protected:
	//void CreateInputTexture();

	//UTexture2D* _inputTexture;
	NvEncoderCustom* _encoder = nullptr;
	std::vector<std::vector<uint8_t>> _vPackets;
	uint32_t _framesSent = 0;
};
