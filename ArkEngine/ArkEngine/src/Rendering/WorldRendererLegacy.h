#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "Common/Pipeline.h"
#include "Maps/InfiniteGrid/InfiniteGrid.h"
#include "Maps/Techniques/Shadow_Mapping_Technique.h"
#include "Maps/Techniques/Skinning_Technique.h"
#include "Rendering/Cache/TextureCache.h"
#include "Rendering/Framebuffer/Framebuffer.h"
#include "Rendering/Framebuffer/Framebuffer_Object.h"
#include "Rendering/Threading/WorldRenderThread.h"
#include "Shader.h"

class BasicMesh;
class SkinnedMesh;

namespace Ark::Rendering
{
	class WorldRendererLegacy
	{
	public:
		WorldRendererLegacy();
		~WorldRendererLegacy();

		bool Initialize();
		bool Resize(uint32_t width, uint32_t height);
		void Render(const WorldRenderInput& input);

		uint32_t GetOutputTextureId() const { return static_cast<uint32_t>(m_viewportFbo.GetTexture()); }
		uint32_t GetLastTriangleCount() const { return m_lastTriangleCount; }

	private:
		BasicMesh* GetOrLoadStaticMesh(const std::string& meshPath);
		SkinnedMesh* GetOrLoadSkeletalMesh(const std::string& meshPath);

		static Matrix4f ToMatrix4f(const glm::mat4& m);

		void EnsureWhiteFallbackTexture();
		void DestroyWhiteFallbackTexture();

	private:
		uint32_t m_width = 1;
		uint32_t m_height = 1;

		Framebuffer m_viewportFbo;

		InfiniteGrid m_grid;
		InfiniteGridConfig m_gridCfg{};

		ShadowMappingTechnique m_shadowTech;
		FramebufferObject m_shadowFbo;

		SkinningTechnique m_skinningTech;

		bool m_initialized = false;

		Pipeline m_pipeline;

		std::unordered_map<std::string, std::unique_ptr<BasicMesh>> m_staticMeshes;
		std::unordered_map<std::string, std::unique_ptr<SkinnedMesh>> m_skeletalMeshes;

		std::vector<Matrix4f> m_boneTransforms;

		GLuint m_dummyVao = 0;

		Shader m_viewportShader;
		TextureCache m_textureCache{ nullptr };

		GLuint m_whiteFallbackTex = 0;

		uint32_t m_lastTriangleCount = 0;
	};
}