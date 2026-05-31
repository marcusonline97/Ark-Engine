#include "Render/Mesh.h"
#include "Graphics/GraphicsAPI.h"
#include "Core/ArkEngine.h"

#include <cgltf/cgltf.h>
#include <algorithm>

namespace
{
    const cgltf_primitive* FindFirstTrianglePrimitive(const cgltf_node* node)
    {
        if (!node)
        {
            return nullptr;
        }

        if (node->mesh)
        {
            for (cgltf_size i = 0; i < node->mesh->primitives_count; ++i)
            {
                const cgltf_primitive& primitive = node->mesh->primitives[i];
                if (primitive.type == cgltf_primitive_type_triangles)
                {
                    return &primitive;
                }
            }
        }

        for (cgltf_size i = 0; i < node->children_count; ++i)
        {
            if (const cgltf_primitive* primitive = FindFirstTrianglePrimitive(node->children[i]))
            {
                return primitive;
            }
        }

        return nullptr;
    }
}

namespace Engine
{
    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices)
    {
        m_vertexLayout = layout;

        auto& graphicsAPI = ArkEngine::GetInstance().GetGraphicsAPI();

        m_VBO = graphicsAPI.CreateVertexBuffer(vertices);
        m_EBO = graphicsAPI.CreateIndexBuffer(indices);

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        for (auto& element : m_vertexLayout.elements)
        {
            glVertexAttribPointer(element.index, element.size, element.type, GL_FALSE,
                m_vertexLayout.stride, (void*)(uintptr_t)element.offset);
            glEnableVertexAttribArray(element.index);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_vertexCout = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;
        m_indexCount = indices.size();
    }

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices)
    {
        m_vertexLayout = layout;

        auto& graphicsAPI = ArkEngine::GetInstance().GetGraphicsAPI();

        m_VBO = graphicsAPI.CreateVertexBuffer(vertices);

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        for (auto& element : m_vertexLayout.elements)
        {
            glVertexAttribPointer(element.index, element.size, element.type, GL_FALSE,
                m_vertexLayout.stride, (void*)(uintptr_t)element.offset);
            glEnableVertexAttribArray(element.index);
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_vertexCout = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;
    }

    void Mesh::Bind()
    {
        glBindVertexArray(m_VAO);
    }

    void Mesh::Unbind()
    {
        glBindVertexArray(0);
    }

    void Mesh::Draw()
    {
        if (m_indexCount > 0)
        {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertexCout));
        }
    }

    void Mesh::DrawIndexedRange(uint32_t startIndex, uint32_t indexCount)
    {
        if (indexCount == 0)
        {
            return;
        }
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount),
            GL_UNSIGNED_INT,
            reinterpret_cast<void*>(static_cast<size_t>(startIndex) * sizeof(uint32_t)));
    }

    void Mesh::UpdateDynamic(const std::vector<float>& vertices)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_vertexCout = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;
    }

    void Mesh::UpdateDynamic(const std::vector<float>& vertices, const std::vector<uint32_t>& indices)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_vertexCout = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;

        if (m_EBO == 0)
        {
            ArkEngine::GetInstance().GetGraphicsAPI().CreateIndexBuffer(indices);
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                indices.size() * sizeof(uint32_t), indices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        m_indexCount = indices.size();
    }

    std::shared_ptr<Mesh> Mesh::CreateBox(const glm::vec3& extents)
    {
        const glm::vec3 half = extents * 0.5f;
        std::vector<float> vertices =
        {
            // Front face
            half.x, half.y, half.z, 1.0f, 0.0f, 0.0f, extents.x, extents.y, 0.0f, 0.0f, 1.0f,
            -half.x, half.y, half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.y, 0.0f, 0.0f, 1.0f,
            -half.x, -half.y, half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            half.x, -half.y, half.z, 1.0f, 1.0f, 0.0f, extents.x, 0.0f, 0.0f, 0.0f, 1.0f,

            // Top face 
            half.x, half.y, -half.z, 1.0f, 0.0f, 0.0f, extents.x, extents.z, 0.0f, 1.0f, 0.0f,
            -half.x, half.y, -half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.z, 0.0f, 1.0f, 0.0f,
            -half.x, half.y, half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            half.x, half.y, half.z, 1.0f, 1.0f, 0.0f, extents.x, 0.0f, 0.0f, 1.0f, 0.0f,

            // Right face
            half.x, half.y, -half.z, 1.0f, 0.0f, 0.0f, extents.z, extents.y, 1.0f, 0.0f, 0.0f,
            half.x, half.y, half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.y, 1.0f, 0.0f, 0.0f,
            half.x, -half.y, half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            half.x, -half.y, -half.z, 1.0f, 1.0f, 0.0f, extents.z, 0.0f, 1.0f, 0.0f, 0.0f,

            // Left face
            -half.x, half.y, half.z, 1.0f, 0.0f, 0.0f, extents.z, extents.y, -1.0f, 0.0f, 0.0f,
            -half.x, half.y, -half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.y, -1.0f, 0.0f, 0.0f,
            -half.x, -half.y, -half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
            -half.x, -half.y, half.z, 1.0f, 1.0f, 0.0f, extents.z, 0.0f, -1.0f, 0.0f, 0.0f,

            // Bottom face
            half.x, -half.y, half.z, 1.0f, 0.0f, 0.0f, extents.x, extents.z, 0.0f, -1.0f, 0.0f,
            -half.x, -half.y, half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.z, 0.0f, -1.0f, 0.0f,
            -half.x, -half.y, -half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
            half.x, -half.y, -half.z, 1.0f, 1.0f, 0.0f, extents.x, 0.0f, 0.0f, -1.0f, 0.0f,

            // Back face
            -half.x, half.y, -half.z, 1.0f, 0.0f, 0.0f, extents.x, extents.y, 0.0f, 0.0f, -1.0f,
            half.x, half.y, -half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.y, 0.0f, 0.0f, -1.0f,
            half.x, -half.y, -half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
            -half.x, -half.y, -half.z, 1.0f, 1.0f, 0.0f, extents.x, 0.0f, 0.0f, 0.0f, -1.0f
        };

        std::vector<unsigned int> indices =
        {
            // front face
            0, 1, 2,
            0, 2, 3,
            // top face
            4, 5, 6,
            4, 6, 7,
            // right face
            8, 9, 10,
            8, 10, 11,
            // left face
            12, 13, 14,
            12, 14, 15,
            // bottom face
            16, 17, 18,
            16, 18, 19,
            // back face
            20, 21, 22,
            20, 22, 23
        };

        Engine::VertexLayout vertexLayout;

        // Postion
        vertexLayout.elements.push_back({
            VertexElement::PositionIndex,
            3,
            GL_FLOAT,
            0
            });
        // Color
        vertexLayout.elements.push_back({
            VertexElement::ColorIndex,
            3,
            GL_FLOAT,
            sizeof(float) * 3
            });
        // UV
        vertexLayout.elements.push_back({
            VertexElement::UVIndex,
            2,
            GL_FLOAT,
            sizeof(float) * 6
            });
        // Normal
        vertexLayout.elements.push_back({
            VertexElement::NormalIndex,
            3,
            GL_FLOAT,
            sizeof(float) * 8
            });
        vertexLayout.stride = sizeof(float) * 11;

        auto result = std::make_shared<Engine::Mesh>(vertexLayout, vertices, indices);

        return result;
    }

    std::shared_ptr<Mesh> Mesh::CreateSphere(float radius, int sectors, int stacks)
    {
        const float PI = 3.14159265358979323846f;

        std::vector<float> vertices((stacks + 1) * (sectors + 1) * 8);
        for (int i = 0; i <= stacks; ++i)
        {
            float stackAngle = PI / 2.0f - static_cast<float>(i) * (PI / static_cast<float>(stacks)); // From -?/2 to ?/2
            float xy = radius * cosf(stackAngle); // x-y plane radius at this stack
            float z = radius * sinf(stackAngle);  // z coordinate

            for (int j = 0; j <= sectors; ++j) {
                float sectorAngle = static_cast<float>(j) * (2.0f * PI / static_cast<float>(sectors)); // From 0 to 2?

                float x = xy * cosf(sectorAngle);
                float y = xy * sinf(sectorAngle);

                size_t vertexStart = (i * (sectors + 1) + j) * 8;
                // Position
                vertices[vertexStart] = x;
                vertices[vertexStart + 1] = y;
                vertices[vertexStart + 2] = z;

                // Normal (normalized position vector)
                float length = sqrtf(x * x + y * y + z * z);
                vertices[vertexStart + 3] = x / length;
                vertices[vertexStart + 4] = y / length;
                vertices[vertexStart + 5] = z / length;

                // UV coordinates
                vertices[vertexStart + 6] = static_cast<float>(j) / static_cast<float>(sectors);
                vertices[vertexStart + 7] = static_cast<float>(i) / static_cast<float>(stacks);
            }
        }

        // Generate indices
        std::vector<unsigned int> indices;
        for (int i = 0; i < stacks; ++i)
        {
            int k1 = i * (sectors + 1);
            int k2 = k1 + sectors + 1;

            for (int j = 0; j < sectors; ++j, ++k1, ++k2)
            {
                if (i != 0)
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                if (i != (stacks - 1))
                {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        Engine::VertexLayout vertexLayout;

        // Postion
        vertexLayout.elements.push_back({
            VertexElement::PositionIndex,
            3,
            GL_FLOAT,
            0
            });
        // Normal
        vertexLayout.elements.push_back({
            VertexElement::NormalIndex,
            3,
            GL_FLOAT,
            sizeof(float) * 3
            });
        // UV
        vertexLayout.elements.push_back({
            VertexElement::UVIndex,
            2,
            GL_FLOAT,
            sizeof(float) * 6
            });
        vertexLayout.stride = sizeof(float) * 8;

        auto result = std::make_shared<Engine::Mesh>(vertexLayout, vertices, indices);

        return result;
    }

    std::shared_ptr<Mesh> Mesh::CreatePlane()
    {
        std::vector<float> vertices =
        {
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
        };

        std::vector<uint32_t> indices =
        {
            0, 1, 2,
            0, 2, 3
        };

        Engine::VertexLayout vertexLayout;

        // Position
        vertexLayout.elements.push_back({
            VertexElement::PositionIndex,
            2,
            GL_FLOAT,
            0
            });
        vertexLayout.stride = sizeof(float) * 2;

        auto result = std::make_shared<Engine::Mesh>(vertexLayout, vertices, indices);

        return result;
    }

    std::shared_ptr<Mesh> Mesh::LoadGLTF(const std::string& path)
    {
        auto& fileSystem = ArkEngine::GetInstance().GetFileSystem();
        auto contents = fileSystem.LoadAssetFileText(path);
        if (contents.empty())
        {
            return nullptr;
        }

        cgltf_options options = {};
        cgltf_data* data = nullptr;
        cgltf_result res = cgltf_parse(&options, contents.data(), contents.size(), &data);
        if (res != cgltf_result_success)
        {
            return nullptr;
        }

        auto fullPath = fileSystem.GetAssetFilePath(path);
        auto fullFolderPath = fullPath.remove_filename();
        res = cgltf_load_buffers(&options, data, fullFolderPath.string().c_str());
        if (res != cgltf_result_success)
        {
            cgltf_free(data);
            return nullptr;
        }

        const cgltf_scene* scene = data->scene;
        if (!scene && data->scenes_count > 0)
        {
            scene = &data->scenes[0];
        }

        const cgltf_primitive* primitive = nullptr;
        if (scene)
        {
            for (cgltf_size i = 0; i < scene->nodes_count; ++i)
            {
                primitive = FindFirstTrianglePrimitive(scene->nodes[i]);
                if (primitive)
                {
                    break;
                }
            }
        }

        if (!primitive)
        {
            cgltf_free(data);
            return nullptr;
        }

        auto readFloats = [](const cgltf_accessor* acc, cgltf_size i, float* out, int n)
        {
            std::fill(out, out + n, 0.0f);
            return cgltf_accessor_read_float(acc, i, out, n) == 1;
        };

        auto readIndex = [](const cgltf_accessor* acc, cgltf_size i)
        {
            cgltf_uint out = 0;
            cgltf_bool ok = cgltf_accessor_read_uint(acc, i, &out, 1);
            return ok ? static_cast<uint32_t>(out) : 0;
        };

        VertexLayout vertexLayout;
        cgltf_accessor* accessors[4] = {};

        for (cgltf_size i = 0; i < primitive->attributes_count; ++i)
        {
            const cgltf_attribute& attr = primitive->attributes[i];
            cgltf_accessor* acc = attr.data;
            if (!acc)
            {
                continue;
            }

            VertexElement element;
            element.type = GL_FLOAT;

            switch (attr.type)
            {
            case cgltf_attribute_type_position:
            {
                accessors[VertexElement::PositionIndex] = acc;
                element.index = VertexElement::PositionIndex;
                element.size = 3;
            }
            break;
            case cgltf_attribute_type_color:
            {
                if (attr.index != 0)
                {
                    continue;
                }
                accessors[VertexElement::ColorIndex] = acc;
                element.index = VertexElement::ColorIndex;
                element.size = 3;
            }
            break;
            case cgltf_attribute_type_texcoord:
            {
                if (attr.index != 0)
                {
                    continue;
                }
                accessors[VertexElement::UVIndex] = acc;
                element.index = VertexElement::UVIndex;
                element.size = 2;
            }
            break;
            case cgltf_attribute_type_normal:
            {
                accessors[VertexElement::NormalIndex] = acc;
                element.index = VertexElement::NormalIndex;
                element.size = 3;
            }
            break;
            default:
                continue;
            }

            if (element.size > 0)
            {
                element.offset = vertexLayout.stride;
                vertexLayout.stride += element.size * sizeof(float);
                vertexLayout.elements.push_back(element);
            }
        }

        if (!accessors[VertexElement::PositionIndex])
        {
            cgltf_free(data);
            return nullptr;
        }

        const cgltf_size vertexCount = accessors[VertexElement::PositionIndex]->count;
        std::vector<float> vertices;
        vertices.resize((vertexLayout.stride / sizeof(float)) * vertexCount);

        for (cgltf_size vi = 0; vi < vertexCount; ++vi)
        {
            for (const auto& element : vertexLayout.elements)
            {
                if (!accessors[element.index])
                {
                    continue;
                }

                auto index = (vi * vertexLayout.stride + element.offset) / sizeof(float);
                float* outData = &vertices[index];
                readFloats(accessors[element.index], vi, outData, element.size);
            }
        }

        std::shared_ptr<Mesh> mesh;
        if (primitive->indices)
        {
            const cgltf_size indexCount = primitive->indices->count;
            std::vector<uint32_t> indices(indexCount);
            for (cgltf_size i = 0; i < indexCount; ++i)
            {
                indices[i] = readIndex(primitive->indices, i);
            }
            mesh = std::make_shared<Mesh>(vertexLayout, vertices, indices);
        }
        else
        {
            mesh = std::make_shared<Mesh>(vertexLayout, vertices);
        }

        cgltf_free(data);
        return mesh;
    }
}