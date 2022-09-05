// WowViewer.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WowViewer.h"

#include "MainWindow.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    std::unique_ptr<MainWindow> win = std::make_unique<MainWindow>("WowViewer", 0, 0, 800, 500);
    win->show(mystd::window_show::normal);
    return mystd::Application::Run();
}