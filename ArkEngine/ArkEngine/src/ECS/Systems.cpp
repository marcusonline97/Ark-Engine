#include "Systems.h"
#include "../Common.h"
#include <iostream>

namespace ECSSystems {
    
    void UpdateTransformRecursive(Entity entity, const glm::mat4& parentWorldMatrix) {
        if (!ECS::IsValid(entity)) {
            return;
        }
        
        auto* transform = ECS::GetComponent<TransformComponent>(entity);
        if (!transform) {
            return;
        }
        
        // Update world matrix
        if (transform->isDirty || parentWorldMatrix != glm::mat4(1.0f)) {
            glm::mat4 localMatrix = transform->GetLocalMatrix();
            transform->worldMatrix = parentWorldMatrix * localMatrix;
            transform->isDirty = false;
        }
        
        // Update children recursively
        auto children = ECS::GetChildren(entity);
        glm::mat4 currentWorldMatrix = transform->worldMatrix;
        for (Entity child : children) {
            UpdateTransformRecursive(child, currentWorldMatrix);
        }
    }
    
    void UpdateTransformHierarchy() {
        auto& registry = ECS::GetRegistry();
        std::vector<Entity> rootEntities;
        
        // Get all entities with transforms
        auto allTransforms = registry.view<TransformComponent>();
        for (auto entity : allTransforms) {
            // If no parent component, it's a root
            if (!ECS::HasComponent<ParentComponent>(entity)) {
                rootEntities.push_back(entity);
            }
            else {
                // If parent is invalid or null, it's also a root
                Entity parent = ECS::GetParent(entity);
                if (!ECS::IsValid(parent)) {
                    // Clean up invalid parent reference
                    ECS::RemoveParent(entity);
                    rootEntities.push_back(entity);
                }
            }
        }
        
        // Update all root entities and their hierarchies
        for (Entity root : rootEntities) {
            UpdateTransformRecursive(root, glm::mat4(1.0f));
        }
        
        // Handle entities that might have been orphaned (have parent but parent doesn't exist)
        auto orphanedView = registry.view<ParentComponent>();
        for (auto entity : orphanedView) {
            Entity parent = ECS::GetParent(entity);
            if (!ECS::IsValid(parent)) {
                // Orphaned entity - remove parent reference and update as root
                ECS::RemoveParent(entity);
                if (auto* transform = ECS::GetComponent<TransformComponent>(entity)) {
                    UpdateTransformRecursive(entity, glm::mat4(1.0f));
                }
            }
        }
    }
    
    glm::mat4 GetWorldTransformMatrix(Entity entity) {
        if (!ECS::IsValid(entity)) {
            return glm::mat4(1.0f);
        }
        
        auto* transform = ECS::GetComponent<TransformComponent>(entity);
        if (!transform) {
            return glm::mat4(1.0f);
        }
        
        // If transform is dirty, update it
        if (transform->isDirty) {
            // Get parent world matrix
            glm::mat4 parentWorldMatrix = glm::mat4(1.0f);
            if (ECS::HasParent(entity)) {
                Entity parent = ECS::GetParent(entity);
                if (ECS::IsValid(parent)) {
                    parentWorldMatrix = GetWorldTransformMatrix(parent);
                }
            }
            
            // Update this transform
            glm::mat4 localMatrix = transform->GetLocalMatrix();
            transform->worldMatrix = parentWorldMatrix * localMatrix;
            transform->isDirty = false;
        }
        
        return transform->worldMatrix;
    }
}
