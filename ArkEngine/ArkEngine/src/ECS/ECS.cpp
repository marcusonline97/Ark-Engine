#include "ECS.h"
#include "../Common.h"
#include <algorithm>
#include <iostream>
#include <memory>

namespace ECS {
    
    // Global registry instance
    static entt::registry s_registry;
    static size_t s_entityCount = 0;  // Track entity count manually
    
    entt::registry& GetRegistry() {
        return s_registry;
    }
    
    Entity CreateEntity() {
        Entity entity = s_registry.create();
        s_entityCount++;
        return entity;
    }
    
    Entity CreateEntity(const std::string& name) {
        Entity entity = s_registry.create();
        s_entityCount++;
        auto& nameComp = s_registry.emplace<NameComponent>(entity, name);
        return entity;
    }
    
    void DestroyEntity(Entity entity) {
        if (!IsValid(entity)) {
            return;
        }
        
        // Remove from parent's children list if has parent
        if (HasParent(entity)) {
            Entity parent = GetParent(entity);
            if (IsValid(parent)) {
                if (auto* childComp = GetComponent<ChildComponent>(parent)) {
                    childComp->RemoveChild(entity);
                }
            }
        }
        
        // Remove all children's parent references
        if (HasChildren(entity)) {
            std::vector<Entity> children = GetChildren(entity);
            for (Entity child : children) {
                if (auto* parentComp = GetComponent<ParentComponent>(child)) {
                    parentComp->parent = entt::null;
                }
                // Mark child's transform as dirty
                if (auto* transform = GetComponent<TransformComponent>(child)) {
                    transform->MarkDirty();
                }
            }
        }
        
        // Remove parent and child components
        RemoveComponent<ParentComponent>(entity);
        RemoveComponent<ChildComponent>(entity);
        
        // Destroy entity (this will remove all components)
        s_registry.destroy(entity);
        if (s_entityCount > 0) {
            s_entityCount--;
        }
    }
    
    bool IsValid(Entity entity) {
        return s_registry.valid(entity);
    }
    
    void SetParent(Entity child, Entity parent) {
        if (!IsValid(child) || !IsValid(parent) || child == parent) {
            return;
        }
        
        // Remove from old parent's children list
        if (HasParent(child)) {
            Entity oldParent = GetParent(child);
            if (auto* oldParentChildComp = GetComponent<ChildComponent>(oldParent)) {
                oldParentChildComp->RemoveChild(child);
            }
        }
        
        // Set new parent
        auto& parentComp = AddComponent<ParentComponent>(child);
        parentComp.parent = parent;
        
        // Add to new parent's children list
        if (!HasComponent<ChildComponent>(parent)) {
            AddComponent<ChildComponent>(parent);
        }
        auto* childComp = GetComponent<ChildComponent>(parent);
        if (childComp) {
            childComp->AddChild(child);
        }
        
        // Mark transform as dirty
        if (auto* transform = GetComponent<TransformComponent>(child)) {
            transform->MarkDirty();
        }
    }
    
    void RemoveParent(Entity child) {
        if (!HasParent(child)) {
            return;
        }
        
        Entity parent = GetParent(child);
        if (IsValid(parent)) {
            if (auto* childComp = GetComponent<ChildComponent>(parent)) {
                childComp->RemoveChild(child);
            }
        }
        
        RemoveComponent<ParentComponent>(child);
        
        // Mark transform as dirty
        if (auto* transform = GetComponent<TransformComponent>(child)) {
            transform->MarkDirty();
        }
    }
    
    Entity GetParent(Entity entity) {
        if (auto* parentComp = GetComponent<ParentComponent>(entity)) {
            return parentComp->parent;
        }
        return entt::null;
    }
    
    std::vector<Entity> GetChildren(Entity entity) {
        if (auto* childComp = GetComponent<ChildComponent>(entity)) {
            return childComp->children;
        }
        return std::vector<Entity>();
    }
    
    bool HasParent(Entity entity) {
        if (auto* parentComp = GetComponent<ParentComponent>(entity)) {
            return parentComp->HasParent();
        }
        return false;
    }
    
    bool HasChildren(Entity entity) {
        if (auto* childComp = GetComponent<ChildComponent>(entity)) {
            return childComp->GetChildCount() > 0;
        }
        return false;
    }
    
    void SetName(Entity entity, const std::string& name) {
        if (!IsValid(entity)) {
            return;
        }
        auto& nameComp = AddComponent<NameComponent>(entity);
        nameComp.name = name;
    }
    
    std::string GetName(Entity entity) {
        if (auto* nameComp = GetComponent<NameComponent>(entity)) {
            return nameComp->name;
        }
        return "UnnamedEntity";
    }
    
    void SetTag(Entity entity, const std::string& tag) {
        if (!IsValid(entity)) {
            return;
        }
        auto& tagComp = AddComponent<TagComponent>(entity);
        tagComp.tag = tag;
    }
    
    std::string GetTag(Entity entity) {
        if (auto* tagComp = GetComponent<TagComponent>(entity)) {
            return tagComp->tag;
        }
        return "";
    }
    
    Entity FindEntityByName(const std::string& name) {
        auto view = View<NameComponent>();
        for (auto entity : view) {
            auto& nameComp = view.get<NameComponent>(entity);
            if (nameComp.name == name) {
                return entity;
            }
        }
        return entt::null;
    }
    
    std::vector<Entity> FindEntitiesByTag(const std::string& tag) {
        std::vector<Entity> entities;
        auto view = View<TagComponent>();
        for (auto entity : view) {
            auto& tagComp = view.get<TagComponent>(entity);
            if (tagComp.tag == tag) {
                entities.push_back(entity);
            }
        }
        return entities;
    }
    
    void SetTransform(Entity entity, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
        if (!IsValid(entity)) {
            return;
        }
        auto& transform = AddComponent<TransformComponent>(entity);
        transform.position = position;
        transform.rotation = rotation;
        transform.scale = scale;
        transform.MarkDirty();
    }
    
    TransformComponent* GetTransform(Entity entity) {
        return GetComponent<TransformComponent>(entity);
    }
    
    glm::mat4 GetWorldMatrix(Entity entity) {
        if (auto* transform = GetComponent<TransformComponent>(entity)) {
            return transform->GetWorldMatrix();
        }
        return glm::mat4(1.0f);
    }
    
    glm::vec3 GetWorldPosition(Entity entity) {
        glm::mat4 worldMatrix = GetWorldMatrix(entity);
        return glm::vec3(worldMatrix[3]); // Extract position from matrix
    }
    
    void Clear() {
        s_registry.clear();
        s_entityCount = 0;
    }
    
    size_t GetEntityCount() {
        return s_entityCount;
    }
}
