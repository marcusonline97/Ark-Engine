#pragma once
class ArkEngine
{
	public:
	ArkEngine();
	~ArkEngine();
	void Run();
	void Quit();

protected:
	bool m_IsRunning = true;
};