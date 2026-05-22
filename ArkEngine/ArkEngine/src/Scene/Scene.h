#pragma once

#include "GameObject.h"
#include "Core/Common.h"

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
		static void RegisterTypes();

		void Update(float deltaTime);
		void Clear();

		GameObject* CreateObject(const std::string& name, GameObject* parent = nullptr);

		template<typename T, typename = typename std::enable_if_t<std::is_base_of_v<GameObject, T>>>
		T* CreateObject(const std::string& name, GameObject* parent = nullptr)
		{
			auto obj = new T();
			obj->SetName(name);
			obj->m_scene = this;
			SetParent(obj, parent);
			return obj;
		}

		bool SetParent(GameObject* obj, GameObject* parent);

		void SetMainCamera(GameObject* camera);
		GameObject* GetMainCamera();

		std::vector<LightData> CollectLights();



	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::vector<std::unique_ptr<GameObject>> m_objects;

		GameObject* m_mainCamera = nullptr;
		//-------------------------------------------
		// Functions
		//-------------------------------------------
		static std::shared_ptr<Scene> Load(const std::string& path);


		void CollectLightsRecursive(GameObject* obj, std::vector<LightData>& out);
		void LoadObject(const nlohmann::json& jsonObject, GameObject* parent);
	};
}