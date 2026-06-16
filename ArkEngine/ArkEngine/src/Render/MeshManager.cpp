#include "Render/MeshManager.h"

#include "Core/ArkEngine.h"
#include "Graphics/VertexLayout.h"
#include "Logger.h"
#include "Render/Mesh.h"
#include "Scene/GameObject.h"
#include "Scene/Scene.h"
#include "Scene/Components/MeshComponent.h"

#include <Assimp/Importer.hpp>
#include <Assimp/postprocess.h>
#include <Assimp/scene.h>
#include <glm/geometric.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec4.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <vector>

namespace Engine
{
	namespace
	{
		std::string ToLower(std::string value)
		{
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
				{
					return static_cast<char>(std::tolower(c));
				});
			return value;
		}

		const VertexElement* FindElement(const VertexLayout& layout, GLuint index)
		{
			const auto it = std::find_if(layout.elements.begin(), layout.elements.end(), [index](const VertexElement& element)
				{
					return element.index == index;
				});
			return it != layout.elements.end() ? &(*it) : nullptr;
		}

		bool GetVertexElement(GLuint index, VertexElement& element, GLint& stride)
		{
			GLint enabled = 0;
			glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
			if (!enabled)
			{
				return false;
			}

			GLint size = 0;
			GLint type = GL_FLOAT;
			void* offset = nullptr;
			glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
			glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
			glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
			glGetVertexAttribPointerv(index, GL_VERTEX_ATTRIB_ARRAY_POINTER, &offset);

			element.index = index;
			element.size = static_cast<GLuint>(size);
			element.type = static_cast<GLuint>(type);
			element.offset = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(offset));
			return stride > 0 && element.size > 0;
		}

		std::shared_ptr<Mesh> ApplyWorldTransformToMesh(const std::shared_ptr<Mesh>& source, const glm::mat4& transform)
		{
			if (!source)
			{
				return nullptr;
			}

			GLint previousVertexArray = 0;
			GLint previousArrayBuffer = 0;
			glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVertexArray);
			glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousArrayBuffer);

			source->Bind();

			VertexLayout vertexLayout;
			GLint vertexStride = 0;
			GLint vertexBuffer = 0;
			glGetVertexAttribiv(VertexElement::PositionIndex, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vertexBuffer);

			const GLuint attributeIndices[] =
			{
				VertexElement::PositionIndex,
				VertexElement::ColorIndex,
				VertexElement::UVIndex,
				VertexElement::NormalIndex
			};
			for (GLuint index : attributeIndices)
			{
				VertexElement element;
				GLint stride = 0;
				if (GetVertexElement(index, element, stride))
				{
					vertexLayout.elements.push_back(element);
					vertexStride = vertexStride == 0 ? stride : vertexStride;
				}
			}
			vertexLayout.stride = static_cast<uint32_t>(vertexStride);

			const VertexElement* positionElement = FindElement(vertexLayout, VertexElement::PositionIndex);
			if (!positionElement || vertexBuffer == 0 || vertexLayout.stride == 0)
			{
				glBindVertexArray(static_cast<GLuint>(previousVertexArray));
				glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previousArrayBuffer));
				return source;
			}

			glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(vertexBuffer));
			GLint vertexBufferSize = 0;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vertexBufferSize);
			if (vertexBufferSize <= 0 || vertexBufferSize % sizeof(float) != 0)
			{
				glBindVertexArray(static_cast<GLuint>(previousVertexArray));
				glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previousArrayBuffer));
				return source;
			}

			std::vector<float> vertices(static_cast<size_t>(vertexBufferSize) / sizeof(float));
			glGetBufferSubData(GL_ARRAY_BUFFER, 0, vertexBufferSize, vertices.data());

			const size_t strideFloats = vertexLayout.stride / sizeof(float);
			const size_t positionOffset = positionElement->offset / sizeof(float);
			const size_t vertexCount = strideFloats > 0 ? vertices.size() / strideFloats : 0;
			for (size_t vi = 0; vi < vertexCount; ++vi)
			{
				const size_t index = vi * strideFloats + positionOffset;
				glm::vec4 position(0.0f, 0.0f, 0.0f, 1.0f);
				for (GLuint ci = 0; ci < positionElement->size && ci < 3; ++ci)
				{
					position[ci] = vertices[index + ci];
				}

				const glm::vec4 transformed = transform * position;
				vertices[index] = transformed.x;
				vertices[index + 1] = transformed.y;
				vertices[index + 2] = transformed.z;
			}

			if (const VertexElement* normalElement = FindElement(vertexLayout, VertexElement::NormalIndex))
			{
				const glm::mat3 normalTransform = glm::transpose(glm::inverse(glm::mat3(transform)));
				const size_t normalOffset = normalElement->offset / sizeof(float);
				for (size_t vi = 0; vi < vertexCount; ++vi)
				{
					const size_t index = vi * strideFloats + normalOffset;
					const glm::vec3 normal(vertices[index], vertices[index + 1], vertices[index + 2]);
					const glm::vec3 transformedNormal = normalTransform * normal;
					const float length = glm::length(transformedNormal);
					if (length > 0.0f)
					{
						const glm::vec3 normalized = transformedNormal / length;
						vertices[index] = normalized.x;
						vertices[index + 1] = normalized.y;
						vertices[index + 2] = normalized.z;
					}
				}
			}

			GLint indexBuffer = 0;
			glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &indexBuffer);
			std::vector<uint32_t> indices;
			if (indexBuffer != 0)
			{
				GLint indexBufferSize = 0;
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(indexBuffer));
				glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &indexBufferSize);
				if (indexBufferSize > 0 && indexBufferSize % sizeof(uint32_t) == 0)
				{
					indices.resize(static_cast<size_t>(indexBufferSize) / sizeof(uint32_t));
					glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexBufferSize, indices.data());
				}
			}

			glBindVertexArray(static_cast<GLuint>(previousVertexArray));
			glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previousArrayBuffer));

			if (!indices.empty())
			{
				return std::make_shared<Mesh>(vertexLayout, vertices, indices);
			}

			return std::make_shared<Mesh>(vertexLayout, vertices);
		}

		void AddVertexElement(VertexLayout& layout, GLuint index, GLuint size)
		{
			VertexElement element;
			element.index = index;
			element.size = size;
			element.type = GL_FLOAT;
			element.offset = layout.stride;

			layout.stride += element.size * sizeof(float);
			layout.elements.push_back(element);
		}

		std::shared_ptr<Mesh> FindFirstMesh(GameObject* object)
		{
			if (!object)
			{
				return nullptr;
			}

			if (auto* meshComponent = object->GetComponent<MeshComponent>())
			{
				return ApplyWorldTransformToMesh(meshComponent->GetMesh(), object->GetWorldTransform());
			}

			for (const auto& child : object->GetChildren())
			{
				if (auto mesh = FindFirstMesh(child.get()))
				{
					return mesh;
				}
			}

			return nullptr;
		}

		std::shared_ptr<Mesh> LoadMeshFromGLTF(const std::string& path)
		{
			Scene loadScene;
			GameObject* loadedObject = GameObject::LoadGLTF(path, &loadScene);
			if (!loadedObject) return nullptr;

			// Count total mesh components to warn about multi-mesh GLTFs
			int meshCount = 0;
			std::function<void(GameObject*)> count = [&](GameObject* obj) {
				if (obj->GetComponent<MeshComponent>()) ++meshCount;
				for (auto& c : obj->GetChildren()) count(c.get());
			};
			count(loadedObject);

			if (meshCount > 1)
			{
				Logging::Warning() << "MeshManager: '" << path << "' contains " << meshCount
					<< " meshes. Only the first will be used. "
					"Place this asset as a scene object (type:gltf) instead.";
			}

			return FindFirstMesh(loadedObject);
		}

		std::shared_ptr<Mesh> LoadMeshWithAssimp(const std::string& path)
		{
			const auto fullPath = ArkEngine::GetInstance().GetFileSystem().GetAssetFilePath(path);

			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(
				fullPath.string(),
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices |
				aiProcess_GenSmoothNormals |
				aiProcess_FlipUVs);

			if (!scene || !scene->HasMeshes())
			{
				return nullptr;
			}

			const aiMesh* sourceMesh = scene->mMeshes[0];
			if (!sourceMesh || !sourceMesh->HasPositions())
			{
				return nullptr;
			}

			VertexLayout vertexLayout;
			AddVertexElement(vertexLayout, VertexElement::PositionIndex, 3);

			const bool hasNormals = sourceMesh->HasNormals();
			const bool hasUVs = sourceMesh->HasTextureCoords(0);
			if (hasNormals)
			{
				AddVertexElement(vertexLayout, VertexElement::NormalIndex, 3);
			}
			if (hasUVs)
			{
				AddVertexElement(vertexLayout, VertexElement::UVIndex, 2);
			}

			std::vector<float> vertices;
			vertices.reserve(sourceMesh->mNumVertices * (vertexLayout.stride / sizeof(float)));

			for (unsigned int i = 0; i < sourceMesh->mNumVertices; ++i)
			{
				const auto& position = sourceMesh->mVertices[i];
				vertices.push_back(position.x);
				vertices.push_back(position.y);
				vertices.push_back(position.z);

				if (hasNormals)
				{
					const auto& normal = sourceMesh->mNormals[i];
					vertices.push_back(normal.x);
					vertices.push_back(normal.y);
					vertices.push_back(normal.z);
				}

				if (hasUVs)
				{
					const auto& uv = sourceMesh->mTextureCoords[0][i];
					vertices.push_back(uv.x);
					vertices.push_back(uv.y);
				}
			}

			std::vector<uint32_t> indices;
			indices.reserve(sourceMesh->mNumFaces * 3);

			for (unsigned int i = 0; i < sourceMesh->mNumFaces; ++i)
			{
				const aiFace& face = sourceMesh->mFaces[i];
				if (face.mNumIndices != 3)
				{
					continue;
				}

				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}

			if (!indices.empty())
			{
				return std::make_shared<Mesh>(vertexLayout, vertices, indices);
			}

			return std::make_shared<Mesh>(vertexLayout, vertices);
		}

		std::shared_ptr<Mesh> LoadMesh(const std::string& path)
		{
			const auto extension = ToLower(std::filesystem::path(path).extension().string());
			if (extension == ".gltf" || extension == ".glb")
			{
				return LoadMeshFromGLTF(path);
			}

			return LoadMeshWithAssimp(path);
		}
	}

	std::shared_ptr<Mesh> MeshManager::GetOrLoadMesh(const std::string& path)
	{
		auto it = m_meshes.find(path);
		if (it != m_meshes.end())
		{
			return it->second;
		}

		auto mesh = path.empty() ? nullptr : LoadMesh(path);
		m_meshes[path] = mesh;
		return mesh;
	}
}