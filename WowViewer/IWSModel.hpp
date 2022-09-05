#pragma once
#include "framework.h"

#include "ConstantBuffer.hpp"

struct M2_AnimConstantBuffer
{
	//XMFLOAT4 time;
	//M2_D3D11_Bone bones[150];
	XMMATRIX bones[255];
};

struct M2_ConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
	XMFLOAT4 CamPos;
	XMFLOAT4 Color[3];
	uint32_t textureCount;
};

struct IWSModel
{
	virtual void Draw(ID3D11DeviceContext* pDC, ConstantBuffer<M2_ConstantBuffer>& cb, ID3D11SamplerState* pSampler, ConstantBuffer<M2_AnimConstantBuffer>& animCB, float elapsed) = 0;

	virtual ~IWSModel() = 0;
};

inline IWSModel::~IWSModel() {}