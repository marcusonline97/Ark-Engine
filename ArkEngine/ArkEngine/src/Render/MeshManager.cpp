#include "Render/MeshManager.h"

#include "Core/ArkEngine.h"
#include "Graphics/VertexLayout.h"
#include "Logger.h"
#include "Render/Mesh.h"

#include <Assimp/Importer.hpp>
#include <Assimp/postprocess.h>
#include <Assimp/scene.h>
#include <cgltf/cgltf.h>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
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

		const VertexElement* FindElement(const VertexLayout& layout, GLuint index)
		{
			const auto it = std::find_if(layout.elements.begin(), layout.elements.end(), [index](const VertexElement& element)
				{
					return element.index == index;
				});
			return it != layout.elements.end() ? &(*it) : nullptr;
		}

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

		glm::mat4 GetGLTFNodeLocalTransform(const cgltf_node* node)
		{
			if (!node)
			{
				return glm::mat4(1.0f);
			}

			if (node->has_matrix)
			{
				return glm::make_mat4(node->matrix);
			}

			glm::mat4 transform(1.0f);
			if (node->has_translation)
			{
				transform = glm::translate(transform, glm::vec3(
					node->translation[0],
					node->translation[1],
					node->translation[2]));
			}

			if (node->has_rotation)
			{
				const glm::quat rotation(
					node->rotation[3],
					node->rotation[0],
					node->rotation[1],
					node->rotation[2]);
				transform *= glm::mat4_cast(rotation);
			}

			if (node->has_scale)
			{
				transform = glm::scale(transform, glm::vec3(
					node->scale[0],
					node->scale[1],
					node->scale[2]));
			}

			return transform;
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

		bool ReadAccessorFloats(const cgltf_accessor* accessor, cgltf_size index, float* values, cgltf_size valueCount)
		{
			std::fill(values, values + valueCount, 0.0f);
			if (!accessor)
			{
				return false;
			}

			return cgltf_accessor_read_float(accessor, index, values, valueCount) == 1;
		}

		bool CreateMeshDataFromGLTFPrimitive(const cgltf_primitive& primitive, const glm::mat4& transform, MeshData& outData)
		{
			if (primitive.type != cgltf_primitive_type_triangles)
			{
				return false;
			}

			const cgltf_accessor* positionAccessor = nullptr;
			const cgltf_accessor* colorAccessor = nullptr;
			const cgltf_accessor* uvAccessor = nullptr;
			const cgltf_accessor* normalAccessor = nullptr;

			for (cgltf_size i = 0; i < primitive.attributes_count; ++i)
			{
				const cgltf_attribute& attribute = primitive.attributes[i];
				if (!attribute.data)
				{
					continue;
				}

				switch (attribute.type)
				{
				case cgltf_attribute_type_position:
					positionAccessor = attribute.data;
					break;
				case cgltf_attribute_type_color:
					if (attribute.index == 0)
					{
						colorAccessor = attribute.data;
					}
					break;
				case cgltf_attribute_type_texcoord:
					if (attribute.index == 0)
					{
						uvAccessor = attribute.data;
					}
					break;
				case cgltf_attribute_type_normal:
					normalAccessor = attribute.data;
					break;
				default:
					break;
				}
			}

			if (!positionAccessor || positionAccessor->count == 0)
			{
				return false;
			}

			VertexLayout vertexLayout;
			AddVertexElement(vertexLayout, VertexElement::PositionIndex, 3);
			if (colorAccessor)
			{
				AddVertexElement(vertexLayout, VertexElement::ColorIndex, 3);
			}
			if (uvAccessor)
			{
				AddVertexElement(vertexLayout, VertexElement::UVIndex, 2);
			}
			if (normalAccessor)
			{
				AddVertexElement(vertexLayout, VertexElement::NormalIndex, 3);
			}

			const size_t strideFloats = vertexLayout.stride / sizeof(float);
			std::vector<float> vertices;
			vertices.reserve(positionAccessor->count * strideFloats);

			const glm::mat3 normalTransform = glm::transpose(glm::inverse(glm::mat3(transform)));
			for (cgltf_size vertexIndex = 0; vertexIndex < positionAccessor->count; ++vertexIndex)
			{
				float values[4] = {};
				if (!ReadAccessorFloats(positionAccessor, vertexIndex, values, 4))
				{
					return false;
				}

				const glm::vec4 transformedPosition = transform * glm::vec4(values[0], values[1], values[2], 1.0f);
				vertices.push_back(transformedPosition.x);
				vertices.push_back(transformedPosition.y);
				vertices.push_back(transformedPosition.z);

				if (colorAccessor)
				{
					ReadAccessorFloats(colorAccessor, vertexIndex, values, 4);
					vertices.push_back(values[0]);
					vertices.push_back(values[1]);
					vertices.push_back(values[2]);
				}

				if (uvAccessor)
				{
					ReadAccessorFloats(uvAccessor, vertexIndex, values, 4);
					vertices.push_back(values[0]);
					vertices.push_back(values[1]);
				}

				if (normalAccessor)
				{
					ReadAccessorFloats(normalAccessor, vertexIndex, values, 4);
					const glm::vec3 transformedNormal = normalTransform * glm::vec3(values[0], values[1], values[2]);
					const float length = glm::length(transformedNormal);
					const glm::vec3 normalizedNormal = length > 0.0f
						? transformedNormal / length
						: glm::vec3(0.0f, 0.0f, 1.0f);
					vertices.push_back(normalizedNormal.x);
					vertices.push_back(normalizedNormal.y);
					vertices.push_back(normalizedNormal.z);
				}
			}

			std::vector<uint32_t> indices;
			if (primitive.indices)
			{
				indices.reserve(primitive.indices->count);
				for (cgltf_size i = 0; i < primitive.indices->count; ++i)
				{
					const cgltf_size index = cgltf_accessor_read_index(primitive.indices, i);
					if (index > static_cast<cgltf_size>(std::numeric_limits<uint32_t>::max()))
					{
						Logging::Error() << "MeshManager: GLTF mesh index exceeds the 32-bit index limit.";
						return false;
					}

					indices.push_back(static_cast<uint32_t>(index));
				}
			}

			outData.layout = vertexLayout;
			outData.vertices = std::move(vertices);
			outData.indices = std::move(indices);
			outData.vertexCount = positionAccessor->count;
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

		void CollectGLTFMeshes(const cgltf_node* node, const glm::mat4& parentTransform, std::vector<MeshData>& meshes)
		{
			if (!node)
			{
				return;
			}

			const glm::mat4 transform = parentTransform * GetGLTFNodeLocalTransform(node);
			if (node->mesh)
			{
				for (cgltf_size i = 0; i < node->mesh->primitives_count; ++i)
				{
					MeshData meshData;
					if (CreateMeshDataFromGLTFPrimitive(node->mesh->primitives[i], transform, meshData))
					{
						meshes.push_back(std::move(meshData));
					}
				}
			}

			for (cgltf_size i = 0; i < node->children_count; ++i)
			{
				CollectGLTFMeshes(node->children[i], transform, meshes);
			}
		}

		std::shared_ptr<Mesh> LoadMeshFromGLTF(const std::string& path)
		{
			auto& fileSystem = ArkEngine::GetInstance().GetFileSystem();
			const auto contents = fileSystem.LoadAssetFile(path);
			if (contents.empty())
			{
				return nullptr;
			}

			cgltf_options options = {};
			cgltf_data* data = nullptr;
			cgltf_result result = cgltf_parse(&options, contents.data(), contents.size(), &data);
			if (result != cgltf_result_success || !data)
			{
				return nullptr;
			}

			const auto fullPath = fileSystem.GetAssetFilePath(path);
			result = cgltf_load_buffers(&options, data, fullPath.string().c_str());
			if (result != cgltf_result_success)
			{
				cgltf_free(data);
				return nullptr;
			}

			std::vector<MeshData> meshes;
			const cgltf_scene* scene = data->scene
				? data->scene
				: (data->scenes_count > 0 ? &data->scenes[0] : nullptr);
			if (scene)
			{
				for (cgltf_size i = 0; i < scene->nodes_count; ++i)
				{
					CollectGLTFMeshes(scene->nodes[i], glm::mat4(1.0f), meshes);
				}
			}
			else
			{
				for (cgltf_size i = 0; i < data->nodes_count; ++i)
				{
					if (!data->nodes[i].parent)
					{
						CollectGLTFMeshes(&data->nodes[i], glm::mat4(1.0f), meshes);
					}
				}
			}

			if (meshes.size() > 1)
			{
				Logging::Debug() << "MeshManager: '" << path << "' contains " << meshes.size()
					<< " meshes. Merging them into a single mesh.";
			}

			auto mesh = BuildMergedMesh(meshes);
			cgltf_free(data);
			return mesh;
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