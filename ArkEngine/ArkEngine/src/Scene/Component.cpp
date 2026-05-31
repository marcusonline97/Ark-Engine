#include "Component.h"

#include <algorithm>

namespace Engine
{
	size_t Component::nextId = 1;


	void Component::LoadProperties(const nlohmann::json& json)
	{

	}

    void Component::SaveProperties(nlohmann::json& json) const
    {

    }

	void Component::Init()
	{

	}

    void Component::OnRemoved()
    {

	}

    void Component::Update(float deltaTime)
    {

	}


	GameObject* Component::GetOwner()
	{
		return m_owner;
	}
    const nlohmann::json& Component::GetSerializedData() const
    {
        return m_serializedData;
    }

    void Component::SetSerializedData(const nlohmann::json& json)
    {
        m_serializedData = json;
    }
	
	ComponentFactory& ComponentFactory::GetInstance()
	{
		static ComponentFactory instance;
		return instance;
	}

    Component* ComponentFactory::CreateComponent(const std::string& name)
    {
        auto it = m_creators.find(name);
        if (it != m_creators.end())
        {
            return it->second->CreateComponent();
        }

        return nullptr;
    }

    bool ComponentFactory::HasParent(size_t objectType, size_t parentType)
    {
        auto record = m_parentMap.find(objectType);
        if (record == m_parentMap.end())
        {
            return false;
        }

        auto& parents = record->second;
        if (std::find(parents.begin(), parents.end(), parentType) != parents.end())
        {
            return true;
        }

        for (auto p : parents)
        {
            if (HasParent(p, parentType))
            {
                return true;
            }
        }

        return false;
    }

    const std::vector<std::string>& ComponentFactory::GetRegisteredNames() const
    {
        return m_registeredNames;
    }

    std::string ComponentFactory::GetTypeName(size_t typeId) const
    {
        auto it = m_typeNames.find(typeId);
        if (it == m_typeNames.end())
        {
            return "UnknownComponent";
        }

        return it->second;
    }
}