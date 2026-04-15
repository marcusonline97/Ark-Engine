#include "ArkEngine.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <chrono>
#include <thread>
#endif

ArkEngine::ArkEngine()
{
}

ArkEngine::~ArkEngine()
{
}


void ArkEngine::Run()
{

	while (m_IsRunning)
	{
#if defined(_WIN32)
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
#else
		// Legacy fallback loop: ArkEngine is no longer responsible for owning a GLFW window.
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
	}
}

void ArkEngine::Quit()
{
	m_IsRunning = false;
}
