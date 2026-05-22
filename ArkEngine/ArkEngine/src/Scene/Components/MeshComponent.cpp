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