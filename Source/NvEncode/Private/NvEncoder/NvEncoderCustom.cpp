/*
* Copyright 2017-2018 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/

#include "NvEncoder/NvEncoderCustom.h"
#include <RHI.h>

#include "Windows/AllowWindowsPlatformTypes.h"
#include <d3d11.h>
#include "Windows/HideWindowsPlatformTypes.h"

#include <cudaD3D11.h>
#include <vector>

NvEncoderCustom::NvEncoderCustom(FRHICommandListImmediate& cmdList, uint32_t nWidth, uint32_t nHeight) :
	NvEncoder(NV_ENC_DEVICE_TYPE_CUDA, CreateCUDAContext(), nWidth, nHeight, NV_ENC_BUFFER_FORMAT_ARGB, 0, false),
	_cmdList(cmdList), Width(nWidth), Height(nHeight) {

	if(!m_hEncoder) {
		NVENC_THROW_ERROR("Encoder Initialization failed", NV_ENC_ERR_INVALID_DEVICE);
	}
}

NvEncoderCustom::~NvEncoderCustom() {
	ReleaseResources();
}

CUcontext NvEncoderCustom::CreateCUDAContext() {
	cuInit(0);
	CUdevice decodeDevice = 0;
	if(cuCtxCreate(&_context, 0, decodeDevice) != CUDA_SUCCESS) {
		NVENC_THROW_ERROR("Could not create CUDA context", NV_ENC_ERR_INVALID_CALL);
	}
	return _context;
}

void NvEncoderCustom::SetInputTexture(FTexture2DRHIRef texture) {
	/*if(_inputTexture != nullptr) {
		UnregisterResources();
	}

	_inputTexture = texture;
	std::vector<void*> inputframes;
	inputframes.push_back(_inputTexture);
	RegisterResources(inputframes, NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX, Width, Height, 0, GetPixelFormat());*/
	_inputTexture = texture;
}

void NvEncoderCustom::AllocateInputBuffers(int32_t numInputBuffers) {
	if(!IsHWEncoderInitialized()) {
		NVENC_THROW_ERROR("Encoder intialization failed", NV_ENC_ERR_ENCODER_NOT_INITIALIZED);
	}

	// We use an existing Unreal ID3D11Texture2D.

	std::vector<void*> inputFrames;
	/*D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = GetMaxEncodeWidth();
	desc.Height = GetMaxEncodeHeight();
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	if(((ID3D11Device*)_cmdList.GetNativeDevice())->CreateTexture2D(&desc, NULL, &_nvencTexture) != S_OK) {
		NVENC_THROW_ERROR("Failed to create d3d11textures", NV_ENC_ERR_OUT_OF_MEMORY);
	}
	inputFrames.push_back(_nvencTexture);*/

	cuCtxPushCurrent(_context);
	auto texCI = FRHIResourceCreateInfo(L"NvEncodeSource");
	_nvencTexture = _cmdList.CreateTexture2D(Width, Height, PF_B8G8R8A8, 1, 1, TexCreate_RenderTargetable, texCI);
	CUgraphicsResource resource;
	cuGraphicsD3D11RegisterResource(&resource, (ID3D11Texture2D*)_nvencTexture.GetReference(), 0);

	inputFrames.push_back(resource);
	cuCtxPopCurrent(nullptr);

	RegisterResources(inputFrames, NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR, Width, Height, 0, GetPixelFormat(), false);
}

void NvEncoderCustom::CopyInputToEncoder(FRHICommandListImmediate& RHICmdList) {
	/*ID3D11DeviceContext* context;
	((ID3D11Device*)RHICmdList.GetNativeDevice())->GetImmediateContext(&context);
	context->CopyResource((ID3D11Resource*)_inputTexture.GetReference(), _nvencTexture);*/
	RHICmdList.CopyTexture(_inputTexture, _nvencTexture, FRHICopyTextureInfo{});
}

void NvEncoderCustom::ReleaseInputBuffers() {
	ReleaseResources();
}

void NvEncoderCustom::ReleaseResources() {
	if(!m_hEncoder) {
		return;
	}

	UnregisterResources();

	m_vInputFrames.clear();
	if(_context != nullptr) {
		if(cuCtxDestroy(_context) != CUDA_SUCCESS) {
			NVENC_THROW_ERROR("Could not destroy CUDA context", NV_ENC_ERR_INVALID_CALL);
		}
		_context = nullptr;
	}
}

