#pragma once

#include <Windows.h>

namespace mystd
{
	class Application
	{
	public:
		//	Start message loop.
		static int Run()
		{
			MSG msg;

			// Main message loop:
			while (GetMessageW(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

			return (int)msg.wParam;
		}

		//	Quit application with given exit code.
		static void Quit(int nExitCode)
		{
			PostQuitMessage(nExitCode);
		}
	};
};