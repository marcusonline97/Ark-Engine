#pragma once
#include "../Common.h"
#include <entt/entt.hpp>
#include "Components.h"
#include <string>
#include <vector>
#include <memory>

// ECS wrapper around ENTT registry
// Provides entity management, component storage, and parent/child hierarchy support
namespace ECS {
    
    // Get the global registry
    entt::registry& GetRegistry();
    
    // Entity management
    Entity CreateEntity();
    Entity CreateEntity(const std::string& name);
    void DestroyEntity(Entity entity);
    bool IsValid(Entity entity);
    
    // Component management
    template<typename Component>
    Component& AddComponent(Entity entity, const Component& component);
    
    template<typename Component>
    Component& AddComponent(Entity entity);
    
    template<typename Component>
    void RemoveComponent(Entity entity);
    
    template<typename Component>
    Component* GetComponent(Entity entity);
    
    template<typename Component>
    bool HasComponent(Entity entity);
    
    // Parent/Child relationship management
    void SetParent(Entity child, Entity parent);
    void RemoveParent(Entity child);
    Entity GetParent(Entity entity);
    std::vector<Entity> GetChildren(Entity entity);
    bool HasParent(Entity entity);
    bool HasChildren(Entity entity);
    
    // Name/Tag management
    void SetName(Entity entity, const std::string& name);
    std::string GetName(Entity entity);
    void SetTag(Entity entity, const std::string& tag);
    std::string GetTag(Entity entity);
    Entity FindEntityByName(const std::string& name);
    std::vector<Entity> FindEntitiesByTag(const std::string& tag);
    
    // Transform helpers
    void SetTransform(Entity entity, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
    TransformComponent* GetTransform(Entity entity);
    glm::mat4 GetWorldMatrix(Entity entity);
    glm::vec3 GetWorldPosition(Entity entity);
    
    // Query helpers
    template<typename... Components>
    auto View();
    
    template<typename... Components>
    auto Group();
    
    // Cleanup
    void Clear();
    size_t GetEntityCount();
}

// Template implementations
namespace ECS {
    
    template<typename Component>
    Component& AddComponent(Entity entity, const Component& component) {
        auto& registry = GetRegistry();
        return registry.emplace_or_replace<Component>(entity, component);
    }
    
    template<typename Component>
    Component& AddComponent(Entity entity) {
        auto& registry = GetRegistry();
        return registry.emplace<Component>(entity);
    }
    
    template<typename Component>
    void RemoveComponent(Entity entity) {
        auto& registry = GetRegistry();
        registry.remove<Component>(entity);
    }
    
    template<typename Component>
    Component* GetComponent(Entity entity) {
        auto& registry = GetRegistry();
        return registry.try_get<Component>(entity);
    }
    
    template<typename Component>
    bool HasComponent(Entity entity) {
        auto& registry = GetRegistry();
        return registry.all_of<Component>(entity);
    }
    
    template<typename... Components>
    auto View() {
        auto& registry = GetRegistry();
        return registry.view<Components...>();
    }
    
    template<typename... Components>
    auto Group() {
        auto& registry = GetRegistry();
        return registry.group<Components...>();
    }
}
