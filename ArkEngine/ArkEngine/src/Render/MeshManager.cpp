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
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <utility>
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

		constexpr GLuint MeshAttributeIndices[] =
		{
			VertexElement::PositionIndex,
			VertexElement::ColorIndex,
			VertexElement::UVIndex,
			VertexElement::NormalIndex
		};

		struct MeshData
		{
			VertexLayout layout;
			std::vector<float> vertices;
			std::vector<uint32_t> indices;
			size_t vertexCount = 0;
		};

		float GetDefaultAttributeValue(GLuint attributeIndex, GLuint componentIndex)
		{
			if (attributeIndex == VertexElement::ColorIndex)
			{
				return 1.0f;
			}

			if (attributeIndex == VertexElement::NormalIndex && componentIndex == 2)
			{
				return 1.0f;
			}

			return 0.0f;
		}

		bool ExtractMeshData(const std::shared_ptr<Mesh>& source, const glm::mat4& transform, MeshData& outData)
		{
			if (!source)
			{
				return false;
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

			for (GLuint index : MeshAttributeIndices)
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
				return false;
			}

			glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(vertexBuffer));
			GLint vertexBufferSize = 0;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vertexBufferSize);
			if (vertexBufferSize <= 0 || vertexBufferSize % sizeof(float) != 0)
			{
				glBindVertexArray(static_cast<GLuint>(previousVertexArray));
				glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previousArrayBuffer));
				return false;
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

			outData.layout = vertexLayout;
			outData.vertices = std::move(vertices);
			outData.indices = std::move(indices);
			outData.vertexCount = vertexCount;
			return true;
		}

		VertexLayout BuildMergedLayout(const std::vector<MeshData>& meshes)
		{
			VertexLayout mergedLayout;

			for (GLuint attributeIndex : MeshAttributeIndices)
			{
				GLuint componentCount = 0;
				for (const auto& mesh : meshes)
				{
					if (const VertexElement* element = FindElement(mesh.layout, attributeIndex))
					{
						componentCount = std::max(componentCount, element->size);
					}
				}

				if (componentCount == 0)
				{
					continue;
				}

				VertexElement element;
				element.index = attributeIndex;
				element.size = componentCount;
				element.type = GL_FLOAT;
				element.offset = mergedLayout.stride;
				mergedLayout.stride += element.size * sizeof(float);
				mergedLayout.elements.push_back(element);
			}

			return mergedLayout;
		}

		std::shared_ptr<Mesh> BuildMergedMesh(const std::vector<MeshData>& meshes)
		{
			if (meshes.empty())
			{
				return nullptr;
			}

			const VertexLayout mergedLayout = BuildMergedLayout(meshes);
			const size_t mergedStrideFloats = mergedLayout.stride / sizeof(float);
			if (!FindElement(mergedLayout, VertexElement::PositionIndex) || mergedStrideFloats == 0)
			{
				return nullptr;
			}

			size_t totalVertexCount = 0;
			size_t totalIndexCount = 0;
			for (const auto& mesh : meshes)
			{
				totalVertexCount += mesh.vertexCount;
				totalIndexCount += mesh.indices.empty() ? mesh.vertexCount : mesh.indices.size();
			}

			if (totalVertexCount > static_cast<size_t>(std::numeric_limits<uint32_t>::max()))
			{
				Logging::Error() << "MeshManager: merged mesh exceeds the 32-bit index limit.";
				return nullptr;
			}

			std::vector<float> mergedVertices;
			std::vector<uint32_t> mergedIndices;
			mergedVertices.reserve(totalVertexCount * mergedStrideFloats);
			mergedIndices.reserve(totalIndexCount);

			uint32_t vertexOffset = 0;
			for (const auto& mesh : meshes)
			{
				const size_t sourceStrideFloats = mesh.layout.stride / sizeof(float);
				if (sourceStrideFloats == 0)
				{
					continue;
				}

				for (size_t vertexIndex = 0; vertexIndex < mesh.vertexCount; ++vertexIndex)
				{
					for (const auto& destinationElement : mergedLayout.elements)
					{
						const VertexElement* sourceElement = FindElement(mesh.layout, destinationElement.index);
						for (GLuint componentIndex = 0; componentIndex < destinationElement.size; ++componentIndex)
						{
							float value = GetDefaultAttributeValue(destinationElement.index, componentIndex);
							if (sourceElement && componentIndex < sourceElement->size)
							{
								const size_t sourceIndex =
									vertexIndex * sourceStrideFloats +
									(sourceElement->offset / sizeof(float)) +
									componentIndex;
								value = mesh.vertices[sourceIndex];
							}

							mergedVertices.push_back(value);
						}
					}
				}

				if (!mesh.indices.empty())
				{
					for (uint32_t index : mesh.indices)
					{
						mergedIndices.push_back(vertexOffset + index);
					}
				}
				else
				{
					for (uint32_t index = 0; index < mesh.vertexCount; ++index)
					{
						mergedIndices.push_back(vertexOffset + index);
					}
				}

				vertexOffset += static_cast<uint32_t>(mesh.vertexCount);
			}

			if (!mergedIndices.empty())
			{
				return std::make_shared<Mesh>(mergedLayout, mergedVertices, mergedIndices);
			}

			return std::make_shared<Mesh>(mergedLayout, mergedVertices);
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

		glm::mat4 ToGlm(const aiMatrix4x4& matrix)
		{
			return glm::mat4(
				matrix.a1, matrix.b1, matrix.c1, matrix.d1,
				matrix.a2, matrix.b2, matrix.c2, matrix.d2,
				matrix.a3, matrix.b3, matrix.c3, matrix.d3,
				matrix.a4, matrix.b4, matrix.c4, matrix.d4);
		}

		bool CreateMeshDataFromAssimp(const aiMesh* sourceMesh, const glm::mat4& transform, MeshData& outData)
		{
			if (!sourceMesh || !sourceMesh->HasPositions())
			{
				return false;
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

			const glm::mat3 normalTransform = glm::transpose(glm::inverse(glm::mat3(transform)));
			for (unsigned int i = 0; i < sourceMesh->mNumVertices; ++i)
			{
				const auto& position = sourceMesh->mVertices[i];
				const glm::vec4 transformedPosition = transform * glm::vec4(position.x, position.y, position.z, 1.0f);
				vertices.push_back(transformedPosition.x);
				vertices.push_back(transformedPosition.y);
				vertices.push_back(transformedPosition.z);

				if (hasNormals)
				{
					const auto& normal = sourceMesh->mNormals[i];
					const glm::vec3 transformedNormal = normalTransform * glm::vec3(normal.x, normal.y, normal.z);
					const float length = glm::length(transformedNormal);
					const glm::vec3 normalizedNormal = length > 0.0f
						? transformedNormal / length
						: glm::vec3(0.0f, 0.0f, 1.0f);
					vertices.push_back(normalizedNormal.x);
					vertices.push_back(normalizedNormal.y);
					vertices.push_back(normalizedNormal.z);
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

			outData.layout = vertexLayout;
			outData.vertices = std::move(vertices);
			outData.indices = std::move(indices);
			outData.vertexCount = sourceMesh->mNumVertices;
			return true;
		}

		void CollectAssimpMeshes(const aiScene* scene, const aiNode* node, const glm::mat4& parentTransform, std::vector<MeshData>& meshes)
		{
			if (!scene || !node)
			{
				return;
			}

			const glm::mat4 transform = parentTransform * ToGlm(node->mTransformation);
			for (unsigned int i = 0; i < node->mNumMeshes; ++i)
			{
				const unsigned int meshIndex = node->mMeshes[i];
				if (meshIndex >= scene->mNumMeshes)
				{
					continue;
				}

				MeshData meshData;
				if (CreateMeshDataFromAssimp(scene->mMeshes[meshIndex], transform, meshData))
				{
					meshes.push_back(std::move(meshData));
				}
			}

			for (unsigned int i = 0; i < node->mNumChildren; ++i)
			{
				CollectAssimpMeshes(scene, node->mChildren[i], transform, meshes);
			}
		}

		void CollectMeshes(GameObject* object, std::vector<MeshData>& meshes)
		{
			if (!object)
			{
				return;
			}

			for (const auto& component : object->GetComponents())
			{
				auto* meshComponent = dynamic_cast<MeshComponent*>(component.get());
				if (!meshComponent)
				{
					continue;
				}

				MeshData meshData;
				if (ExtractMeshData(meshComponent->GetMesh(), object->GetWorldTransform(), meshData))
				{
					meshes.push_back(std::move(meshData));
				}
			}

			for (const auto& child : object->GetChildren())
			{
				CollectMeshes(child.get(), meshes);
			}
		}

		std::shared_ptr<Mesh> LoadMeshFromGLTF(const std::string& path)
		{
			Scene loadScene;
			GameObject* loadedObject = GameObject::LoadGLTF(path, &loadScene);
			if (!loadedObject) return nullptr;

			std::vector<MeshData> meshes;
			CollectMeshes(loadedObject, meshes);

			if (meshes.size() > 1)
			{
				Logging::Debug() << "MeshManager: '" << path << "' contains " << meshes.size()
					<< " meshes. Merging them into a single mesh.";
			}

			return BuildMergedMesh(meshes);
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

			std::vector<MeshData> meshes;
			if (scene->mRootNode)
			{
				CollectAssimpMeshes(scene, scene->mRootNode, glm::mat4(1.0f), meshes);
			}
			else
			{
				for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
				{
					MeshData meshData;
					if (CreateMeshDataFromAssimp(scene->mMeshes[i], glm::mat4(1.0f), meshData))
					{
						meshes.push_back(std::move(meshData));
					}
				}
			}

			if (meshes.size() > 1)
			{
				Logging::Debug() << "MeshManager: '" << path << "' contains " << meshes.size()
					<< " meshes. Merging them into a single mesh.";
			}

			return BuildMergedMesh(meshes);
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