#include "Scene.h"
#include "Scene/Components/AnimationComponent.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/LightComponent.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Components/PhysicsComponent.h"
#include "Scene/Components/PlayerControllerComponent.h"
#include "Scene/Components/AudioComponent.h"
#include "Scene/Components/AudioListenerComponent.h"
#include "Scene/Components/SpriteComponent.h"
#include "Scene/Components/UI/UIElementComponent.h"
#include "Scene/Components/UI/CanvasComponent.h"
#include "Scene/Components/UI/TextComponent.h"
#include "Scene/Components/UI/ButtonComponent.h"
#include "Scene/Components/UI/RectTransformComponent.h"
#include "Core/ArkEngine.h"

#include <algorithm>
#include <cmath>
#include <glm/geometric.hpp>

namespace Engine
{

    void Scene::RegisterTypes()
    {
        // TODO: Add more components in the future
        AnimationComponent::Register();
        CameraComponent::Register();
        LightComponent::Register();
        MeshComponent::Register();
        PhysicsComponent::Register();
        PlayerControllerComponent::Register();
        AudioComponent::Register();
        AudioListenerComponent::Register();
		SpriteComponent::Register();
        UIElementComponent::Register();
        CanvasComponent::Register();
		TextComponent::Register();
		ButtonComponent::Register();
		RectTransformComponent::Register();
    }

