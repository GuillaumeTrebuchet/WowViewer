#pragma once
#include "framework.h"

#include "IRenderer.hpp"

class RenderWindow
{
	RenderWindow() = delete;
	RenderWindow(const RenderWindow&) = delete;
	RenderWindow& operator=(const RenderWindow&) = delete;

	HWND m_hWnd;

	//	D3D11
	CComPtr<ID3D11Device>			m_pD3D11Device;
	CComPtr<ID3D11DeviceContext>	m_pD3D11DeviceContext;
	CComPtr<IDXGISwapChain>			m_pSwapChain;

	//	Buffers
	CComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
	CComPtr<ID3D11Texture2D>		m_pDepthStencil;
	CComPtr<ID3D11DepthStencilView>	m_pDepthStencilView;

	D3D11_VIEWPORT					m_vp;

	//	Synchronization
	std::unique_ptr<std::thread>	m_pRenderingThread;
	std::atomic<bool>				m_bRunning = false;
	std::mutex						m_renderMutex;

	__int64							m_frequency = 0;

	std::atomic<__int64>			m_clockCountPerFrame;

	std::atomic<IRenderer*>			m_pRenderer = nullptr;
	__int64							m_lastUpdateCount = 0;
	__int64							m_lastRenderCount = 0;
	uint64_t						m_time = 0;

	UINT							m_width;
	UINT							m_height;

	void ReleaseBuffers()
	{
		m_pRenderTargetView.Release();
		m_pDepthStencil.Release();
		m_pDepthStencilView.Release();
	}
	void BindBuffers(UINT width, UINT height)
	{
		m_width = width;
		m_height = height;

		// Crée le backbuffer
		CComPtr<ID3D11Texture2D> pBackBuffer;
		if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)))
			throw("Cannot create backbuffer.");

		HRESULT hr = m_pD3D11Device->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
		if (FAILED(hr))
			throw("Cannot create RenderTargetView.");

		// Crée le depth buffer
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 4;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = m_pD3D11Device->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
		if (FAILED(hr))
			throw("Cannot create DepthBuffer.");

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		descDSV.Texture2D.MipSlice = 0;
		hr = m_pD3D11Device->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
		if (FAILED(hr))
			throw("Cannot create DepthStencilView.");

		//m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//	Crée le viewport

		m_vp.Width = (FLOAT)(width);
		m_vp.Height = (FLOAT)(height);
		m_vp.MinDepth = 0.0f;
		m_vp.MaxDepth = 1.0f;
		m_vp.TopLeftX = 0;
		m_vp.TopLeftY = 0;
		//m_pImmediateContext->RSSetViewports(1, &vp);

		// Initialize the projection matrix
		//m_pProjectionMatrix = (XMMATRIX*)_aligned_malloc(sizeof(XMMATRIX), 16);
		//XMMATRIX proj_matrix = XMMatrixPerspectiveFovLH(1, (rc.right - rc.left) / (FLOAT)(rc.bottom - rc.top), 0.01f, 1000.0f);
		//memcpy(m_pProjectionMatrix, &proj_matrix, sizeof(XMMATRIX));
	}
	void InitD3D11(ID3D11Device* pD3D11Device)
	{
		//	Retrouve la taille de la fenetre.
		RECT rc = {};
		GetClientRect(m_hWnd, &rc);

		if (pD3D11Device != nullptr)
		{
			m_pD3D11Device = pD3D11Device;

			//	Create swap chain from device
			CComPtr<IDXGIDevice> pDXGIDevice;
			HRESULT hr = pD3D11Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);

			CComPtr<IDXGIAdapter> pDXGIAdapter;
			hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);

			CComPtr<IDXGIFactory> pDXGIFactory;
			pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDXGIFactory);

			//	Retrouve la taille de la fenetre.
			RECT rc = {};
			GetClientRect(m_hWnd, &rc);

			//	Crée le swapchain
			DXGI_SWAP_CHAIN_DESC sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = 1;
			sd.BufferDesc.Width = rc.right - rc.left;
			sd.BufferDesc.Height = rc.bottom - rc.top;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = m_hWnd;
			sd.SampleDesc.Count = 4;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = true;

			hr = pDXGIFactory->CreateSwapChain(pD3D11Device, &sd, &m_pSwapChain);
		}
		else
		{


			//	Crée le swapchain
			DXGI_SWAP_CHAIN_DESC sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = 1;
			sd.BufferDesc.Width = rc.right - rc.left;
			sd.BufferDesc.Height = rc.bottom - rc.top;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = m_hWnd;
			sd.SampleDesc.Count = 4;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = true;

			D3D_FEATURE_LEVEL featureLevels[] =
			{
				D3D_FEATURE_LEVEL_11_0,
				//D3D_FEATURE_LEVEL_10_1,
				//D3D_FEATURE_LEVEL_10_0,
			};

			UINT flags = NULL;
#ifdef _DEBUG
			flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			D3D_FEATURE_LEVEL featureLevel;
			HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, 1,
				D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pD3D11Device, &featureLevel, &m_pD3D11DeviceContext);

			if (FAILED(hr))
				throw("Cannot create device or swapchain.");
		}

		BindBuffers(rc.right - rc.left, rc.bottom - rc.top);
	}

	void RenderingLoop()
	{
		while (m_bRunning)
		{
			Render();
		}
	}
