#include "WorldRendererLegacy.h"

#include <filesystem>

#include "ECS/Common/Basic_Mesh.h"
#include "ECS/SkinnedMesh/Skinned_Mesh.h"
#include "Logger.h"

namespace Ark::Rendering
{
	WorldRendererLegacy::WorldRendererLegacy() = default;

	WorldRendererLegacy::~WorldRendererLegacy()
	{
		if (m_dummyVao != 0)
		{
			glDeleteVertexArrays(1, &m_dummyVao);
			m_dummyVao = 0;
		}
	}

	bool WorldRendererLegacy::Initialize()
	{
		if (m_initialized)
			return true;

		if (m_dummyVao == 0)
			glGenVertexArrays(1, &m_dummyVao);

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

		const Vector3f camPos(input.camera.position);

		// Compute target direction from pitch/yaw (like ArkCamera)
		const float pitch = input.camera.pitchYawDeg.x;
		const float yaw = input.camera.pitchYawDeg.y;
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(front);

		const Vector3f camTarget(glm::vec3(input.camera.position) + front);
		const Vector3f camUp(0.0f, 1.0f, 0.0f);

		m_pipeline.SetCamera(camPos, camTarget - camPos, camUp);

		m_viewportFbo.BindForWriting();
		m_viewportFbo.Clear();

		if (input.wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Ensure a VAO is bound for core profile draws (grid uses glDrawArrays*)
		glBindVertexArray(m_dummyVao);

		// Grid
		{
			const Matrix4f VP = m_pipeline.GetVPTrans();
			m_grid.Render(m_gridCfg, VP, camPos);
		}

		for (const RenderInstance& inst : input.instances)
		{
			// Pipeline expects TRS, so feed it directly from glm model.
			// NOTE: this is a limitation of the current Pipeline API.
			// If needed, we can add a `Pipeline::SetWorldMatrix(Matrix4f)` later.
			const Matrix4f world = ToMatrix4f(inst.model);
			(void)world;

			// Approximate by pulling TRS out of glm and feeding Pipeline.
			// This keeps correct WVP for editor transforms.
			glm::vec3 t{}, s{}, eulerDeg{};
			{
				// cheap TRS approximation: translation and uniform-ish scale
				t = glm::vec3(inst.model[3]);
				s = glm::vec3(
					glm::length(glm::vec3(inst.model[0])),
					glm::length(glm::vec3(inst.model[1])),
					glm::length(glm::vec3(inst.model[2])));
				eulerDeg = glm::vec3(0.0f);
			}

			m_pipeline.WorldPos(t.x, t.y, t.z);
			m_pipeline.Scale(s.x, s.y, s.z);
			m_pipeline.Rotate(eulerDeg.x, eulerDeg.y, eulerDeg.z);

			const Matrix4f WVP = m_pipeline.GetWVPTrans();
			const Matrix4f WorldM = m_pipeline.GetWorldTrans();

			if (inst.meshType == RenderMeshType::Static)
			{
				if (BasicMesh* m = GetOrLoadStaticMesh(inst.meshPath))
				{
					m->Render(nullptr);
				}
			}
			else
			{
				SkinnedMesh* m = GetOrLoadSkeletalMesh(inst.meshPath);
				if (!m)
					continue;

				m_skinningTech.Enable();
				m_skinningTech.SetWVP(WVP);
				m_skinningTech.SetWorldMatrix(WorldM);
				// NOTE: SkinningTechnique doesn't expose SetEyeWorldPos in this codebase.

				const unsigned int animIndex = (inst.animationIndex < 0) ? 0u : static_cast<unsigned int>(inst.animationIndex);

				m_boneTransforms.clear();
				m->GetBoneTransforms(inst.animTimeSec, m_boneTransforms, animIndex);

				for (uint i = 0; i < static_cast<uint>(m_boneTransforms.size()); ++i)
					m_skinningTech.SetBoneTransform(i, m_boneTransforms[i]);

				m->Render(nullptr);
			}
		}

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