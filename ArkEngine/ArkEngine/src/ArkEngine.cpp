#include "ArkEngine.h"
#include "ArkWindow.h"

#include <chrono>
#include <thread>

ArkEngine::ArkEngine()
{
	m_Window = std::make_unique<ArkWindow>(800, 600, "Ark Engine");
}

ArkEngine::~ArkEngine()
{
}


void ArkEngine::Run()
{
	while (m_IsRunning && m_Window && !m_Window->ShouldClose())
	{
		m_Window->PollEvents();
		m_Window->SwapBuffers();

		// Basic throttle to avoid spinning at 100% when idle
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void ArkEngine::Quit()
{
	m_IsRunning = false;
}
