#pragma once

#include "NvEncode.h"
#include "NvEncoder/NvEncoderCuda.h"
#include <cuda.h>

#include <vector>
#include <atomic>

#include <Tickable.h>
#include "BP/USBGCommonBP.h"

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
class NVENCODE_API UNvEncoder : public UObject, public FTickableGameObject {
	GENERATED_UCLASS_BODY()
public:
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FEncodeFinishedDelegate, int64, data, int, size);


	TStatId GetStatId() const override { return GetStatID(); }
	bool IsTickable() const override {
		return true;
	}

	void Tick(float) override {
		int size = _encodedSize.exchange(0, std::memory_order_acquire);
		if(size > 0) {
			_delegate.ExecuteIfBound(reinterpret_cast<int64>(_encodedData.load(std::memory_order_relaxed)), size);
		}
	}

	void BeginDestroy() override { DestroyEncoder(); Super::BeginDestroy(); }

	UFUNCTION(BlueprintCallable, Category = "NvEncoder")
		static bool CreateEncoder(int width, int height, int bitrate, UNvEncoder*& encoder);

	UFUNCTION(BlueprintCallable)
		bool SetInputTexture(UTexture2D* input);

	UFUNCTION(BlueprintCallable)
		bool SetInputRenderTarget(UTextureRenderTarget2D* input);

	UFUNCTION(BlueprintCallable)
		bool EncodeFrame(const FEncodeFinishedDelegate& encodeDone);

	UFUNCTION(BlueprintCallable)
		void DestroyEncoder();


	bool IsValid() const { return _encoder != nullptr; }

	/**
	* Test validity of UNvEncoder
	*
	* @param	Test			The object to test
	* @return	Return true if the object is usable
	*/
	UFUNCTION(BlueprintPure, Meta = (CompactNodeTitle = "IsValid"))
		static bool IsValid(const UNvEncoder* Test) { return IsValidT(Test); }

protected:
	//void CreateInputTexture();
	CUcontext _context;
	int width, height;

	//UTexture2D* _inputTexture;
	NvEncoderCuda* _encoder{nullptr};
	std::atomic<void*> _encodedData{nullptr};
	std::atomic<int> _encodedSize{0};

	UTextureRenderTarget2D* _input;
	uint8_t* _inBuffer;

	std::vector<std::vector<uint8_t>> _vPackets;
	uint32_t _framesSent = 0;

private:
	FEncodeFinishedDelegate _delegate;
};
