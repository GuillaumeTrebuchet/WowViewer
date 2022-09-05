#pragma once
#include "framework.h"

#include "Window.hpp"
#include "Application.hpp"

#include "IRenderer.hpp"

#include "RenderWindow.hpp"
#include "WSRenderer.hpp"

#include "WSResourceManager.hpp"

#include "VertexShader.hpp"
#include "PixelShader.hpp"

#include "LocalFileStream.hpp"

#include "Cube.hpp"
#include "db2.hpp"

#include "CreatureInstance.hpp"

//#include "WMORoot.hpp"
//#include "ADT.hpp"

class MainWindow
	: public mystd::window
{

	std::unique_ptr<RenderWindow> m_pRW;
	std::unique_ptr<WSRenderer> m_pRenderer;

	std::unique_ptr<WSResourceManager> m_resourceManager;

	std::shared_ptr<CreatureInstance> m_instance;
public:
	MainWindow(const char* title, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		: window(title, x, y, width, height, nullptr, mystd::window_style::normal)
	{

		//	Setup 3D window
		m_pRW = std::make_unique<RenderWindow>(m_hWnd);
		m_pRenderer = std::make_unique<WSRenderer>(m_pRW->GetDevice());
		m_pRW->SetRenderer(m_pRenderer.get());

		//	Setup resource manager
		m_resourceManager = std::make_unique<WSResourceManager>(m_pRW->GetDevice());
		m_resourceManager->Initialize(LR"(C:\Program Files (x86)\World of Warcraft*wow)");

		m_instance = std::make_shared<CreatureInstance>();
		m_instance->LoadFromDisplayInfo(*m_resourceManager, 16874);
		//m_instance->LoadFromDisplayInfo(*m_resourceManager, 17693);
		//m_instance->LoadFromM2FileID(*m_resourceManager, 986699);
		m_pRenderer->SetInstance(m_instance);
	}
	~MainWindow()
	{
		m_pRW.release();
		m_pRenderer.release();
	}

	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		int wmId, wmEvent;
		PAINTSTRUCT ps;
		HDC hdc;

		switch (message)
		{
		case WM_KEYDOWN:
		{
			if (wParam == VK_F1 || wParam == VK_F2)
			{
				m_pRenderer->OnKeyDown(wParam);
				break;
			}

			if (wParam == VK_RIGHT)
			{
				m_instance->SetAnimID(m_instance->GetAnimID() + 1);
			}
			else if (wParam == VK_LEFT)
			{
				m_instance->SetAnimID(m_instance->GetAnimID() - 1);
			}

			break;
		}
		case WM_LBUTTONDOWN:
			m_pRenderer->OnLButtonDown(LOWORD(lParam), HIWORD(lParam), wParam);
			break;
		case WM_LBUTTONUP:
			m_pRenderer->OnLButtonUp(LOWORD(lParam), HIWORD(lParam), wParam);
			break;
		case WM_MOUSEMOVE:
			m_pRenderer->OnMouseMove(LOWORD(lParam), HIWORD(lParam), wParam);
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);

			EndPaint(hWnd, &ps);
			break;
		case WM_SIZE:
			m_pRW->Resize(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_DESTROY:
			mystd::Application::Quit(0);
			break;
		}

		return window::WndProc(hWnd, message, wParam, lParam);
	}
};