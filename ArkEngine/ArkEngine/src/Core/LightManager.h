#pragma once
#include "Light.h"
#include "../Common.h"
#include <vector>
#include <memory>
#include <string>

// LightManager for Phase 4 - manages multiple lights in the scene
namespace LightManager {
    
    // Maximum number of lights supported (can be increased later with UBOs)
    constexpr int MAX_LIGHTS = 32;
    
    // Initialize light manager
    void Initialize();
    
    // Cleanup
    void Cleanup();
    
    // Add a light to the scene
    // Returns the index of the added light, or -1 if max lights reached
    int AddLight(const Light& light);
    
    // Remove a light by index
    void RemoveLight(int index);
    
    // Remove a light by name
    void RemoveLight(const std::string& name);
    
    // Get light by index
    Light* GetLight(int index);
    
    // Get light by name
    Light* GetLight(const std::string& name);
    
    // Get all lights
    std::vector<Light>& GetLights();
    
    // Get number of active (enabled) lights
    int GetActiveLightCount();
    
    // Get enabled lights only
    std::vector<Light*> GetEnabledLights();
    
    // Convert lights to GPU-friendly format for shader uniforms
    // Returns vector of GPULightData for uniform buffer upload
    std::vector<GPULightData> GetGPULightData();
    
    // Create default test lights (for Phase 4 demo)
    void CreateDefaultLights();
    
    // Clear all lights
    void ClearAllLights();
}
