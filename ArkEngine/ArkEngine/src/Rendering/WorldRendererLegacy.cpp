#include "WorldRendererLegacy.h"

#include <filesystem>

#include <glad/glad.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "ECS/Common/Basic_Mesh.h"
#include "ECS/Common/Mesh_Common.h"
#include "ECS/SkinnedMesh/Skinned_Mesh.h"
#include "Logger.h"

namespace Ark::Rendering
{
	namespace
	{
		class ViewportRenderCallbacks final : public IRenderCallbacks
		{
		public:
			ViewportRenderCallbacks(Shader& shader, Texture* albedoTexture, GLuint fallbackWhiteTex)
				: m_shader(shader),
				m_albedoTexture(albedoTexture),
				m_fallbackWhiteTex(fallbackWhiteTex)
			{
			}

			void DisableDiffuseTexture() override
			{
				m_albedoTexture = nullptr;
			}

			void DrawStartCB(uint DrawIndex) override
			{
				(void)DrawIndex;

				m_shader.SetInt("uAlbedoTexture", 0);

				if (m_albedoTexture)
				{
					m_albedoTexture->Bind(GL_TEXTURE0);
					m_shader.SetInt("uHasAlbedoTexture", 1);
				}
				else
				{
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, m_fallbackWhiteTex);
					m_shader.SetInt("uHasAlbedoTexture", 0);
				}
			}

