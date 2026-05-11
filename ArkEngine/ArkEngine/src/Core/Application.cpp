#include "Application.h"

namespace Engine
{
	bool Application::Init()
	{
		// Initialization code here
		return true;
	}

	void Application::Destroy()
	{
		// Cleanup code here
	}
	void Application::SetNeedsToBeClosed(bool value)
	{
		m_needsToBeClosed = value;
	}
	bool Application::NeedsToBeClosed() const
	{
		return m_needsToBeClosed;
	}
}