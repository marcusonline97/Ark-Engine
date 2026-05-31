#include "Application.h"

namespace Engine
{
	void Application::RegisterTypes()
	{

	}

	void Application::RenderUI()
	{

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