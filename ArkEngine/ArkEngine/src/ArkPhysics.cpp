#include "ArkPhysics.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/RegisterTypes.h>

namespace Ark::Physics
{
	namespace
	{
		using namespace JPH;

		namespace Layers
		{
			static constexpr ObjectLayer NonMoving = 0;
			static constexpr ObjectLayer Moving = 1;
			static constexpr ObjectLayer NumLayers = 2;
		}

		namespace BroadPhaseLayers
		{
			static constexpr BroadPhaseLayer NonMoving(0);
			static constexpr BroadPhaseLayer Moving(1);
			static constexpr uint NumLayers = 2;
		}

		class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
		{
		public:
			BPLayerInterfaceImpl()
			{
				mObjectToBroadPhase[Layers::NonMoving] = BroadPhaseLayers::NonMoving;
				mObjectToBroadPhase[Layers::Moving] = BroadPhaseLayers::Moving;
			}

			uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NumLayers; }

			BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
			{
				return mObjectToBroadPhase[inLayer];
			}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
			const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
			{
				switch ((BroadPhaseLayer::Type)inLayer)
				{
				case (BroadPhaseLayer::Type)BroadPhaseLayers::NonMoving: return "NON_MOVING";
				case (BroadPhaseLayer::Type)BroadPhaseLayers::Moving: return "MOVING";
				default: return "UNKNOWN";
				}
			}
#endif

		private:
			BroadPhaseLayer mObjectToBroadPhase[Layers::NumLayers];
		};

		class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
		{
		public:
			bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
			{
				if (inObject1 == Layers::NonMoving)
					return inObject2 == Layers::Moving;
				return true;
			}
		};

		class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
		{
		public:
			bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
			{
				if (inLayer1 == Layers::NonMoving)
					return inLayer2 == BroadPhaseLayers::Moving;
				return true;
			}
		};

		class JoltBootstrap
		{
		public:
			JoltBootstrap()
			{
				RegisterDefaultAllocator();
				Factory::sInstance = new Factory();
				RegisterTypes();
			}

			~JoltBootstrap()
			{
				UnregisterTypes();
				delete Factory::sInstance;
				Factory::sInstance = nullptr;
			}
		};

		static JoltBootstrap g_joltBootstrap;

