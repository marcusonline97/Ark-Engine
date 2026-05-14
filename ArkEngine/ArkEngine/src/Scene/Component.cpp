#include "Component.h"

namespace Engine
{
	size_t Component::nextId = 1;

	GameObject* Component::GetOwner()
	{
		return m_owner;
	}
	
}