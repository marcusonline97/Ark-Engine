#include "LightManager.h"
#include <algorithm>

namespace LightManager {
    
    static std::vector<Light> s_lights;
    static bool s_initialized = false;
    
    void Initialize() {
        if (s_initialized) {
            return;
        }
        
        s_lights.clear();
        s_lights.reserve(MAX_LIGHTS);
        s_initialized = true;
    }
    
    void Cleanup() {
        s_lights.clear();
        s_initialized = false;
    }
    
    int AddLight(const Light& light) {
        if (s_lights.size() >= static_cast<size_t>(MAX_LIGHTS)) {
            std::cout << "WARNING: Maximum number of lights (" << MAX_LIGHTS << ") reached. Cannot add more lights.\n";
            return -1;
        }
        
        int index = static_cast<int>(s_lights.size());
        s_lights.push_back(light);
        
        // If light has no name, assign a default one
        if (s_lights[index].name == "UnnamedLight" || s_lights[index].name.empty()) {
            s_lights[index].name = "Light_" + std::to_string(index);
        }
        
        return index;
    }
    
    void RemoveLight(int index) {
        if (index >= 0 && index < static_cast<int>(s_lights.size())) {
            s_lights.erase(s_lights.begin() + index);
        }
    }
    
    void RemoveLight(const std::string& name) {
        auto it = std::find_if(s_lights.begin(), s_lights.end(),
            [&name](const Light& light) { return light.name == name; });
        
        if (it != s_lights.end()) {
            s_lights.erase(it);
        }
    }
    
    Light* GetLight(int index) {
        if (index >= 0 && index < static_cast<int>(s_lights.size())) {
            return &s_lights[index];
        }
        return nullptr;
    }
    
    Light* GetLight(const std::string& name) {
        auto it = std::find_if(s_lights.begin(), s_lights.end(),
            [&name](const Light& light) { return light.name == name; });
        
        if (it != s_lights.end()) {
            return &(*it);
        }
        return nullptr;
    }
    
    std::vector<Light>& GetLights() {
        return s_lights;
    }
    
    int GetActiveLightCount() {
        int count = 0;
        for (const auto& light : s_lights) {
            if (light.enabled) {
                count++;
            }
        }
        return count;
    }
    
    std::vector<Light*> GetEnabledLights() {
        std::vector<Light*> enabledLights;
        for (auto& light : s_lights) {
            if (light.enabled) {
                enabledLights.push_back(&light);
            }
        }
        return enabledLights;
    }
    
    std::vector<GPULightData> GetGPULightData() {
        std::vector<GPULightData> gpuData;
        gpuData.reserve(s_lights.size());
        
        for (const auto& light : s_lights) {
            if (!light.enabled) {
                continue; // Skip disabled lights
            }
            
            GPULightData gpuLight;
            
            // Position and type
            gpuLight.position = glm::vec4(light.position, static_cast<float>(static_cast<int>(light.type)));
            
            // Direction and intensity
            gpuLight.direction = glm::vec4(light.direction, light.intensity);
            
            // Color and constant attenuation
            gpuLight.color = glm::vec4(light.color, light.constant);
            
            // Linear, quadratic, cutOff, outerCutOff
            gpuLight.attenuation = glm::vec4(light.linear, light.quadratic, light.cutOff, light.outerCutOff);
            
            gpuData.push_back(gpuLight);
        }
        
        return gpuData;
    }
    
    void CreateDefaultLights() {
        // Create a directional light (like sun)
        Light directionalLight;
        directionalLight.type = LightType::DIRECTIONAL;
        directionalLight.direction = glm::normalize(glm::vec3(0.5f, -1.0f, 0.3f));
        directionalLight.color = glm::vec3(1.0f, 0.95f, 0.8f); // Warm sunlight
        directionalLight.intensity = 1.0f;
        directionalLight.name = "SunLight";
        AddLight(directionalLight);
        
        // Create a point light
        Light pointLight;
        pointLight.type = LightType::POINT;
        pointLight.position = glm::vec3(2.0f, 2.0f, 2.0f);
        pointLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
        pointLight.intensity = 1.5f;
        pointLight.SetAttenuation(1.0f, 0.09f, 0.032f);
        pointLight.name = "PointLight1";
        AddLight(pointLight);
        
        // Create another point light with different color
        Light pointLight2;
        pointLight2.type = LightType::POINT;
        pointLight2.position = glm::vec3(-2.0f, 1.0f, -2.0f);
        pointLight2.color = glm::vec3(0.8f, 0.9f, 1.0f); // Slightly blue
        pointLight2.intensity = 1.0f;
        pointLight2.SetAttenuation(1.0f, 0.09f, 0.032f);
        pointLight2.name = "PointLight2";
        AddLight(pointLight2);
        
        std::cout << "Created " << GetActiveLightCount() << " default lights\n";
    }
    
    void ClearAllLights() {
        s_lights.clear();
    }
}
