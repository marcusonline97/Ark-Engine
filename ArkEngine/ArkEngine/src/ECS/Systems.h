#pragma once
#include "../Common.h"
#include "ECS.h"
#include "Components.h"
#include <entt/entt.hpp>

// ECS Systems for Phase 4
// Systems update entities based on their components

namespace ECSSystems {
    
    // Transform System - Updates world matrices based on parent/child hierarchy
    // Must be called every frame before rendering
    void UpdateTransformHierarchy();
    
    // Helper: Recursively update transform for entity and all its children
    void UpdateTransformRecursive(Entity entity, const glm::mat4& parentWorldMatrix = glm::mat4(1.0f));
    
    // Get world matrix for an entity (computes on-the-fly if dirty)
    glm::mat4 GetWorldTransformMatrix(Entity entity);
}
