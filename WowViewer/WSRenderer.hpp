#pragma once
#include "framework.h"

#include "IRenderer.hpp"

#include "Camera.hpp"

#include "CreatureInstance.hpp"

enum class WSCameraMovement
{
	None,
	Rotation,
	Scale,
	Translation,
};

class WSRenderer
	: public IRenderer
{
	WSRenderer() = delete;
	WSRenderer(const WSRenderer&) = delete;
	WSRenderer& operator=(const WSRenderer&) = delete;

	CComPtr<ID3D11Device>		m_pD3D11Device;
	CComPtr<ID3D11SamplerState>	m_pSampler;
	CComPtr<ID3D11BlendState>	m_pBlendState;
	CComPtr<ID3D11RasterizerState> m_pRasterState;

	ConstantBuffer<M2_ConstantBuffer> m_pM2_CB;
	ConstantBuffer<M2_AnimConstantBuffer> m_pM2_AnimCB;

	//ConstantBuffer<WMO_ConstantBuffer> m_pWMO_CB;

	//ConstantBuffer<ADT_ConstantBuffer> m_pADT_CB;

	std::shared_ptr<CreatureInstance> m_instance;
	//std::atomic<WMORoot*>	m_pWMO = nullptr;
	//std::atomic<ADT*>		m_pADT = nullptr;

	std::unique_ptr<Camera> m_pCamera;

	FLOAT m_width = 0;
	FLOAT m_height = 0;

	std::mutex m_mutex;

	WSCameraMovement m_movement = WSCameraMovement::None;
	UINT m_mouseX = 0;
	UINT m_mouseY = 0;


public:
	WSRenderer(ID3D11Device* pD3D11Device)
		: m_pD3D11Device(pD3D11Device),
		m_pM2_CB(pD3D11Device),
		m_pM2_AnimCB(pD3D11Device)//,
		//m_pWMO_CB(pD3D11Device),
		//m_pADT_CB(pD3D11Device)
	{

		m_pCamera = std::make_unique<Camera>();

		//	Create rasterization state
		D3D11_RASTERIZER_DESC rasterDesc;
		ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));
		rasterDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
		//rasterDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
		//rasterDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
		rasterDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
		rasterDesc.FrontCounterClockwise = TRUE;
		rasterDesc.DepthBias = 0;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = TRUE;
		rasterDesc.ScissorEnable = FALSE;
		rasterDesc.MultisampleEnable = TRUE;
		rasterDesc.AntialiasedLineEnable = FALSE;

		pD3D11Device->CreateRasterizerState(&rasterDesc, &m_pRasterState);

		//	Create texture sampler
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		pD3D11Device->CreateSamplerState(&sampDesc, &m_pSampler);

		// Create blend state
		D3D11_RENDER_TARGET_BLEND_DESC RTblend = {};
		RTblend.BlendEnable = true;
		RTblend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		RTblend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		RTblend.BlendOp = D3D11_BLEND_OP_ADD;
		RTblend.SrcBlendAlpha = D3D11_BLEND_ONE;
		RTblend.DestBlendAlpha = D3D11_BLEND_ZERO;
		RTblend.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		RTblend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		D3D11_BLEND_DESC BlendStateDesc = {};
		BlendStateDesc.AlphaToCoverageEnable = true;
		BlendStateDesc.IndependentBlendEnable = false;
		BlendStateDesc.RenderTarget[0] = RTblend;

		pD3D11Device->CreateBlendState(&BlendStateDesc, &m_pBlendState);


		//pBlendState->Release();
	}

	void Render(ID3D11DeviceContext* pDC,
		ID3D11RenderTargetView* pRTV,
		ID3D11DepthStencilView* pDSV,
		uint64_t time)
	{
		std::lock_guard<std::mutex> locker(m_mutex);

		pDC->OMSetRenderTargets(1, &pRTV, pDSV);
		pDC->OMSetBlendState(m_pBlendState, 0, 0xffffffff);

		pDC->RSSetViewports(1, &m_pCamera->GetViewport());
		pDC->RSSetState(m_pRasterState);

		//	Render scene
		float ClearColor[4] = { 0, 0, 0, 1 };
		pDC->ClearRenderTargetView(pRTV, ClearColor);
		pDC->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

		m_pM2_CB.cb.View = m_pCamera->GetViewMatrix();
		m_pM2_CB.cb.Projection = m_pCamera->GetTransProjMatrix();
		m_pM2_CB.cb.World = XMMatrixTranspose(XMMatrixIdentity());
		m_pM2_CB.cb.CamPos = m_pCamera->GetPosition();

		auto instance = m_instance.get();
		if (instance)
			instance->Draw(pDC, m_pM2_CB, m_pSampler, m_pM2_AnimCB, time);


		/*m_pWMO_CB.cb.View = m_pCamera->GetViewMatrix();
		m_pWMO_CB.cb.Projection = m_pCamera->GetTransProjMatrix();
		m_pWMO_CB.cb.World = XMMatrixTranspose(XMMatrixIdentity());
		m_pWMO_CB.cb.CamPos = m_pCamera->GetPosition();

		WMORoot* pWMO = m_pWMO;
		if (pWMO)
		{
			pWMO->Draw(pDC, m_pWMO_CB, m_pSampler, m_pM2_CB, m_pM2_AnimCB, elapsedMs);
		}

		m_pADT_CB.cb.View = m_pCamera->GetViewMatrix();
		m_pADT_CB.cb.Projection = m_pCamera->GetTransProjMatrix();
		m_pADT_CB.cb.World = XMMatrixTranspose(XMMatrixIdentity());
		m_pADT_CB.cb.CamPos = m_pCamera->GetPosition();

		ADT* pADT = m_pADT;
		if (pADT)
		{
			pADT->Draw(pDC, m_pSampler, m_pADT_CB, m_pM2_CB, m_pM2_AnimCB, elapsedMs);
		}*/
	}

	void SetInstance(std::shared_ptr<CreatureInstance> inst)
	{
		m_instance = inst;
	}

	/*void SetWMO(WMORoot* pWMO)
	{
		m_pWMO = pWMO;
	}

	void SetADT(ADT* pADT)
	{
		m_pADT = pADT;
	}*/

	void Resize(UINT width, UINT height)
	{
		m_width = (FLOAT)width;
		m_height = (FLOAT)height;

		m_pCamera->SetViewport(width, height, 0, 1, 0, 0);
		m_pCamera->SetProjection(XM_PIDIV2, 0.1f, 1000.0f);
	}

	Camera& GetCamera()
	{
		return *m_pCamera.get();
	}

	void OnLButtonDown(UINT x, UINT y, WPARAM wParam)
	{
		if (wParam & MK_CONTROL)
		{
			m_movement = WSCameraMovement::Scale;
		}
		else if (wParam & MK_SHIFT)
		{
			m_movement = WSCameraMovement::Translation;
		}
		else
		{
			m_movement = WSCameraMovement::Rotation;
		}

		m_mouseX = x;
		m_mouseY = y;
	}

	void OnLButtonUp(UINT x, UINT y, WPARAM wParam)
	{
		m_movement = WSCameraMovement::None;
	}

	void OnMouseMove(UINT x, UINT y, WPARAM wParam)
	{
		switch (m_movement)
		{
		case WSCameraMovement::Rotation:
		{
			FLOAT diffX = ((FLOAT)x) - m_mouseX;
			FLOAT diffY = ((FLOAT)y) - m_mouseY;

			m_pCamera->Rotate(diffY / 100.0f, -diffX / 100.0f, 0);
			break;
		}
		case WSCameraMovement::Scale:
		{
			FLOAT diffX = ((FLOAT)x) - m_mouseX;
			FLOAT diffY = ((FLOAT)y) - m_mouseY;


			m_pCamera->AddFocusDist(diffY / 10.f);
			break;
		}
		case WSCameraMovement::Translation:
		{
			FLOAT diffX = ((FLOAT)x) - m_mouseX;
			FLOAT diffY = ((FLOAT)y) - m_mouseY;


			m_pCamera->Translate(diffX / 10.f, diffY / 10.f, 0);
			break;
		}
		}

		m_mouseX = x;
		m_mouseY = y;
	}

	void OnKeyDown(WPARAM wParam)
	{
		if (wParam == VK_F1)
		{
			m_pCamera->SetRotation(0, 0, 0);
			m_pCamera->SetTranslation(0, 0, 0);
			m_pCamera->SetFocusDist(5.0f);
		}
		if (wParam == VK_F2)
		{
			//ADT* pADT = m_pADT;

			//XMFLOAT3 pos = pADT->GetPosition();
			//m_pCamera->SetTranslation(pos.x, pos.y, pos.z);
		}
	}
};