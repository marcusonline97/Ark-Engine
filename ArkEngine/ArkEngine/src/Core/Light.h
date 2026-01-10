#pragma once
#include "../Common.h"
#include <glm/glm.hpp>
#include <string>

// Light types for Phase 4
enum class LightType {
    POINT,       // Omnidirectional light from a point
    DIRECTIONAL, // Light from a direction (like sun)
    SPOT         // Conical light from a point with direction
};

// Enhanced Light structure for Phong lighting
struct Light {
    LightType type = LightType::POINT;
    
    // Position (for Point and Spot lights)
    glm::vec3 position = glm::vec3(0.0f);
    
    // Direction (for Directional and Spot lights)
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    
    // Color and intensity
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    
    // Attenuation (for Point and Spot lights)
    // Attenuation formula: 1.0 / (constant + linear * distance + quadratic * distance^2)
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    
    // Spot light specific
    float cutOff = glm::cos(glm::radians(12.5f));      // Inner cone
    float outerCutOff = glm::cos(glm::radians(15.0f)); // Outer cone
    
    // Name for identification
    std::string name = "UnnamedLight";
    
    // Enabled flag
    bool enabled = true;
    
    // Constructors
    Light() = default;
    
    // Point light constructor
    Light(const glm::vec3& pos, const glm::vec3& col, float intens = 1.0f)
        : type(LightType::POINT), position(pos), color(col), intensity(intens) {}
    
    // Directional light constructor
    Light(const glm::vec3& dir, const glm::vec3& col, float intens = 1.0f, LightType t = LightType::DIRECTIONAL)
        : type(t), direction(glm::normalize(dir)), color(col), intensity(intens) {}
    
    // Spot light constructor
    Light(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& col, 
          float innerAngle = 12.5f, float outerAngle = 15.0f, float intens = 1.0f)
        : type(LightType::SPOT), position(pos), direction(glm::normalize(dir)), 
          color(col), intensity(intens), cutOff(glm::cos(glm::radians(innerAngle))),
          outerCutOff(glm::cos(glm::radians(outerAngle))) {}
    
    // Helper methods
    void SetAttenuation(float constant, float linear, float quadratic) {
        this->constant = constant;
        this->linear = linear;
        this->quadratic = quadratic;
    }
    
    void SetSpotAngles(float innerDegrees, float outerDegrees) {
        cutOff = glm::cos(glm::radians(innerDegrees));
        outerCutOff = glm::cos(glm::radians(outerDegrees));
    }
};

// GPU-friendly light structure for shader uniform buffers
// Packed for efficient GPU upload (std140 alignment)
struct GPULightData {
    glm::vec4 position;      // xyz = position, w = light type (0=Point, 1=Directional, 2=Spot)
    glm::vec4 direction;     // xyz = direction, w = intensity
    glm::vec4 color;         // rgb = color, w = constant attenuation
    glm::vec4 attenuation;   // x = linear, y = quadratic, z = cutOff, w = outerCutOff
};
