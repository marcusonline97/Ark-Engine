#include "ArkEngine.h"
#include "ArkWindow.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <chrono>
#include <thread>
#endif

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
		if (!m_Window || m_Window->ShouldClose())
		{
			m_IsRunning = false;
			break;
		}

		// GLFW-based pump (ArkWindow wraps GLFW on non-Windows targets)
		m_Window->PollEvents();
		m_Window->SwapBuffers();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
	}
}

void ArkEngine::Quit()
{
	m_IsRunning = false;
}
