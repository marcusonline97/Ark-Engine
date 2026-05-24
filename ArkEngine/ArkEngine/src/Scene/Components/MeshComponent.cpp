#include "Scene/components/MeshComponent.h"
#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/RenderQueue.h"
#include "Scene/GameObject.h"
#include "Core/ArkEngine.h"

namespace Engine
{
    MeshComponent::MeshComponent(const std::shared_ptr<Material>& material, const std::shared_ptr<Mesh>& mesh)
        : m_material(material), m_mesh(mesh)
    {

    }

    void MeshComponent::LoadProperties(const nlohmann::json& json)
    {
        if (json.contains("material"))
        {
			const std::string matPath = json.value("material", "");
            auto material = Material::Load(matPath);
            if (material)
            {
				SetMaterial(material);
            }


        }

        if (json.contains("mesh"))
        {
			const auto& meshObj = json["mesh"];
			const std::string type = meshObj.value("type", "box");
            if (type == "box")
            {
                glm::vec3 extents(
					meshObj.value("x", 1.0f),
					meshObj.value("y", 1.0f),
					meshObj.value("z", 1.0f)                
                );
				auto mesh = Mesh::CreateBox(extents);
				SetMesh(mesh);
            }
        }
    }


    void MeshComponent::Update(float deltaTime)
    {
        if (!m_material || !m_mesh)
        {
            return;
        }

        RenderCommand command;
        command.material = m_material.get();
        command.mesh = m_mesh.get();
        command.modelMatrix = GetOwner()->GetWorldTransform();

        auto& renderQueue = ArkEngine::GetInstance().GetRenderQueue();
        renderQueue.Submit(command);
    }


    void MeshComponent::SetMaterial(const std::shared_ptr<Material>& material)
    {
        m_material = material;
    }

    void MeshComponent::SetMesh(const std::shared_ptr<Mesh>& mesh)
    {
        m_mesh = mesh;
    }

}