    void Scene::Update(float deltaTime)
    {
        m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(), [](const std::unique_ptr<GameObject>& obj) { return !obj->IsAlive(); }),
            m_objects.end());

        for(auto& obj : m_objectsToAdd)
        {
            SetParent(obj.first, obj.second);
		}
        m_objectsToAdd.clear();

        m_isUpdating = true;
        for (auto it = m_objects.begin(); it != m_objects.end();)
        {
            if ((*it)->IsAlive())
            {
                (*it)->Update(deltaTime);
                ++it;
            }
            else
            {
                it = m_objects.erase(it);
            }
        }
        m_isUpdating = false;
    }

    void Scene::Clear()
    {
        m_objects.clear();
    }

    GameObject* Scene::CreateObject(const std::string& name, GameObject* parent)
    {
        auto obj = new GameObject();
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

    GameObject* Scene::CreateObject(const std::string& type, const std::string& name, GameObject* parent)
    {
        auto obj = GameObjectFactory::GetInstance().CreateGameObject(type);
        if (obj)
        {
            obj->SetName(name);
            obj->m_scene = this;
            if(m_isUpdating)
            {
				m_objectsToAdd.push_back({ obj, parent });
            }
            else
            {
                SetParent(obj, parent);

            }
        }
        return obj;

    }


    bool Scene::SetParent(GameObject* obj, GameObject* parent)
    {
        bool result = false;
        auto currentParent = obj->GetParent();

        if (parent == nullptr)
        {
            if (currentParent != nullptr)
            {
                auto it = std::find_if(
                    currentParent->m_children.begin(),
                    currentParent->m_children.end(),
                    [obj](const std::unique_ptr<GameObject>& el) {
                        return el.get() == obj;
                    }
                );

                if (it != currentParent->m_children.end())
                {
                    m_objects.push_back(std::move(*it));
                    obj->m_parent = nullptr;
                    currentParent->m_children.erase(it);
                    result = true;
                }
            }
            // No parent currently. This can be in 2 cases
            // 1. The object is in the scene root
            // 2. The object has been just created
            else
            {
                auto it = std::find_if(
                    m_objects.begin(),
                    m_objects.end(),
                    [obj](const std::unique_ptr<GameObject>& el) {
                        return el.get() == obj;
                    }
                );

                if (it == m_objects.end())
                {
                    std::unique_ptr<GameObject> objHolder(obj);
                    m_objects.push_back(std::move(objHolder));
                    result = true;
                }
            }
        }
        // We are trying to add it as a child of another object
        else
        {
            if (currentParent != nullptr)
            {
                auto it = std::find_if(
                    currentParent->m_children.begin(),
                    currentParent->m_children.end(),
                    [obj](const std::unique_ptr<GameObject>& el) {
                        return el.get() == obj;
                    }
                );

                if (it != currentParent->m_children.end())
                {
                    bool found = false;
                    auto currentElement = parent;
                    while (currentElement)
                    {
                        if (currentElement == obj)
                        {
                            found = true;
                            break;
                        }
                        currentElement = currentElement->GetParent();
                    }

                    if (!found)
                    {
                        parent->m_children.push_back(std::move(*it));
                        obj->m_parent = parent;
                        currentParent->m_children.erase(it);
                        result = true;
                    }
                }
            }
            // No parent currently. This can be in 2 cases
            // 1. The object is in the scene root
            // 2. The object has been just created
            else
            {
                auto it = std::find_if(
                    m_objects.begin(),
                    m_objects.end(),
                    [obj](const std::unique_ptr<GameObject>& el) {
                        return el.get() == obj;
                    }
                );

                // The object has been hust created
                if (it == m_objects.end())
                {
                    std::unique_ptr<GameObject> objHolder(obj);
                    parent->m_children.push_back(std::move(objHolder));
                    obj->m_parent = parent;
                    result = true;
                }
                else
                {
                    bool found = false;
                    auto currentElement = parent;
                    while (currentElement)
                    {
                        if (currentElement == obj)
                        {
                            found = true;
                            break;
                        }
                        currentElement = currentElement->GetParent();
                    }

                    if (!found)
                    {
                        parent->m_children.push_back(std::move(*it));
                        obj->m_parent = parent;
                        m_objects.erase(it);
                        result = true;
                    }
                }
            }
        }

        return result;
    }

    GameObject* Scene::FindObjectByName(const std::string& name)
    {
        for (auto& obj : m_objects)
        {
            if (auto child = obj->FindChildByName(name))
            {
                return child;
            }
        }
        return nullptr;
    }

    const std::vector<std::unique_ptr<GameObject>>& Scene::GetRootObjects() const
    {
        return m_objects;
	}

    void Scene::SetMainCamera(GameObject* camera)
    {
        m_mainCamera = camera;
    }

    GameObject* Scene::GetMainCamera()
    {
        return m_mainCamera;
    }

    std::vector<LightData> Scene::CollectLights()
    {
        std::vector<LightData> lights;
        for (auto& obj : m_objects)
        {
            CollectLightsRecursive(obj.get(), lights);
        }
        return lights;
    }

    std::shared_ptr<Scene> Scene::Load(const std::string& path)
    {
        const std::string contents = ArkEngine::GetInstance().GetFileSystem().LoadAssetFileText(path);
        if (contents.empty())
        {
            return nullptr;
        }

		auto json = nlohmann::json::parse(contents);
        if (json.empty())
        {
            return nullptr;
		}

		auto result = std::make_shared<Scene>();

		result->m_name = json.value("name", "noname");

        if(json.contains("objects") && json["objects"].is_array())
        {
			const auto& objects = json["objects"];

            for (const auto& obj : objects)
            {
                result->LoadObject(obj, nullptr);
			}
   		}

        if (json.contains("camera"))
        {
            std::string cameraObjName = json.value("camera", "");
			result->m_cameraName = cameraObjName;
            for (const auto& child : result->m_objects)
            {
                if (auto object = child->FindChildByName(cameraObjName))
                {
					result->SetMainCamera(object);
                    break;
                }
            }
        }

		std::string activeCanvasName = json.value("activeCanvas", "");
		result->m_activeCanvasName = activeCanvasName;
        for (auto& child : result->m_objects)
        {
            if (auto canvasObject = child->FindChildByName(activeCanvasName))
            {
                if (auto component = canvasObject->GetComponent<CanvasComponent>())
                {
					ArkEngine::GetInstance().GetUIInputSystem().SetCanvas(component);
                    break;
                }
            }
        }

        return result;
    }

    bool Scene::Save(const std::string& path) const
    {
		auto json = Serialize();
		return ArkEngine::GetInstance().GetFileSystem().SaveAssetFileText(path, json.dump(2));
    }

    nlohmann::json Scene::Serialize() const
    {
        nlohmann::json result;
		result["name"] = m_name;
        result["objects"] = nlohmann::json::array();

        for (const auto& object : m_objects)
        {
            if(object && object->IsAlive())
            {
                result["objects"].push_back(SerializeObject(object.get()));
			}
        }

        if(!m_cameraName.empty())
        {
            result["camera"] = m_cameraName;
		}

        if (!m_activeCanvasName.empty())
        {
            result["activeCanvas"] = m_activeCanvasName;
        }

		return result;
    }

    void Scene::CollectLightsRecursive(GameObject* obj, std::vector<LightData>& out)
    {
        if (auto light = obj->GetComponent<LightComponent>())
        {
            LightData data;
            data.color = light->GetColor();
            data.position = obj->GetWorldPosition();
            data.direction = glm::vec3(0.0f);
            data.type = static_cast<int>(light->GetLightType());
            data.intensity = light->GetIntensity();
            data.range = light->GetRange();

            if (data.type == 0)
            {
                const glm::quat worldRotation = obj->GetWorldRotation();
                const bool hasMeaningfulRotation =
                    std::abs(worldRotation.x) > 0.0001f ||
                    std::abs(worldRotation.y) > 0.0001f ||
                    std::abs(worldRotation.z) > 0.0001f ||
                    std::abs(std::abs(worldRotation.w) - 1.0f) > 0.0001f;

                glm::vec3 direction = worldRotation * glm::vec3(0.0f, 0.0f, -1.0f);
                if (!hasMeaningfulRotation || glm::length(direction) < 0.0001f)
                {
                    direction = glm::vec3(0.3f, -1.0f, 0.2f);
                }
                data.direction = glm::normalize(direction);
            }
            out.push_back(data);
        }

        for (auto& child : obj->m_children)
        {
            CollectLightsRecursive(child.get(), out);
        }
    }

    void Scene::LoadObject(const nlohmann::json& jsonObject, GameObject* parent)
    {
		const std::string name = jsonObject.value("name", "Object");

        GameObject* gameObject = nullptr;

        if (jsonObject.contains("type"))
        {
            const std::string type = jsonObject.value("type", "");
            if (type == "gltf")
            {
                //Gltf
                std::string path = jsonObject.value("path", "");
				gameObject = GameObject::LoadGLTF(path, this);
                if(gameObject)
                {
                    gameObject->SetParent(parent);
					gameObject->SetName(name);

                }
            }
            else
            {
                //Custom type.
				gameObject = CreateObject(type, name, parent);
            }
        }
        else
        {
            gameObject = CreateObject(name, parent);
        }

        if (!gameObject)
        {
            return;

        }

		gameObject->SetSerializedData(jsonObject);

        // Read Transform
        if (jsonObject.contains("position"))
        {
			auto posObj = jsonObject["position"];
            glm::vec3 pos;
            pos.x = posObj.value("x", 0.0f);
			pos.y = posObj.value("y", 0.0f);
			pos.z = posObj.value("z", 0.0f);
			gameObject->SetPosition(pos);
        }

        if (jsonObject.contains("rotation"))
        {
            auto rotObj = jsonObject["rotation"];
            glm::quat rot;
			rot.x = rotObj.value("x", 0.0f);
			rot.y = rotObj.value("y", 0.0f);
			rot.z = rotObj.value("z", 0.0f);
			rot.w = rotObj.value("w", 1.0f);
			gameObject->SetRotation(rot);
        }

        if (jsonObject.contains("scale"))
        {
            auto scaleObj = jsonObject["scale"];
            glm::vec3 scale;
			scale.x = scaleObj.value("x", 1.0f);
            scale.y = scaleObj.value("y", 1.0f);
			scale.z = scaleObj.value("z", 1.0f);
			gameObject->SetScale(scale);
        }

		gameObject->LoadProperties(jsonObject);

        if (jsonObject.contains("components") && jsonObject["components"].is_array())
        {
            const auto& components = jsonObject["components"];
            for (const auto& comp : components)
            {
                std::string type = comp.value("componentType", comp.value("type", ""));
                if (type == "directional" || type == "point")
                {
					type == "LightComponent";
                }
                Component* component = ComponentFactory::GetInstance().CreateComponent(type);
                if (component)
                {
					component->SetSerializedData(comp);
                    component->LoadProperties(comp);
                    gameObject->AddComponent(component);
                }
            }
        }

        if (jsonObject.contains("children") && jsonObject["children"].is_array())
        {
			const auto& children = jsonObject["children"];
            for (const auto& child : children)
            {
                LoadObject(child, gameObject);
            }
        }

        gameObject->Init();
    }

    nlohmann::json Scene::SerializeObject(const GameObject* object) const
    {
		nlohmann::json result = object->GetSerializedData().is_object() ? object->GetSerializedData() : nlohmann::json::object();

		result["name"] = object->GetName(); 

        result["position"] = {
            {"x", object->GetPosition().x},
            {"y", object->GetPosition().y},
            {"z", object->GetPosition().z}
		};

        result["rotation"] = {
            {"x", object->GetRotation().x},
            {"y", object->GetRotation().y},
            {"z", object->GetRotation().z},
			{"w", object->GetRotation().w}
            };

        result["scale"] = {
            {"x", object->GetScale().x},
            {"y", object->GetScale().y},
            {"z", object->GetScale().z}
		};

		object->SaveProperties(result);

		result["components"] = nlohmann::json::array();
        for (const auto& component : object->GetComponents())
        {
            if (component)
            {
				result["components"].push_back(SerializeComponent(component.get()));

            }
        }

        if(result["components"].empty())
        {
            result.erase("components");
		}

		const bool preserveGeneratedChildren = result.value("type", "") == "gltf" && !object->GetSerializedData().contains("children");

		result["children"] = nlohmann::json::array();
        if (!preserveGeneratedChildren)
        {
            for(const auto& child : object->GetChildren())
            {
                if (child && child->IsAlive())
                {
                    result["children"].push_back(SerializeObject(child.get()));
                }
			}
        }

        if(result["children"].empty())
        {
            result.erase("children");
		}
		return result;
    }

    nlohmann::json Scene::SerializeComponent(const Component* component) const
    {
		nlohmann::json result = component->GetSerializedData().is_object() ? component->GetSerializedData() : nlohmann::json::object();

        if (!result.contains("type"))
        {
            result["type"] = ComponentFactory::GetInstance().GetTypeName(component->GetTypeId());
        }

		component->SaveProperties(result);
        const std::string savedType = result.value("type", "");
        if (savedType != ComponentFactory::GetInstance().GetTypeName(component->GetTypeId()))
        {
            result["componentType"] = ComponentFactory::GetInstance().GetTypeName(component->GetTypeId());
        }
		return result;
    }
}