		static constexpr float kFixedDt = 1.0f / 60.0f;
		static constexpr int kCollisionSteps = 1;
		static constexpr uint32_t kMaxBodies = 4096;
		static constexpr uint32_t kMaxBodyPairs = 8192;
		static constexpr uint32_t kMaxContactConstraints = 8192;
		static constexpr uint32_t kTempAllocatorBytes = 10 * 1024 * 1024;
	}

	class PhysicsWorldImpl
	{
	public:
		PhysicsWorldImpl()
			: mTempAllocator(kTempAllocatorBytes),
			mJobSystem(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::max(1u, std::thread::hardware_concurrency() - 1))
		{
			mPhysicsSystem.Init(
				kMaxBodies,
				0,
				kMaxBodyPairs,
				kMaxContactConstraints,
				mBroadPhaseLayers,
				mObjectVsBroadPhaseLayerFilter,
				mObjectLayerPairFilter);
		}

		void ApplyScene(const std::vector<BodyConfig>& bodies, bool simulationEnabled)
		{
			BodyInterface& bodyInterface = mPhysicsSystem.GetBodyInterface();

			std::unordered_map<uint32_t, BodyConfig> incoming;
			incoming.reserve(bodies.size());
			for (const BodyConfig& body : bodies)
			{
				if (body.objectId == 0)
					continue;
				incoming[body.objectId] = body;
			}

			for (auto it = mBodiesByObjectId.begin(); it != mBodiesByObjectId.end(); )
			{
				if (incoming.find(it->first) == incoming.end())
				{
					if (bodyInterface.IsAdded(it->second))
						bodyInterface.RemoveBody(it->second);
					bodyInterface.DestroyBody(it->second);
					mBodyConfigByObjectId.erase(it->first);
					it = mBodiesByObjectId.erase(it);
				}
				else
				{
					++it;
				}
			}

			for (const auto& [objectId, cfg] : incoming)
			{
				const auto existing = mBodiesByObjectId.find(objectId);
				if (existing == mBodiesByObjectId.end())
				{
					CreateBody(cfg);
				}
				else
				{
					const auto prevCfgIt = mBodyConfigByObjectId.find(objectId);
					if (prevCfgIt != mBodyConfigByObjectId.end() && ShouldRecreateBody(prevCfgIt->second, cfg))
					{
						if (bodyInterface.IsAdded(existing->second))
							bodyInterface.RemoveBody(existing->second);
						bodyInterface.DestroyBody(existing->second);
						CreateBody(cfg);
					}
					else
					{
						UpdateBody(existing->second, cfg, simulationEnabled);
					}
				}

				mBodyConfigByObjectId[objectId] = cfg;
			}
		}

		void Step(float dt)
		{
			mPhysicsSystem.Update(dt, kCollisionSteps, &mTempAllocator, &mJobSystem);
		}

		std::vector<BodyTransform> ReadTransforms() const
		{
			std::vector<BodyTransform> out;
			out.reserve(mBodiesByObjectId.size());

			const BodyInterface& bodyInterface = mPhysicsSystem.GetBodyInterface();
			for (const auto& [objectId, bodyId] : mBodiesByObjectId)
			{
				RVec3 pos;
				Quat rot;
				bodyInterface.GetPositionAndRotation(bodyId, pos, rot);

				const Vec3 eulerRad = rot.GetEulerAngles();

				BodyTransform t{};
				t.objectId = objectId;
				t.position = glm::vec3(
					static_cast<float>(pos.GetX()),
					static_cast<float>(pos.GetY()),
					static_cast<float>(pos.GetZ()));
				t.rotationDeg = glm::degrees(glm::vec3(eulerRad.GetX(), eulerRad.GetY(), eulerRad.GetZ()));
				out.push_back(t);
			}

			return out;
		}

	private:
		static EMotionType ToJoltMotionType(MotionType type)
		{
			switch (type)
			{
			case MotionType::Static: return EMotionType::Static;
			case MotionType::Kinematic: return EMotionType::Kinematic;
			case MotionType::Dynamic:
			default: return EMotionType::Dynamic;
			}
		}

		static ObjectLayer ToObjectLayer(MotionType type)
		{
			return type == MotionType::Static ? Layers::NonMoving : Layers::Moving;
		}

		static bool ShouldRecreateBody(const BodyConfig& prev, const BodyConfig& next)
		{
			if (prev.motionType != next.motionType)
				return true;

			const glm::vec3 diff = glm::abs(prev.halfExtents - next.halfExtents);
			return diff.x > 0.001f || diff.y > 0.001f || diff.z > 0.001f;
		}

		void CreateBody(const BodyConfig& cfg)
		{
			BodyInterface& bodyInterface = mPhysicsSystem.GetBodyInterface();

			const Vec3 halfExtents(
				std::max(0.01f, cfg.halfExtents.x),
				std::max(0.01f, cfg.halfExtents.y),
				std::max(0.01f, cfg.halfExtents.z));
			RefConst<Shape> shape = new BoxShape(halfExtents);

			const glm::vec3 rotRad = glm::radians(cfg.rotationDeg);
			const Quat rot = Quat::sEulerAngles(Vec3(rotRad.x, rotRad.y, rotRad.z));

			BodyCreationSettings settings(
				shape,
				RVec3(cfg.position.x, cfg.position.y, cfg.position.z),
				rot,
				ToJoltMotionType(cfg.motionType),
				ToObjectLayer(cfg.motionType));

			settings.mAllowDynamicOrKinematic = true;

			BodyID bodyId = bodyInterface.CreateAndAddBody(settings, EActivation::Activate);
			if (bodyId.IsInvalid())
				return;

			bodyInterface.SetGravityFactor(bodyId, cfg.useGravity ? 1.0f : 0.0f);
			mBodiesByObjectId[cfg.objectId] = bodyId;
		}

		void UpdateBody(BodyID bodyId, const BodyConfig& cfg, bool simulationEnabled)
		{
			BodyInterface& bodyInterface = mPhysicsSystem.GetBodyInterface();

			const bool canTeleportBody =
				cfg.motionType == MotionType::Static ||
				cfg.motionType == MotionType::Kinematic ||
				!simulationEnabled;

			if (canTeleportBody)
			{
				const glm::vec3 rotRad = glm::radians(cfg.rotationDeg);
				const Quat rot = Quat::sEulerAngles(Vec3(rotRad.x, rotRad.y, rotRad.z));
				bodyInterface.SetPositionAndRotation(
					bodyId,
					RVec3(cfg.position.x, cfg.position.y, cfg.position.z),
					rot,
					EActivation::Activate);
			}

			if (cfg.motionType != MotionType::Dynamic || !simulationEnabled)
				bodyInterface.SetLinearVelocity(bodyId, Vec3::sZero());

			bodyInterface.SetGravityFactor(bodyId, cfg.useGravity ? 1.0f : 0.0f);
		}

	private:
		BPLayerInterfaceImpl mBroadPhaseLayers;
		ObjectVsBroadPhaseLayerFilterImpl mObjectVsBroadPhaseLayerFilter;
		ObjectLayerPairFilterImpl mObjectLayerPairFilter;

		PhysicsSystem mPhysicsSystem;
		TempAllocatorImpl mTempAllocator;
		JobSystemThreadPool mJobSystem;

		std::unordered_map<uint32_t, BodyID> mBodiesByObjectId;
		std::unordered_map<uint32_t, BodyConfig> mBodyConfigByObjectId;
	};

	PhysicsThreadedWorld::PhysicsThreadedWorld()
	{
		m_thread = std::thread([this]() { ThreadMain(); });
	}

	PhysicsThreadedWorld::~PhysicsThreadedWorld()
	{
		RequestStop();
		if (m_thread.joinable())
			m_thread.join();
	}

	void PhysicsThreadedWorld::RequestStop()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_stop = true;
			m_hasScene = true;
		}
		m_cv.notify_one();
	}

	void PhysicsThreadedWorld::SubmitScene(const std::vector<BodyConfig>& bodies)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_pendingScene = bodies;
			m_hasScene = true;
		}
		m_cv.notify_one();
	}

	void PhysicsThreadedWorld::SetSimulationEnabled(bool enabled)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_simulationEnabled = enabled;
		}
		m_cv.notify_one();
	}

	std::vector<BodyTransform> PhysicsThreadedWorld::ConsumeLatestTransforms()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_latestTransforms;
	}

	void PhysicsThreadedWorld::ThreadMain()
	{
		PhysicsWorldImpl world;
		m_initialized.store(true, std::memory_order_relaxed);

		auto previousTick = std::chrono::steady_clock::now();
		std::vector<BodyConfig> sceneSnapshot;

		while (true)
		{
			bool simEnabled = false;
			bool sceneUpdated = false;
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				if (!m_hasScene && !m_stop)
				{
					m_cv.wait_for(lock, std::chrono::milliseconds(4));
				}

				if (m_stop)
					break;

				if (m_hasScene)
				{
					sceneSnapshot = m_pendingScene;
					m_hasScene = false;
					sceneUpdated = true;
				}
				simEnabled = m_simulationEnabled;
			}

			if (sceneUpdated)
				world.ApplyScene(sceneSnapshot, simEnabled);

			const auto now = std::chrono::steady_clock::now();
			const std::chrono::duration<float> elapsed = now - previousTick;
			previousTick = now;

			if (simEnabled)
			{
				float dt = elapsed.count();
				if (dt < 0.0001f)
					dt = kFixedDt;
				if (dt > 0.05f)
					dt = 0.05f;

				world.Step(dt);
			}

			std::vector<BodyTransform> latest = world.ReadTransforms();
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_latestTransforms = std::move(latest);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
} // namespace Ark::Physics