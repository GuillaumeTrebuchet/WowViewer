#pragma once
#include "framework.h"

struct IWSTexture
{
	virtual ID3D11ShaderResourceView* GetShaderResource() = 0;
	//virtual ID3D11Texture2D * GetTexture() = 0;

	virtual ~IWSTexture() = 0;
};

inline IWSTexture::~IWSTexture() {}