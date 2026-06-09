#pragma once

#include "Scene/Component.h"
#include <memory>
#include <string>

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
        void Update(float deltaTime) override;

        void SetMaterial(const std::shared_ptr<Material>& material);
        Material* GetMaterial() const;
        void SetMesh(const std::shared_ptr<Mesh>& mesh);
        const std::shared_ptr<Mesh>& GetMesh() const;
		void SetMeshPath(const std::string& path);
		const std::string& GetMeshPath() const;

    private:
        std::shared_ptr<Material> m_material;
        std::shared_ptr<Mesh> m_mesh;
        std::string m_meshPath;
    };
}