		private:
			Shader& m_shader;
			Texture* m_albedoTexture = nullptr;
			GLuint m_fallbackWhiteTex = 0;
		};
	}

	WorldRendererLegacy::WorldRendererLegacy() = default;

	WorldRendererLegacy::~WorldRendererLegacy()
	{
		DestroyWhiteFallbackTexture();

		if (m_dummyVao != 0)
		{
			glDeleteVertexArrays(1, &m_dummyVao);
			m_dummyVao = 0;
		}
	}

	void WorldRendererLegacy::EnsureWhiteFallbackTexture()
	{
		if (m_whiteFallbackTex != 0)
			return;

		glGenTextures(1, &m_whiteFallbackTex);
		glBindTexture(GL_TEXTURE_2D, m_whiteFallbackTex);

		const unsigned char pixel[4] = { 255, 255, 255, 255 };

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA8,
			1,
			1,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pixel);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void WorldRendererLegacy::DestroyWhiteFallbackTexture()
	{
		if (m_whiteFallbackTex == 0)
			return;

		glDeleteTextures(1, &m_whiteFallbackTex);
		m_whiteFallbackTex = 0;
	}

	bool WorldRendererLegacy::Initialize()
	{
		if (m_initialized)
			return true;

		if (m_dummyVao == 0)
			glGenVertexArrays(1, &m_dummyVao);

		EnsureWhiteFallbackTexture();

		m_grid.Init();

		if (!m_shadowTech.Init())
		{
			Logging::Error() << "WorldRendererLegacy: ShadowMappingTechnique init failed.\n";
			return false;
		}

		if (!m_skinningTech.Init())
		{
			Logging::Error() << "WorldRendererLegacy: SkinningTechnique init failed.\n";
			return false;
		}

		try
		{
			m_viewportShader.LoadFromFiles(
				"Resources/Shaders/viewport_textured.vert",
				"Resources/Shaders/viewport_textured.frag");
		}
		catch (const std::exception& e)
		{
			Logging::Error() << "WorldRendererLegacy: Failed to load viewport shader: " << e.what() << "\n";
			return false;
		}

		constexpr unsigned int kShadowMapSize = 2048;
		if (!m_shadowFbo.Init(kShadowMapSize, kShadowMapSize, true))
		{
			Logging::Error() << "WorldRendererLegacy: shadow FBO init failed.\n";
			return false;
		}

		m_viewportFbo.Init(static_cast<int>(m_width), static_cast<int>(m_height), 4, true, true, false);

		m_initialized = true;
		return true;
	}

	Matrix4f WorldRendererLegacy::ToMatrix4f(const glm::mat4& m)
	{
		return Matrix4f(m);
	}

	void WorldRendererLegacy::Render(const WorldRenderInput& input)
	{
		if (!m_initialized)
		{
			m_width = input.width < 1 ? 1u : input.width;
			m_height = input.height < 1 ? 1u : input.height;

			if (!Initialize())
				return;
		}

		if (input.width != m_width || input.height != m_height)
			Resize(input.width, input.height);

		PersProjInfo proj{};
		proj.FOV = input.camera.fovDeg;
		proj.Width = static_cast<float>(m_width);
		proj.Height = static_cast<float>(m_height);
		proj.zNear = input.camera.nearPlane;
		proj.zFar = input.camera.farPlane;
		m_pipeline.SetPerspectiveProj(proj);
			
		const float pitch = input.camera.pitchYawDeg.x;
		const float yaw = input.camera.pitchYawDeg.y;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(front);

		const glm::vec3 camPosGlm = input.camera.position;
		const glm::vec3 camTargetGlm = camPosGlm + front;
		const glm::vec3 camUpGlm(0.0f, 1.0f, 0.0f);

		const float aspect = (m_height > 0) ? (static_cast<float>(m_width) / static_cast<float>(m_height)) : 1.0f;

		const glm::mat4 projGlm = glm::perspectiveLH_ZO(
			glm::radians(input.camera.fovDeg),
			aspect,
			input.camera.nearPlane,
			input.camera.farPlane);

		const glm::mat4 viewGlm = glm::lookAtLH(camPosGlm, camTargetGlm, camUpGlm);
		const glm::mat4 vpGlm = projGlm * viewGlm;

		const Vector3f camPos(input.camera.position);
		m_pipeline.SetCamera(camPos, Vector3f(front), Vector3f(camUpGlm));

		m_viewportFbo.BindForWriting();
		m_viewportFbo.Clear();

		if (input.wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBindVertexArray(m_dummyVao);

		if (input.showGrid)
		{
			const Matrix4f VP = m_pipeline.GetVPTrans();
			m_grid.Render(m_gridCfg, VP, camPos);
		}

		uint32_t triangleCount = 0;

		for (const RenderInstance& inst : input.instances)
		{
			const glm::mat4 model = inst.model;
			const glm::mat4 mvp = vpGlm * model;

			if (inst.meshType == RenderMeshType::Static)
			{
				BasicMesh* mesh = GetOrLoadStaticMesh(inst.meshPath);
				if (!mesh)
					continue;

				triangleCount += mesh->GetTriangleCount();

				const GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
				GLint oldCullFaceMode = GL_BACK;
				GLint oldFrontFace = GL_CCW;

				glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFaceMode);
				glGetIntegerv(GL_FRONT_FACE, &oldFrontFace);

				glDisable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				glFrontFace(GL_CW);

				Texture* albedo = nullptr;

				if (!inst.albedoTexturePath.empty())
					albedo = m_textureCache.GetOrLoad2D(inst.albedoTexturePath, false, input.useMipmaps);

				if (!albedo)
					albedo = mesh->GetPBRAlbedoTexture();

				if (!albedo)
					albedo = mesh->GetDiffuseTexture();

				m_viewportShader.Bind();

				m_viewportShader.SetInt("uRenderMode", 1);

				m_viewportShader.SetMat4("uMVP", mvp);
				m_viewportShader.SetMat4("uModel", model);

				m_viewportShader.SetVec3("u_Tint", inst.tint);
				m_viewportShader.SetVec3("uCameraPos", input.camera.position);

				m_viewportShader.SetFloat("uAmbientStrength", input.ambientStrength);
				m_viewportShader.SetFloat("uExposure", input.exposure);

				m_viewportShader.SetInt("uMaterialPreset", 0);

				const int lightCount = std::min(static_cast<int>(input.pointLights.size()), 16);
				m_viewportShader.SetInt("uPointLightCount", lightCount);
				for (int i = 0; i < lightCount; ++i)
				{
					const auto& l = input.pointLights[static_cast<size_t>(i)];
					m_viewportShader.SetVec3(std::string("uPointLightPos[" + std::to_string(i) + "]"), l.position);
					m_viewportShader.SetVec3(std::string("uPointLightColor[" + std::to_string(i) + "]"), l.color);
					m_viewportShader.SetFloat(std::string("uPointLightIntensity[" + std::to_string(i) + "]"), l.intensity);
					m_viewportShader.SetFloat(std::string("uPointLightRadius[" + std::to_string(i) + "]"), l.radius);
				}

				EnsureWhiteFallbackTexture();
				ViewportRenderCallbacks callbacks(m_viewportShader, albedo, m_whiteFallbackTex);
				mesh->Render(&callbacks);

				glCullFace(static_cast<GLenum>(oldCullFaceMode));
				glFrontFace(static_cast<GLenum>(oldFrontFace));

				if (!cullWasEnabled)
					glDisable(GL_CULL_FACE);
				else
					glEnable(GL_CULL_FACE);
			}
			else
			{
				const Matrix4f WorldM = ToMatrix4f(model);
				const Matrix4f WVP = ToMatrix4f(mvp);

				SkinnedMesh* m = GetOrLoadSkeletalMesh(inst.meshPath);
				if (!m)
					continue;

				m_skinningTech.Enable();
				m_skinningTech.SetWVP(WVP);
				m_skinningTech.SetWorldMatrix(WorldM);

				const unsigned int animIndex = (inst.animationIndex < 0) ? 0u : static_cast<unsigned int>(inst.animationIndex);

				m_boneTransforms.clear();
				m->GetBoneTransforms(inst.animTimeSec, m_boneTransforms, animIndex);

				for (uint i = 0; i < static_cast<uint>(m_boneTransforms.size()); ++i)
					m_skinningTech.SetBoneTransform(i, m_boneTransforms[i]);

				m->Render(nullptr);
			}
		}

		m_lastTriangleCount = triangleCount;

		glBindVertexArray(0);
		m_viewportFbo.UnbindWriting();
	}

	bool WorldRendererLegacy::Resize(uint32_t width, uint32_t height)
	{
		m_width = width < 1 ? 1u : width;
		m_height = height < 1 ? 1u : height;

		m_viewportFbo.Init(static_cast<int>(m_width), static_cast<int>(m_height), 4, true, true, false);
		return true;
	}

	BasicMesh* WorldRendererLegacy::GetOrLoadStaticMesh(const std::string& meshPath)
	{
		if (meshPath.empty())
			return nullptr;

		auto it = m_staticMeshes.find(meshPath);
		if (it != m_staticMeshes.end())
			return it->second.get();

		auto mesh = std::make_unique<BasicMesh>();
		if (!mesh->LoadMesh(meshPath, 0))
		{
			Logging::Error() << "WorldRendererLegacy: failed to load static mesh '" << meshPath << "'\n";
			return nullptr;
		}

		BasicMesh* raw = mesh.get();
		m_staticMeshes.emplace(meshPath, std::move(mesh));
		return raw;
	}

	SkinnedMesh* WorldRendererLegacy::GetOrLoadSkeletalMesh(const std::string& meshPath)
	{
		if (meshPath.empty())
			return nullptr;

		auto it = m_skeletalMeshes.find(meshPath);
		if (it != m_skeletalMeshes.end())
			return it->second.get();

		auto mesh = std::make_unique<SkinnedMesh>();
		if (!mesh->LoadMesh(meshPath))
		{
			Logging::Error() << "WorldRendererLegacy: failed to load skinned mesh '" << meshPath << "'\n";
			return nullptr;
		}

		SkinnedMesh* raw = mesh.get();
		m_skeletalMeshes.emplace(meshPath, std::move(mesh));
		return raw;
	}
}