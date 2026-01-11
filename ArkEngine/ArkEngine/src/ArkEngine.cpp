#include "ArkEngine.h"
#include "ArkWindow.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

ArkEngine::ArkEngine()
{
	m_Window = std::make_unique<ArkWindow>(800, 600, "Ark Engine");
}

ArkEngine::~ArkEngine()
{
}


void ArkEngine::Run()
{

	while (m_IsRunning)
	{
		MSG msg = {};
		if (PeekMessage(&msg, HWND(), NULL, NULL, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				m_IsRunning = false;
				continue;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			Sleep(1);
		}
	}
}
