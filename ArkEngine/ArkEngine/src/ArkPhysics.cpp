#include "ArkPhysics.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Ark::Physics
{
    // -------------------------------------------------------------------------
    // Body management
    // -------------------------------------------------------------------------

    void PhysicsWorld::AddOrUpdateBody(const PhysicsBody& body)
    {
        for (auto& b : m_bodies)
        {
            if (b.objectId == body.objectId)
            {
                // Preserve runtime velocity / onGround when updating shape/type.
                const glm::vec3 vel = b.velocity;
                const bool      grnd = b.onGround;
                b = body;
                b.velocity = vel;
                b.onGround = grnd;
                return;
            }
        }
        m_bodies.push_back(body);
    }

    void PhysicsWorld::RemoveBody(uint32_t objectId)
    {
        m_bodies.erase(
            std::remove_if(m_bodies.begin(), m_bodies.end(),
                [objectId](const PhysicsBody& b) { return b.objectId == objectId; }),
            m_bodies.end());
    }

    PhysicsBody* PhysicsWorld::GetBody(uint32_t objectId)
    {
        for (auto& b : m_bodies)
            if (b.objectId == objectId) return &b;
        return nullptr;
    }

    const PhysicsBody* PhysicsWorld::GetBody(uint32_t objectId) const
    {
        for (const auto& b : m_bodies)
            if (b.objectId == objectId) return &b;
        return nullptr;
    }

    void PhysicsWorld::ApplyImpulse(uint32_t objectId, const glm::vec3& impulse)
    {
        if (PhysicsBody* b = GetBody(objectId))
            if (b->type == BodyType::Dynamic || b->type == BodyType::Kinematic)
                b->velocity += impulse;
    }

    // -------------------------------------------------------------------------
    // Broadphase  (brute-force O(n^2) — fine for < ~200 objects)
    // -------------------------------------------------------------------------

    void PhysicsWorld::BroadPhase(std::vector<std::pair<size_t, size_t>>& outPairs) const
    {
        for (size_t i = 0; i < m_bodies.size(); ++i)
        {
            for (size_t j = i + 1; j < m_bodies.size(); ++j)
            {
                // Static vs Static never needs collision response.
                if (m_bodies[i].type == BodyType::Static &&
                    m_bodies[j].type == BodyType::Static)
                    continue;

                outPairs.emplace_back(i, j);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Step
    // -------------------------------------------------------------------------

    void PhysicsWorld::Step(
        float                                                dt,
        std::vector<std::pair<uint32_t, glm::vec3>>& outPositions,
        std::vector<CollisionEvent>& outEvents)
    {
        outEvents.clear();

        // --- 1. Integrate velocity for dynamic bodies ---
        for (auto& body : m_bodies)
        {
            if (body.type != BodyType::Dynamic)
                continue;

            // Gravity
            body.velocity.y += kGravity * dt;

            // Find current world position from outPositions map
            // (caller maintains authoritative positions; physics applies deltas).
            for (auto& [id, pos] : outPositions)
            {
                if (id != body.objectId) continue;
                pos += body.velocity * dt;
                break;
            }

            body.onGround = false;
        }

        // --- 2. Broadphase ---
        std::vector<std::pair<size_t, size_t>> pairs;
        BroadPhase(pairs);

        // --- 3. Narrowphase AABB vs AABB + response ---
        for (const auto& [iA, iB] : pairs)
        {
            PhysicsBody& bodyA = m_bodies[iA];
            PhysicsBody& bodyB = m_bodies[iB];

            // Resolve world-space positions for each body.
            glm::vec3 posA{ 0.0f }, posB{ 0.0f };
            for (const auto& [id, pos] : outPositions)
            {
                if (id == bodyA.objectId) posA = pos;
                if (id == bodyB.objectId) posB = pos;
            }

            const AABB worldA = bodyA.localAABB.Translated(posA);
            const AABB worldB = bodyB.localAABB.Translated(posB);

            glm::vec3 depth{ 0.0f };
            if (!worldA.Penetration(worldB, depth))
                continue;

            // Emit event regardless of trigger state.
            CollisionEvent ev{};
            ev.bodyAId = bodyA.objectId;
            ev.bodyBId = bodyB.objectId;
            ev.depth = depth;
            outEvents.push_back(ev);

            // No physical response for triggers.
            if (bodyA.isTrigger || bodyB.isTrigger)
                continue;

            // --- MTV resolution ---
            // Push dynamic body out; static bodies don't move.
            const bool aIsDynamic = bodyA.type == BodyType::Dynamic || bodyA.type == BodyType::Kinematic;
            const bool bIsDynamic = bodyB.type == BodyType::Dynamic || bodyB.type == BodyType::Kinematic;

            auto applyResolution = [&](PhysicsBody& dynBody, const glm::vec3& pushVec)
                {
                    for (auto& [id, pos] : outPositions)
                    {
                        if (id != dynBody.objectId) continue;
                        pos += pushVec;

                        // Cancel velocity component along the push axis.
                        if (pushVec.x != 0.0f) dynBody.velocity.x = 0.0f;
                        if (pushVec.z != 0.0f) dynBody.velocity.z = 0.0f;
                        if (pushVec.y > 0.0f)
                        {
                            // Landed on something.
                            dynBody.velocity.y = 0.0f;
                            dynBody.onGround = true;
                        }
                        else if (pushVec.y < 0.0f)
                        {
                            // Hit a ceiling.
                            dynBody.velocity.y = 0.0f;
                        }
                        break;
                    }
                };

            if (aIsDynamic && !bIsDynamic)
                applyResolution(bodyA, depth);
            else if (!aIsDynamic && bIsDynamic)
                applyResolution(bodyB, -depth);
            else if (aIsDynamic && bIsDynamic)
            {
                // Both dynamic: split equally.
                applyResolution(bodyA, depth * 0.5f);
                applyResolution(bodyB, -depth * 0.5f);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Line trace
    // -------------------------------------------------------------------------

    LineTraceHit PhysicsWorld::LineTrace(
        const glm::vec3& origin,
        const glm::vec3& directionNorm,
        float            maxDistance,
        const std::vector<std::pair<uint32_t, glm::vec3>>& bodyPositions,
        uint32_t         ignoreObjectId) const
    {
        LineTraceHit best{};
        best.distance = std::numeric_limits<float>::max();

        for (const auto& body : m_bodies)
        {
            if (body.objectId == ignoreObjectId)
                continue;

            // Find world position for this body.
            glm::vec3 pos{ 0.0f };
            bool found = false;
            for (const auto& [id, p] : bodyPositions)
            {
                if (id == body.objectId) { pos = p; found = true; break; }
            }
            if (!found) continue;

            const AABB worldBox = body.localAABB.Translated(pos);

            float t = 0.0f;
            if (!RayVsAABB(origin, directionNorm, worldBox, maxDistance, t))
                continue;

            if (t >= best.distance)
                continue;

            // Compute face normal from which face the ray entered.
            const glm::vec3 hitPt = origin + directionNorm * t;
            const glm::vec3 localPt = hitPt - worldBox.Centre();
            const glm::vec3 he = worldBox.HalfExtents();

            // The dominant axis of localPt / halfExtents is the hit face.
            const glm::vec3 ratio = glm::abs(localPt) / he;
            glm::vec3 normal{ 0.0f };

            if (ratio.x >= ratio.y && ratio.x >= ratio.z)
                normal = glm::vec3(glm::sign(localPt.x), 0.0f, 0.0f);
            else if (ratio.y >= ratio.z)
                normal = glm::vec3(0.0f, glm::sign(localPt.y), 0.0f);
            else
                normal = glm::vec3(0.0f, 0.0f, glm::sign(localPt.z));

            best.hit = true;
            best.distance = t;
            best.point = hitPt;
            best.normal = normal;
            best.objectId = body.objectId;
        }

        return best;
    }

} // namespace Ark::Physics