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

#pragma once

#include <RHIResources.h>
#include <RHICommandList.h>
#include <cuda.h>

#include "NvEncoder.h"

class NvEncoderCustom : public NvEncoder {
public:
	NvEncoderCustom(FRHICommandListImmediate& cmdList, uint32_t nWidth, uint32_t nHeight);
	virtual ~NvEncoderCustom();

	void SetInputTexture(FTexture2DRHIRef texture);
	void CopyInputToEncoder(FRHICommandListImmediate& RHICmdList);

private:
	/**
	*  @brief This function is used to allocate input buffers for encoding.
	*  This function is an override of virtual function NvEncoder::AllocateInputBuffers().
	*  This function creates ID3D11Texture2D textures which is used to accept input data.
	*  To obtain handle to input buffers application must call NvEncoder::GetNextInputFrame()
	*/
	virtual void AllocateInputBuffers(int32_t numInputBuffers) override;

	/**
	*  @brief This function is used to release the input buffers allocated for encoding.
	*  This function is an override of virtual function NvEncoder::ReleaseInputBuffers().
	*/
	virtual void ReleaseInputBuffers() override;

	/**
	*  @brief This is a private function to release textures used for encoding.
	*/
	void ReleaseResources();

	CUcontext CreateCUDAContext();

private:
	FTexture2DRHIRef _inputTexture;
	FTexture2DRHIRef _nvencTexture;

	FRHICommandListImmediate& _cmdList;
	CUcontext _context;

public:
	uint32_t Width, Height;
};
