#pragma once

#include "GameObject.h"
#include "Core/Common.h"

#include <vector>
#include <string>
#include <memory>

namespace Engine
{
    class Scene
    {
    public:
        static void RegisterTypes();
        void Update(float deltaTime);
        void Clear();

        GameObject* CreateObject(const std::string& name, GameObject* parent = nullptr);
        GameObject* CreateObject(const std::string& type, const std::string& name, GameObject* parent = nullptr);

        template<typename T, typename = typename std::enable_if_t<std::is_base_of_v<GameObject, T>>>
        T* CreateObject(const std::string& name, GameObject* parent = nullptr)
        {
            auto obj = new T();
            obj->SetName(name);
            obj->m_scene = this;
            if (m_isUpdating)
            {
				m_objectsToAdd.push_back({ obj, parent });
            }

            else
            {
                SetParent(obj, parent);
            }
            return obj;
        }

        bool SetParent(GameObject* obj, GameObject* parent);
		GameObject* FindObjectByName(const std::string& name);
        const std::vector<std::unique_ptr<GameObject>>& GetRootObjects() const;

        void SetMainCamera(GameObject* camera);
        GameObject* GetMainCamera();

        std::vector<LightData> CollectLights();

        static std::shared_ptr<Scene> Load(const std::string& path);
        bool Save(const std::string& path) const;
        nlohmann::json Serialize() const;

    private:
        void CollectLightsRecursive(GameObject* obj, std::vector<LightData>& out);
        void LoadObject(const nlohmann::json& jsonObject, GameObject* parent);
        nlohmann::json SerializeObject(const GameObject* object) const;
        nlohmann::json SerializeComponent(const Component* component) const;

    private:
        std::string m_name = "MyScene";
        std::string m_cameraName;
        std::string m_activeCanvasName;
        std::vector<std::unique_ptr<GameObject>> m_objects;
        std::vector<std::pair<GameObject*, GameObject*>> m_objectsToAdd;
        GameObject* m_mainCamera = nullptr;
        bool m_isUpdating = false;
    };
}