#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <Windowsx.h>
#endif

#include <locale>
#include <codecvt>
#include <exception>
#include <cstdint>
#include <string_view>

namespace mystd
{
	enum class window_show
	{
		normal,
		maximized,
		minimized,
	};
	enum class window_style
		: uint32_t
	{
		normal = 0,
		popup = 0x1,
		no_maximize = 0x2,
		no_minimize = 0x4,
		no_sizable = 0x8,
	};

	window_style operator|(window_style s1, window_style s2)
	{
		return static_cast<window_style>(static_cast<uint32_t>(s1) | static_cast<uint32_t>(s2));
	}

	window_style operator&(window_style s1, window_style s2)
	{
		return static_cast<window_style>(static_cast<uint32_t>(s1) & static_cast<uint32_t>(s2));
	}
#ifdef _WIN32
	class window
	{
		window() = delete;
		window(const window&) = delete;
		window& operator=(const window&) = delete;
	protected:
		HWND m_hWnd;
		HINSTANCE m_hInstance;

		inline static LRESULT CALLBACK WndProcDispatcher(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			window* pWindow = reinterpret_cast<window*>(GetWindowLongPtrW(hWnd, 0));
			if (pWindow != nullptr)
				return pWindow->WndProc(hWnd, message, wParam, lParam);

			return DefWindowProcW(hWnd, message, wParam, lParam);
		}

		BOOL RegisterWndClass(HINSTANCE hInstance, LPCWSTR className, UINT classStyle, HICON hIcon, LPCTSTR lpszMenuName)
		{
			WNDCLASS wc;
			if (GetClassInfoW(hInstance, className, &wc))
				return TRUE;

			WNDCLASSEX wcex;

			wcex.cbSize = sizeof(WNDCLASSEX);

			wcex.style = CS_HREDRAW | CS_VREDRAW | classStyle;
			wcex.lpfnWndProc = WndProcDispatcher;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = sizeof(window*);
			wcex.hInstance = hInstance;
			wcex.hIcon = hIcon;
			wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
			wcex.hbrBackground = NULL;// (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = lpszMenuName;// MAKEINTRESOURCE(IDC_WOWSTUDIO31);
			wcex.lpszClassName = className;// _T("MYSTD_DEFAULT_WND_CLASS");
			wcex.hIconSm = hIcon;

			return RegisterClassExW(&wcex);
		}

		virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			if (message == WM_DESTROY)
			{
				if (!SetWindowLongPtrW(m_hWnd, 0, reinterpret_cast<LONG_PTR>(nullptr)))
					throw std::exception("SetWindowLongPtr failed");
				m_hWnd = NULL;

				//	This should delete the memory, nothing refering to this should happen after this call
			}
			else if (message == WM_CLOSE)
			{
				on_close();
			}

			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		inline int32_t window_show_to_win32(window_show s)
		{
			switch (s)
			{
			case window_show::normal:
				return SW_NORMAL;
			case window_show::maximized:
				return SW_MAXIMIZE;
			case window_show::minimized:
				return SW_MINIMIZE;
			default:
				throw std::exception();
			}
		}
		inline uint32_t window_style_to_win32(window_style s)
		{
			uint32_t i = 0;
			if ((s & window_style::no_maximize) != window_style::no_maximize)
				i |= WS_MAXIMIZEBOX;
			if ((s & window_style::no_minimize) != window_style::no_minimize)
				i |= WS_MINIMIZEBOX;
			if ((s & window_style::no_sizable) != window_style::no_sizable)
				i |= WS_THICKFRAME;
			if ((s & window_style::popup) == window_style::popup)
				i |= WS_POPUP;
			else
				i |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
			return i;
		}
	public:
		window(std::string_view title, uint32_t x, uint32_t y, uint32_t width, uint32_t height, window* parent, window_style style)
		{
			m_hInstance = GetModuleHandleW(NULL);
			if (!RegisterWndClass(m_hInstance, L"mystd::window", 0, nullptr, nullptr))
				throw std::exception("RegisterClassEx failed.");

			std::wstring wtitle;
			wtitle.resize(title.size());
			if (MultiByteToWideChar(CP_UTF8, 0, title.data(), title.size(), wtitle.data(), wtitle.size()) != title.size())
				throw std::exception();
			m_hWnd = CreateWindowExW(0, L"mystd::window", wtitle.c_str(), window_style_to_win32(style),
				x, y, width, height, parent ? parent->handle() : nullptr, nullptr, m_hInstance, nullptr);

			if (!m_hWnd)
				throw std::exception("CreateWindowEx failed.");

			//	Clear last error cause SetWindowLong does not clear it, and will return 0 if last value was 0.
			SetLastError(0);
			if (!SetWindowLongPtrW(m_hWnd, 0, reinterpret_cast<LONG_PTR>(this)))
			{
				if (GetLastError() != 0)
					throw std::exception("SetWindowLongPtr failed.");
			}
			show(window_show::normal);
		}

		void show(window_show s)
		{
			ShowWindow(m_hWnd, window_show_to_win32(s));
			UpdateWindow(m_hWnd);
		}

		void resize(uint32_t width, uint32_t height)
		{
			::SetWindowPos(m_hWnd,
				NULL,
				0,
				0,
				width,
				height,
				SWP_NOZORDER | SWP_NOMOVE);
		}

		void move(uint32_t x, uint32_t y)
		{
			::SetWindowPos(m_hWnd,
				NULL,
				x,
				y,
				0,
				0,
				SWP_NOZORDER | SWP_NOSIZE);
		}

		void set_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool redraw)
		{
			uint32_t flags = SWP_NOZORDER;

			if (redraw)
				flags = SWP_NOREDRAW;

			::SetWindowPos(m_hWnd,
				NULL,
				x,
				y,
				width,
				height,
				flags);
		}

		void get_rect(uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height)
		{
			RECT rc;
			::GetClientRect(m_hWnd, &rc);
			*x = rc.left;
			*y = rc.top;
			*width = rc.right - rc.left;
			*height = rc.bottom - rc.top;
		}

		void destroy()
		{
			DestroyWindow(m_hWnd);
		}

		bool is_closed()
		{
			return m_hWnd == NULL;
		}

		bool is_visible()
		{
			return IsWindowVisible(m_hWnd);
		}

		void hide()
		{
			ShowWindow(m_hWnd, SW_HIDE);
		}

		void set_parent(window* wnd)
		{
			if (::SetParent(m_hWnd, wnd ? wnd->handle() : nullptr) == NULL)
				throw std::exception("SetParent failed");
		}

		HWND handle()
		{
			return m_hWnd;
		}

		virtual void on_close()
		{

		}
	};
#endif
};