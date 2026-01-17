#pragma once

#include <memory>

class ArkWindow;
class ArkEngine
{
	public:
	ArkEngine();
	~ArkEngine();
	void Run();
	void Quit();

protected:
	bool m_IsRunning = true;
	std::unique_ptr<ArkWindow> m_Window;
};