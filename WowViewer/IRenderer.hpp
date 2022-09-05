#pragma once
#include "framework.h"

struct IRenderer
{
	virtual void Render(ID3D11DeviceContext* pDC,
		ID3D11RenderTargetView* pRTV,
		ID3D11DepthStencilView* pDSV,
		uint64_t time) = 0;

	virtual void Resize(UINT width, UINT height) = 0;
};