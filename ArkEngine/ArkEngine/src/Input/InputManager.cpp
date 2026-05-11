#include "InputManager.h"

namespace Engine
{
	void InputManager::SetKeyPressed(int key, bool pressed)
	{
		if(key < 0 || key >= static_cast<int>(m_keys.size()))
		{
			return;
		}
		m_keys[key] = pressed;
	}

	bool InputManager::IsKeyPressed(int key) const
	{
		if(key < 0 || key >= static_cast<int>(m_keys.size()))
		{
			return false;
		}
		return m_keys[key];
	}


}



