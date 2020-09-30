#pragma once

#include "NvEncode.h"
#include "NvEncoder/NvEncoderCuda.h"
#include <cuda.h>

#include <vector>
#include <atomic>

#include <Engine/TextureRenderTarget2D.h>

#include "BP/USBGCommonBP.h"


extern NVENCODE_API struct FLogCategoryNvEncodeLog : public FLogCategory<ELogVerbosity::Log, ELogVerbosity::All> {
	FLogCategoryNvEncodeLog() : FLogCategory(TEXT("NvEncode Log")) {}
} NvEncodeLog;

namespace NvEncode {

void NVENCODE_API LogMessage(const FString& msg);
void NVENCODE_API LogMessageOnScreen(const FString& msg);
void NVENCODE_API LogMessage(const char* msg);
void NVENCODE_API LogMessageOnScreen(const char* msg);

}

class NVENCODE_API NvEncoderUnreal {
public:

	NvEncoderUnreal(int width, int height, int bitrate, ETextureRenderTargetFormat format);
	~NvEncoderUnreal();


	bool SetInputTexture(UTexture2D* input);
	bool SetInputRenderTarget(UTextureRenderTarget2D* input);

	void EncodeFrame();
	bool GetFrame(int64& data, int& size) const;

	void DestroyEncoder();

protected:
	//void CreateInputTexture();
	CUcontext _context{nullptr};
	int width, height;
	NV_ENC_BUFFER_FORMAT _format;

	//UTexture2D* _inputTexture;
	NvEncoderCuda* _encoder{nullptr};
	mutable std::atomic<void*> _encodedData{nullptr};
	mutable std::atomic<int> _encodedSize{0};

	std::atomic<UTextureRenderTarget2D*> _input{nullptr};
	uint8_t* _inBuffer;

	std::vector<std::vector<uint8_t>> _vPackets;
	uint32_t _framesSent = 0;
};