public:
	RenderWindow(HWND hWnd, ID3D11Device* pD3D11Device)
		: m_hWnd(hWnd)
	{
		InitD3D11(pD3D11Device);

		//	Query frequency for high resolution timer
		QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);

		SetFrameRate(100);

		m_bRunning = true;
		m_pRenderingThread = std::make_unique<std::thread>(&RenderWindow::RenderingLoop, this);
	}
	RenderWindow(HWND hWnd)
		: m_hWnd(hWnd)
	{
		InitD3D11(nullptr);

		//	Query frequency for high resolution timer
		QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);

		SetFrameRate(100);

		m_bRunning = true;
		m_pRenderingThread = std::make_unique<std::thread>(&RenderWindow::RenderingLoop, this);
	}
	~RenderWindow()
	{
		m_bRunning = false;
		m_pRenderingThread->join();
	}

	//	Resize buffers. This function is thread safe.
	void Resize(UINT width, UINT height)
	{
		if (width == 0 || height == 0)
			return;

		//	Lock mutex to prevent data races. 
		std::lock_guard<std::mutex> locker(m_renderMutex);

		//	Release buffers
		ReleaseBuffers();

		//	Rebuild buffers and resize swap chain
		UINT flags = NULL;
#ifdef _DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT hr = m_pSwapChain->ResizeBuffers(1, 0, 0, DXGI_FORMAT_UNKNOWN, flags);

		BindBuffers(width, height);

		IRenderer* pRenderer = m_pRenderer;
		if (pRenderer)
			pRenderer->Resize(width, height);
	}

	void Render()
	{
		//	Get current clock count
		__int64 currentCount;
		QueryPerformanceCounter((LARGE_INTEGER*)&currentCount);
		m_lastUpdateCount = currentCount;

		IRenderer* pRenderer = m_pRenderer;
		if (pRenderer)
		{
			std::lock_guard<std::mutex> lock(m_renderMutex);
			pRenderer->Render(m_pD3D11DeviceContext, m_pRenderTargetView, m_pDepthStencilView, m_lastUpdateCount * 1000 / m_frequency );
		}

		QueryPerformanceCounter((LARGE_INTEGER*)&currentCount);
		__int64 elapsedCount = currentCount - m_lastRenderCount;

		//	Sleep to keep frame rate
		if (elapsedCount < m_clockCountPerFrame)
			Sleep((m_clockCountPerFrame - elapsedCount) * 1000 / m_frequency);

		{
			std::lock_guard<std::mutex> lock(m_renderMutex);
			m_pSwapChain->Present(0, 0);
		}
		QueryPerformanceCounter((LARGE_INTEGER*)&m_lastRenderCount);
	}

	void SetFrameRate(unsigned int rate)
	{
		m_clockCountPerFrame = m_frequency / rate;
	}

	void SetRenderer(IRenderer* pRenderer)
	{
		std::lock_guard<std::mutex> locker(m_renderMutex);
		m_pRenderer = pRenderer;
		if (pRenderer)
			pRenderer->Resize(m_width, m_height);
	}

	ID3D11Device* GetDevice()
	{
		return m_pD3D11Device;
	}
};