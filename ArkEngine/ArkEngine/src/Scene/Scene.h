#pragma once

#include "GameObject.h"

#include <vector>
#include <string>
#include <memory>

namespace Engine
{
	class GameObject;

	class Scene
	{
	public:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------

		//-------------------------------------------
		// Functions
		//-------------------------------------------
		void Update(float deltaTime);
		void Clear();

		GameObject* CreateGameObject(const std::string& name, GameObject* parent = nullptr);

		template<typename T, typename = typename std::enable_if_t<std::is_base_of_v<GameObject, T>>>
		T* CreateObject(const std::string& name, GameObject* parent = nullptr)
		{
			auto obj = new T();
			obj->SetName(name);
			SetParent(obj, parent);
			return obj;
		}

		void SetParent(GameObject* obj, GameObject* parent);


	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::vector<std::unique_ptr<GameObject>> m_objects;
		//-------------------------------------------
		// Functions
		//-------------------------------------------
	};
}