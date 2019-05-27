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

NvEncoderCustom::NvEncoderCustom(uint32_t nWidth, uint32_t nHeight) :
	NvEncoder(NV_ENC_DEVICE_TYPE_DIRECTX, nullptr, nWidth, nHeight, NV_ENC_BUFFER_FORMAT_ARGB, 0, false) {

	if(!m_hEncoder) {
		NVENC_THROW_ERROR("Encoder Initialization failed", NV_ENC_ERR_INVALID_DEVICE);
	}
}

NvEncoderCustom::~NvEncoderCustom() {
	ReleaseD3D11Resources();
}

void NvEncoderCustom::SetInputTexture(ID3D11Texture2D* texture) {
	if(_inputTexture != nullptr) {
		UnregisterResources();
	}

	_inputTexture = texture;
	std::vector<void*> inputframes;
	inputframes.push_back(_inputTexture);
	RegisterResources(inputframes, NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX, _texWidth, _texHeight, 0, GetPixelFormat());
}

void NvEncoderCustom::AllocateInputBuffers(int32_t numInputBuffers) {
	if(!IsHWEncoderInitialized()) {
		NVENC_THROW_ERROR("Encoder intialization failed", NV_ENC_ERR_ENCODER_NOT_INITIALIZED);
	}

	// We use an existing Unreal ID3D11Texture2D.

	/*std::vector<void*> inputFrames;
	for(int i = 0; i < numInputBuffers; i++) {
		ID3D11Texture2D *pInputTextures = NULL;
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = GetMaxEncodeWidth();
		desc.Height = GetMaxEncodeHeight();
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = GetD3D11Format(GetPixelFormat());
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		if(m_pD3D11Device->CreateTexture2D(&desc, NULL, &pInputTextures) != S_OK) {
			NVENC_THROW_ERROR("Failed to create d3d11textures", NV_ENC_ERR_OUT_OF_MEMORY);
		}
		inputFrames.push_back(pInputTextures);
	}
	RegisterResources(inputFrames, NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX, GetMaxEncodeWidth(), GetMaxEncodeHeight(), 0, GetPixelFormat(), false);*/
}

void NvEncoderCustom::ReleaseInputBuffers() {
	ReleaseD3D11Resources();
}

void NvEncoderCustom::ReleaseD3D11Resources() {
	if(!m_hEncoder) {
		return;
	}

	UnregisterResources();

	m_vInputFrames.clear();	// D3D11 resources are handled by Unreal
}

