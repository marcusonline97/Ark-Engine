#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <optional>
#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace Ark::Rendering
{
	class ResourceLoader;

	struct WorldCameraInput
	{
		glm::vec3 position{ 0.0f, 0.0f, 3.0f };
		glm::vec2 pitchYawDeg{ 0.0f, -90.0f };

		float fovDeg = 45.0f;
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
	};

	enum class RenderMeshType : uint8_t
	{
		Static = 0,
		Skeletal = 1,
	};

	struct RenderInstance
	{
		glm::mat4 model{ 1.0f };
		glm::vec3 tint{ 1.0f, 1.0f, 1.0f };

		RenderMeshType meshType = RenderMeshType::Static;

		// Stable identity for per-object state on the render thread.
		// Must be set by the caller.
		uint32_t objectId = 0;

		std::string meshPath;
		std::string albedoTexturePath;

		// Skeletal-only (optional, ignored for static meshes)
		std::string animationPath;
		int animationIndex = -1;

		// Per-object accumulated animation time (seconds).
		float animTimeSec = 0.0f;

		bool hasMaterial = false;

		std::optional<bool> useMipmaps;
	};

	struct PointLightInput
	{
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 color{ 1.0f, 1.0f, 1.0f };
		float intensity = 1.0f;
		float radius = 1.0f;
	};

	struct WorldRenderInput
	{
		uint32_t width = 1;
		uint32_t height = 1;

		WorldCameraInput camera{};
		std::vector<RenderInstance> instances;

		std::vector<PointLightInput> pointLights;
		bool wireframe = false;

		float ambientStrength = 0.12f;
		float exposure = 1.35f;

		bool useMipmaps = true;

		bool showGrid = true;
	};

	class WorldRenderThread
	{
	public:
		explicit WorldRenderThread(GLFWwindow* shareWithWindow, ResourceLoader* cpuLoader);
		~WorldRenderThread();

		WorldRenderThread(const WorldRenderThread&) = delete;
		WorldRenderThread& operator=(const WorldRenderThread&) = delete;

		void Submit(const WorldRenderInput& input);

		uint32_t GetLatestTextureId() const { return m_latestTextureId.load(std::memory_order_relaxed); }


	private:
		void ThreadMain();
		void RequestStop();

	private:
		GLFWwindow* m_renderWindow = nullptr;

		ResourceLoader* m_cpuLoader = nullptr;

		std::thread m_thread;
		std::mutex m_mutex;
		std::condition_variable m_cv;

		WorldRenderInput m_input{};
		bool m_hasInput = false;
		bool m_stop = false;

		std::atomic<uint32_t> m_latestTextureId{ 0 };
	};
}