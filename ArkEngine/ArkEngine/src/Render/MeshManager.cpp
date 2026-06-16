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
				return meshComponent->GetMesh();
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