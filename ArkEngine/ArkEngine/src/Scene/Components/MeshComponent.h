#pragma once

#include "Scene/Component.h"
#include <glm/vec3.hpp>
#include <memory>
namespace Engine
{

	class Material;
	class Mesh;

    class MeshComponent : public Component
    {
        COMPONENT(MeshComponent)
    public:
        MeshComponent() = default;
        MeshComponent(const std::shared_ptr<Material>& material, const std::shared_ptr<Mesh>& mesh);
		void LoadProperties(const nlohmann::json& json) override;
        void SaveProperties(nlohmann::json& json) const override;
        void Update(float deltaTime) override;

        void SetMaterial(const std::shared_ptr<Material>& material);
        void SetMesh(const std::shared_ptr<Mesh>& mesh);
        bool SetMaterialPath(const std::string& path);
        bool SetMeshPath(const std::string& path);
        const std::string& GetMaterialPath() const;
        const std::string& GetMeshPath() const;

    private:
        std::shared_ptr<Material> m_material;
        std::shared_ptr<Mesh> m_mesh;
        std::string m_materialPath;
        std::string m_meshPath;
        std::string m_meshType;
        glm::vec3 m_boxExtents = glm::vec3(1.0f);
        float m_sphereRadius = 1.0f;
    };
}