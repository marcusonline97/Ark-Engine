#include "Scene/Components/MeshComponent.h"
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
            auto& matObj = json["material"];
            const std::string path = matObj.value("path", "");
            auto mat = Material::Load(path);
            if (mat && matObj.contains("params"))
            {
                auto& paramsObj = matObj["params"];

                // Floats
                if (paramsObj.contains("float"))
                {
                    for (auto& p : paramsObj["float"])
                    {
                        std::string name = p.value("name", "");
                        float value = p.value("value", 0.0f);
                        mat->SetParam(name, value);
                    }
                }

                // Float2
                if (paramsObj.contains("float2"))
                {
                    for (auto& p : paramsObj["float2"])
                    {
                        std::string name = p.value("name", "");
                        float v0 = p.value("value0", 0.0f);
                        float v1 = p.value("value1", 0.0f);
                        mat->SetParam(name, v0, v1);
                    }
                }

                // Float3
                if (paramsObj.contains("float3"))
                {
                    for (auto& p : paramsObj["float3"])
                    {
                        std::string name = p.value("name", "");
                        float v0 = p.value("value0", 0.0f);
                        float v1 = p.value("value1", 0.0f);
                        float v2 = p.value("value2", 0.0f);
                        mat->SetParam(name, glm::vec3(v0, v1, v2));
                    }
                }

                // Textures
                if (paramsObj.contains("textures"))
                {
                    for (auto& p : paramsObj["textures"])
                    {
                        std::string name = p.value("name", "");
                        std::string texPath = p.value("path", "");
                        auto texture = Texture::Load(texPath);

                        mat->SetParam(name, texture);
                    }
                }
            }
            SetMaterial(mat);
            m_materialPath = path;
        }

        if (json.contains("mesh"))
        {
            auto& meshObj = json["mesh"];
            const std::string path = meshObj.value("path", "");
            if (!path.empty())
            {
                SetMeshPath(path);
                return;
            }

            const std::string type = meshObj.value("type", "box");
            if (type == "box")
            {
                glm::vec3 extents;
                extents.x = meshObj.value("x", 1.0f);
                extents.y = meshObj.value("y", 1.0f);
                extents.z = meshObj.value("z", 1.0f);
                auto mesh = Mesh::CreateBox(extents);
                SetMesh(mesh);
                m_meshType = type;
                m_boxExtents = extents;
                m_meshPath.clear();
            }
            else if (type == "sphere")
            {
                float r = meshObj.value("r", 1.0f);
                auto mesh = Mesh::CreateSphere(r, 16, 16);
                SetMesh(mesh);
                m_meshType = type;
                m_sphereRadius = r;
                m_meshPath.clear();
            }
        }
    }

    void MeshComponent::SaveProperties(nlohmann::json& json) const
    {
        if (!m_materialPath.empty())
        {
            if (!json["material"].is_object())
            {
                json["material"] = nlohmann::json::object();
            }
            json["material"]["path"] = m_materialPath;
        }

        if (!m_meshPath.empty())
        {
            json["mesh"] = {
                {"type", "gltf"},
                {"path", m_meshPath}
            };
        }
        else if (m_meshType == "box")
        {
            json["mesh"] = {
                {"type", "box"},
                {"x", m_boxExtents.x},
                {"y", m_boxExtents.y},
                {"z", m_boxExtents.z}
            };
        }
        else if (m_meshType == "sphere")
        {
            json["mesh"] = {
                {"type", "sphere"},
                {"r", m_sphereRadius}
            };
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

    bool MeshComponent::SetMaterialPath(const std::string& path)
    {
        auto material = Material::Load(path);
        if (!material)
        {
            return false;
        }

        SetMaterial(material);
        m_materialPath = path;
        return true;
    }

    bool MeshComponent::SetMeshPath(const std::string& path)
    {
        auto mesh = Mesh::LoadGLTF(path);
        if (!mesh)
        {
            return false;
        }

        SetMesh(mesh);
        m_meshPath = path;
        m_meshType = "gltf";
        return true;
    }

    const std::string& MeshComponent::GetMaterialPath() const
    {
        return m_materialPath;
    }

    const std::string& MeshComponent::GetMeshPath() const
    {
        return m_meshPath;
    }

}