#pragma once
#include "framework.h"

#include "IWSModel.hpp"

#include "ImmutableIndexBuffer.hpp"
#include "ImmutableVertexBuffer.hpp"

#include "VertexShader.hpp"
#include "PixelShader.hpp"

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

class Cube
	: public IWSModel
{
	//Cube() = delete;
	Cube(const Cube&) = delete;
	Cube& operator=(const Cube&) = delete;

	std::unique_ptr<ImmutableIndexBuffer> m_ib;
	std::unique_ptr<ImmutableVertexBuffer<SimpleVertex>> m_vb;
	std::shared_ptr<VertexShader> m_vs;
	std::shared_ptr<PixelShader> m_ps;
	std::shared_ptr<PixelShader> m_ps2;
	CComPtr<ID3D11RasterizerState> m_rasterizerState;
public:
	Cube(ID3D11Device* device, std::shared_ptr<VertexShader> vs, std::shared_ptr<PixelShader> ps, std::shared_ptr<PixelShader> ps2)
	{
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
		rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.DepthBias = -1;
		rasterizerDesc.AntialiasedLineEnable = TRUE;
		device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);

		m_vs = vs;
		m_ps = ps;
		m_ps2 = ps2;

		std::vector<uint16_t> indices =
		{
			0, 1, 2,
			0, 2, 3,

			4, 5, 6,
			4, 6, 7,
			
			8, 9, 10,
			8, 10, 11,

			12, 13, 14,
			12, 14, 15,

			16, 17, 18,
			16, 18, 19,

			20, 21, 22,
			20, 22, 23,
		};
		m_ib = std::make_unique<ImmutableIndexBuffer>(device, indices);
		std::vector<SimpleVertex> vertices =
		{
			{ XMFLOAT3(-1, 1, -1), XMFLOAT3(0, 0, -1) },
			{ XMFLOAT3(1, 1, -1), XMFLOAT3(0, 0, -1) },
			{ XMFLOAT3(1, -1, -1), XMFLOAT3(0, 0, -1) },
			{ XMFLOAT3(-1, -1, -1), XMFLOAT3(0, 0, -1) },

			{ XMFLOAT3(-1, 1, 1), XMFLOAT3(0, 0, 1) },
			{ XMFLOAT3(1, 1, 1), XMFLOAT3(0, 0, 1) },
			{ XMFLOAT3(1, -1, 1), XMFLOAT3(0, 0, 1) },
			{ XMFLOAT3(-1, -1, 1), XMFLOAT3(0, 0, 1) },

			{ XMFLOAT3(-1, -1, -1), XMFLOAT3(0, -1, 0) },
			{ XMFLOAT3(-1, -1, 1), XMFLOAT3(0, -1, 0) },
			{ XMFLOAT3(1, -1, 1), XMFLOAT3(0, -1, 0) },
			{ XMFLOAT3(1, -1, -1), XMFLOAT3(0, -1, 0) },

			{ XMFLOAT3(-1, 1, -1), XMFLOAT3(0, 1, 0) },
			{ XMFLOAT3(-1, 1, 1), XMFLOAT3(0, 1, 0) },
			{ XMFLOAT3(1, 1, 1), XMFLOAT3(0, 1, 0) },
			{ XMFLOAT3(1, 1, -1), XMFLOAT3(0, 1, 0) },

			{ XMFLOAT3(-1, 1, -1), XMFLOAT3(-1, 0, 0) },
			{ XMFLOAT3(-1, 1, 1), XMFLOAT3(-1, 0, 0) },
			{ XMFLOAT3(-1, -1, 1), XMFLOAT3(-1, 0, 0) },
			{ XMFLOAT3(-1, -1, -1), XMFLOAT3(-1, 0, 0) },

			{ XMFLOAT3(1, 1, -1), XMFLOAT3(1, 0, 0) },
			{ XMFLOAT3(1, 1, 1), XMFLOAT3(1, 0, 0) },
			{ XMFLOAT3(1, -1, 1), XMFLOAT3(1, 0, 0) },
			{ XMFLOAT3(1, -1, -1), XMFLOAT3(1, 0, 0) },
		};
		m_vb = std::make_unique<ImmutableVertexBuffer<SimpleVertex>>(device, vertices);
	}
	void Draw(ID3D11DeviceContext* pDC, ConstantBuffer<M2_ConstantBuffer>& cb, ID3D11SamplerState* pSampler, ConstantBuffer<M2_AnimConstantBuffer>& animCB, float elapsed)
	{
		pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D11Buffer* buffers[]{ m_vb->GetBuffer() };
		UINT strides[]{ m_vb->GetStride() };
		UINT offsets[]{ 0 };
		pDC->IASetVertexBuffers(0, 1, buffers, strides, offsets);
		pDC->IASetIndexBuffer(m_ib->GetBuffer(), DXGI_FORMAT_R16_UINT, 0);

		pDC->IASetInputLayout(m_vs->GetLayout());
		pDC->VSSetShader(m_vs->GetShader(), NULL, 0);
		pDC->PSSetShader(m_ps->GetShader(), NULL, 0);

		cb.Update(pDC);

		ID3D11Buffer* pCB[] =
		{
			cb.GetBuffer(),
			animCB.GetBuffer()
		};

		pDC->VSSetConstantBuffers(0, 2, pCB);
		pDC->PSSetConstantBuffers(0, 2, pCB);

		pDC->PSSetSamplers(0, 1, &pSampler);

		pDC->DrawIndexed(m_ib->GetCount(), 0, 0);

		pDC->RSSetState(m_rasterizerState);
		pDC->PSSetShader(m_ps2->GetShader(), NULL, 0);
		pDC->DrawIndexed(m_ib->GetCount(), 0, 0);

		
	}
};