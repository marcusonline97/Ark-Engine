#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

namespace Ark::Physics
{
	enum class MotionType : uint8_t
	{
		Static = 0,
		Dynamic = 1,
		Kinematic = 2,
	};

	struct BodyConfig
	{
		uint32_t objectId = 0;
		MotionType motionType = MotionType::Dynamic;
		bool useGravity = true;

		glm::vec3 position{ 0.0f };
		glm::vec3 rotationDeg{ 0.0f };
		glm::vec3 halfExtents{ 0.5f, 0.5f, 0.5f };
	};

	struct BodyTransform
	{
		uint32_t objectId = 0;
		glm::vec3 position{ 0.0f };
		glm::vec3 rotationDeg{ 0.0f };
	};

	class PhysicsThreadedWorld
	{
	public:
		PhysicsThreadedWorld();
		~PhysicsThreadedWorld();

		PhysicsThreadedWorld(const PhysicsThreadedWorld&) = delete;
		PhysicsThreadedWorld& operator=(const PhysicsThreadedWorld&) = delete;

		// Submit complete physics body state from the game thread.
		void SubmitScene(const std::vector<BodyConfig>& bodies);

		// Controls whether the simulation advances (typically play mode only).
		void SetSimulationEnabled(bool enabled);

		// Latest transforms written by the physics thread.
		std::vector<BodyTransform> ConsumeLatestTransforms();

		bool IsInitialized() const { return m_initialized.load(std::memory_order_relaxed); }

	private:
		void ThreadMain();
		void RequestStop();

	private:
		std::thread m_thread;
		std::mutex m_mutex;
		std::condition_variable m_cv;

		bool m_stop = false;
		bool m_hasScene = false;
		bool m_simulationEnabled = false;

		std::vector<BodyConfig> m_pendingScene;
		std::vector<BodyTransform> m_latestTransforms;

		std::atomic<bool> m_initialized{ false };
	};
}