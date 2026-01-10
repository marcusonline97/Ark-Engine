#pragma once
#include "../Common.h"
#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

// Forward declare entity type
using Entity = entt::entity;

// Transform Component - wraps existing Transform struct
struct TransformComponent {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);  // Euler angles in degrees
    glm::vec3 scale = glm::vec3(1.0f);
    
    // Cached transform matrix (updated when needed)
    mutable glm::mat4 worldMatrix = glm::mat4(1.0f);
    mutable bool isDirty = true;
    
    // Local transform matrix (relative to parent)
    glm::mat4 GetLocalMatrix() const {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0));
        glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1, 0));
        glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1));
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
        return translation * rotationZ * rotationY * rotationX * scaleMat;
    }
    
    // Convert to mat4 (for compatibility with existing Transform struct)
    glm::mat4 to_mat4() const {
        return GetLocalMatrix();
    }
    
    // Mark transform as dirty (needs recalculation)
    void MarkDirty() {
        isDirty = true;
    }
    
    // Get world matrix (will be updated by TransformSystem)
    const glm::mat4& GetWorldMatrix() const {
        return worldMatrix;
    }
};

// Parent Component - stores reference to parent entity
struct ParentComponent {
    Entity parent = entt::null;
    
    bool HasParent() const {
        return parent != entt::null;
    }
};

// Child Component - stores list of child entities
struct ChildComponent {
    std::vector<Entity> children;
    
    void AddChild(Entity child) {
        // Check if child already exists
        if (std::find(children.begin(), children.end(), child) == children.end()) {
            children.push_back(child);
        }
    }
    
    void RemoveChild(Entity child) {
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
    }
    
    bool HasChild(Entity child) const {
        return std::find(children.begin(), children.end(), child) != children.end();
    }
    
    size_t GetChildCount() const {
        return children.size();
    }
};

// Name Component - for entity identification
struct NameComponent {
    std::string name = "UnnamedEntity";
    
    NameComponent() = default;
    NameComponent(const std::string& n) : name(n) {}
};

// Tag Component - for grouping/querying entities
struct TagComponent {
    std::string tag = "";
    
    TagComponent() = default;
    TagComponent(const std::string& t) : tag(t) {}
};

// Mesh Component - references loaded mesh buffers
struct MeshComponent {
    // Index into mesh buffers vector (from OBJLoader)
    size_t meshBufferIndex = 0;
    
    // Material pointer (void* to avoid circular dependency, cast to Material* when needed)
    void* materialPtr = nullptr;
    
    bool visible = true;
};

// Render Component - for rendering entities
struct RenderComponent {
    bool castShadows = true;
    bool receiveShadows = true;
    int layer = 0;  // Render layer (for sorting)
};
