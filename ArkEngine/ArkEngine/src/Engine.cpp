#include "Engine.h"
#include "BackEnd/BackEnd.h"
#include "API/OpenGL/GL_BackEnd.h"
#include "API/Vulkan/VK_BackEnd.h"
#include "Core/Camera.h"
#include "Core/OBJLoader.h"
#include "Core/Material.h"
#include "Core/MipmapSettings.h"
#include "Core/Light.h"
#include "Core/LightManager.h"
#include "ECS/ECS.h"
#include "ECS/Components.h"
#include "ECS/Systems.h"
#include "Renderer/Shader.h"
#include "Renderer/RendererCommon.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <memory>

// Global state for Phase 2, 3, 4 & ECS
static Camera* g_camera = nullptr;
static Shader* g_phongShader = nullptr;  // Phase 4: Phong lighting shader
static std::vector<LoadedMesh> g_loadedMeshes;
static std::vector<MeshBuffers> g_meshBuffers;
static std::shared_ptr<Material> g_defaultMaterial = nullptr;  // Phase 3: Default material
static std::vector<Entity> g_sceneEntities;  // ECS: Entities in the scene
static bool g_firstMouse = true;
static double g_lastX = 640.0;
static double g_lastY = 360.0;
static float g_deltaTime = 0.0f;
static float g_lastFrame = 0.0f;

// Mouse callback for camera look
void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (g_camera == nullptr) return;
    
    if (g_firstMouse) {
        g_lastX = xpos;
        g_lastY = ypos;
        g_firstMouse = false;
        return;
    }
    
    float xoffset = static_cast<float>(xpos - g_lastX);
    float yoffset = static_cast<float>(g_lastY - ypos); // Reversed since y-coordinates go from bottom to top
    
    g_lastX = xpos;
    g_lastY = ypos;
    
    g_camera->ProcessMouseMovement(xoffset, yoffset);
}

// Scroll callback for camera FOV
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (g_camera == nullptr) return;
    g_camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

void Engine::Run() {

    // Initialize backend - tries OpenGL first, falls back to Vulkan if OpenGL fails
    BackEnd::Init(API::OPENGL);
    
    // Only proceed if OpenGL is available
    if (BackEnd::GetAPI() != API::OPENGL) {
        std::cout << "Phase 2 requires OpenGL. Vulkan support coming in later phases.\n";
        BackEnd::CleanUp();
        return;
    }

    GLFWwindow* window = BackEnd::GetWindowPointer();
    if (window == nullptr) {
        std::cout << "ERROR: Window is null, cannot proceed\n";
        BackEnd::CleanUp();
        return;
    }
    
    // Initialize mipmap settings (Phase 3)
    MipmapSettings::Initialize();
    MipmapSettings::SetGlobalMipmapEnabled(true);
    MipmapSettings::SetDefaultGenerateMipmaps(true);
    
    // Phase 4: Initialize light manager
    LightManager::Initialize();
    LightManager::CreateDefaultLights();
    std::cout << "Light Manager initialized with " << LightManager::GetActiveLightCount() << " lights\n";
    
    // Initialize camera
    g_camera = new Camera();
    int width = BackEnd::GetCurrentWindowWidth();
    int height = BackEnd::GetCurrentWindowHeight();
    float aspectRatio = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 16.0f / 9.0f;
    g_camera->Initialize(glm::vec3(0.0f, 0.0f, 5.0f), -90.0f, 0.0f);
    g_camera->SetAspectRatio(aspectRatio);
    g_camera->SetNearPlane(0.1f);
    g_camera->SetFarPlane(100.0f);
    
    // Set mouse callbacks
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    
    // Disable cursor (for FPS camera)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Phase 4: Load Phong lighting shader
    g_phongShader = new Shader();
    if (!g_phongShader->LoadFromFiles("phong.vert", "phong.frag")) {
        std::cout << "WARNING: Failed to load Phong shader, trying basic shader...\n";
        // Fallback to basic shader
        if (!g_phongShader->LoadFromFiles("basic.vert", "basic.frag")) {
            std::cout << "ERROR: Failed to load any shader. Using fallback rendering.\n";
        }
    }
    else {
        std::cout << "Phong lighting shader loaded successfully!\n";
    }
    
    // Phase 3: Create default material
    g_defaultMaterial = std::make_shared<Material>("DefaultMaterial");
    g_defaultMaterial->SetShininess(32.0f);
    g_defaultMaterial->SetMipmapEnabled(true);
    // Try to load a default diffuse texture if available, otherwise use fallback color
    // For now, we'll use the fallback color in the shader
    std::cout << "Created default material: " << g_defaultMaterial->GetName() << "\n";
    
    // Try to load an OBJ file (if available)
    // TODO: Make this configurable or scan for OBJ files
    std::string testObjPath = "Resources/Models/OBJ/test.obj"; // Example path
    std::vector<LoadedMesh> meshes;
    std::string error;
    
    // Try different possible paths for OBJ files
    std::vector<std::string> possiblePaths = {
        "Resources/Models/OBJ/test.obj",
        "res/models/test.obj",
        "Resources/Models/test.obj",
    };
    
    bool modelLoaded = false;
    for (const auto& path : possiblePaths) {
        if (OBJLoader::LoadOBJFile(path, meshes, error)) {
            std::cout << "Successfully loaded OBJ file: " << path << "\n";
            std::cout << "  Meshes: " << meshes.size() << "\n";
            g_loadedMeshes = meshes;
            modelLoaded = true;
            break;
        }
    }
    
    if (!modelLoaded) {
        std::cout << "No OBJ file found. Creating a simple test quad manually.\n";
        // Create a simple quad for testing
        LoadedMesh quadMesh;
        quadMesh.name = "TestQuad";
        
        // Create vertices with position, normal, UV
        Vertex v0, v1, v2, v3;
        v0.position = glm::vec3(-1.0f, -1.0f, 0.0f);
        v0.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        v0.uv = glm::vec2(0.0f, 0.0f);
        v0.tangent = glm::vec3(0.0f);
        
        v1.position = glm::vec3( 1.0f, -1.0f, 0.0f);
        v1.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        v1.uv = glm::vec2(1.0f, 0.0f);
        v1.tangent = glm::vec3(0.0f);
        
        v2.position = glm::vec3( 1.0f,  1.0f, 0.0f);
        v2.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        v2.uv = glm::vec2(1.0f, 1.0f);
        v2.tangent = glm::vec3(0.0f);
        
        v3.position = glm::vec3(-1.0f,  1.0f, 0.0f);
        v3.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        v3.uv = glm::vec2(0.0f, 1.0f);
        v3.tangent = glm::vec3(0.0f);
        
        quadMesh.vertices = { v0, v1, v2, v3 };
        quadMesh.indices = { 0, 1, 2, 2, 3, 0 };
        OBJLoader::GenerateTangents(quadMesh);
        OBJLoader::CalculateAABB(quadMesh.vertices, quadMesh.aabbMin, quadMesh.aabbMax);
        g_loadedMeshes.push_back(quadMesh);
        modelLoaded = true;
    }
    
    // Upload meshes to GPU
    g_meshBuffers.resize(g_loadedMeshes.size());
    for (size_t i = 0; i < g_loadedMeshes.size(); i++) {
        if (!OBJLoader::UploadMeshToGPU(g_loadedMeshes[i], g_meshBuffers[i])) {
            std::cout << "ERROR: Failed to upload mesh " << i << " to GPU\n";
        }
        else {
            std::cout << "Uploaded mesh " << i << " (" << g_loadedMeshes[i].name << ") to GPU\n";
        }
    }
    
    // ECS: Create entities for each loaded mesh
    g_sceneEntities.clear();
    g_sceneEntities.reserve(g_loadedMeshes.size());
    
    for (size_t i = 0; i < g_loadedMeshes.size(); i++) {
        // Create entity with name
        Entity entity = ECS::CreateEntity("Mesh_" + g_loadedMeshes[i].name);
        
        // Add transform component
        auto& transform = ECS::AddComponent<TransformComponent>(entity);
        transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
        transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
        transform.MarkDirty();
        
        // Add mesh component
        auto& mesh = ECS::AddComponent<MeshComponent>(entity);
        mesh.meshBufferIndex = i;
        mesh.materialPtr = static_cast<void*>(g_defaultMaterial.get());  // Cast Material* to void*
        mesh.visible = true;
        
        // Add render component
        auto& render = ECS::AddComponent<RenderComponent>(entity);
        render.castShadows = true;
        render.receiveShadows = true;
        render.layer = 0;
        
        // Add tag for easy querying
        ECS::SetTag(entity, "Renderable");
        
        g_sceneEntities.push_back(entity);
        
        std::cout << "Created ECS entity: " << ECS::GetName(entity) << " (ID: " << static_cast<uint32_t>(entity) << ")\n";
    }
    
    // ECS: Example parent/child relationship - make first entity parent of others if multiple exist
    if (g_sceneEntities.size() > 1) {
        Entity parentEntity = g_sceneEntities[0];
        for (size_t i = 1; i < g_sceneEntities.size(); i++) {
            Entity childEntity = g_sceneEntities[i];
            
            // Offset child position relative to parent
            auto* childTransform = ECS::GetTransform(childEntity);
            if (childTransform) {
                childTransform->position = glm::vec3(static_cast<float>(i) * 2.0f, 0.0f, 0.0f);
                childTransform->MarkDirty();
            }
            
            // Set parent/child relationship
            ECS::SetParent(childEntity, parentEntity);
            std::cout << "Set " << ECS::GetName(childEntity) << " as child of " << ECS::GetName(parentEntity) << "\n";
        }
    }
    
    std::cout << "ECS System initialized with " << ECS::GetEntityCount() << " entities\n";

    // Main render loop
    while (BackEnd::WindowIsOpen()) {
        
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        g_deltaTime = currentFrame - g_lastFrame;
        g_lastFrame = currentFrame;
        
        // Handle input
        BackEnd::BeginFrame();
        
        // ECS: Update transform hierarchy (must be called before rendering)
        ECSSystems::UpdateTransformHierarchy();
        
        // Process keyboard input for camera
        if (g_camera != nullptr) {
            g_camera->ProcessKeyboard(GLFW_KEY_W, glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
            g_camera->ProcessKeyboard(GLFW_KEY_S, glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
            g_camera->ProcessKeyboard(GLFW_KEY_A, glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
            g_camera->ProcessKeyboard(GLFW_KEY_D, glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
            g_camera->ProcessKeyboard(GLFW_KEY_SPACE, glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
            g_camera->ProcessKeyboard(GLFW_KEY_LEFT_CONTROL, glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
        }
        
        // Update camera
        if (g_camera != nullptr) {
            g_camera->Update(g_deltaTime);
            
            // Update aspect ratio on window resize
            int newWidth = BackEnd::GetCurrentWindowWidth();
            int newHeight = BackEnd::GetCurrentWindowHeight();
            if (newWidth > 0 && newHeight > 0) {
                float newAspect = static_cast<float>(newWidth) / static_cast<float>(newHeight);
                if (std::abs(newAspect - g_camera->GetAspectRatio()) > 0.01f) {
                    g_camera->SetAspectRatio(newAspect);
                }
            }
        }
        
        // ECS: Example - rotate root entities over time (optional demo)
        static float rotationSpeed = 30.0f; // degrees per second
        for (Entity entity : g_sceneEntities) {
            auto* transform = ECS::GetTransform(entity);
            if (transform && !ECS::HasParent(entity)) {  // Only rotate root entities
                transform->rotation.y += rotationSpeed * g_deltaTime;
                transform->MarkDirty();
            }
        }
        
        // Check for ESC key to close window
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            BackEnd::ForceCloseWindow();
        }
        
        // Rendering
        if (BackEnd::GetAPI() == API::OPENGL) {
            // Clear screen
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Phase 4: Use Phong shader for lighting
            if (g_phongShader != nullptr && g_phongShader->IsValid()) {
                g_phongShader->Use();
                
                // Set camera matrices (same for all entities)
                if (g_camera != nullptr) {
                    g_phongShader->SetMat4("uView", g_camera->GetViewMatrix());
                    g_phongShader->SetMat4("uProjection", g_camera->GetProjectionMatrix());
                    
                    // Phase 4: Set view position for lighting calculations
                    g_phongShader->SetVec3("uViewPos", g_camera->GetPosition());
                }
                
                // Phase 3: Bind material textures (shared for all entities for now)
                if (g_defaultMaterial != nullptr) {
                    // Bind material textures to texture units
                    g_defaultMaterial->Bind(GL_TEXTURE0, GL_TEXTURE1);
                    
                    // Set shader uniforms for material
                    g_phongShader->SetInt("uDiffuseTexture", 0);   // Texture unit 0
                    g_phongShader->SetInt("uSpecularTexture", 1);  // Texture unit 1
                    g_phongShader->SetFloat("uShininess", g_defaultMaterial->GetShininess());
                }
                else {
                    // Fallback: set texture units to 0 and 1, but shader will use default
                    g_phongShader->SetInt("uDiffuseTexture", 0);
                    g_phongShader->SetInt("uSpecularTexture", 1);
                    g_phongShader->SetFloat("uShininess", 32.0f);
                }
                
                // Set fallback color (used if no texture)
                g_phongShader->SetVec3("uColor", glm::vec3(0.8f, 0.3f, 0.2f));
                
                // Phase 4: Upload light data to shader
                std::vector<GPULightData> gpuLights = LightManager::GetGPULightData();
                int lightCount = static_cast<int>(gpuLights.size());
                
                // Clamp to maximum
                if (lightCount > LightManager::MAX_LIGHTS) {
                    lightCount = LightManager::MAX_LIGHTS;
                    gpuLights.resize(lightCount);
                }
                
                g_phongShader->SetInt("uLightCount", lightCount);
                
                // Upload lights (OpenGL uniforms for struct arrays)
                // Note: Setting struct members using "uLights[index].member" syntax
                for (int i = 0; i < lightCount; i++) {
                    std::string baseName = "uLights[" + std::to_string(i) + "]";
                    g_phongShader->SetVec4(baseName + ".position", gpuLights[i].position);
                    g_phongShader->SetVec4(baseName + ".direction", gpuLights[i].direction);
                    g_phongShader->SetVec4(baseName + ".color", gpuLights[i].color);
                    g_phongShader->SetVec4(baseName + ".attenuation", gpuLights[i].attenuation);
                }
                
                // ECS: Render entities using ECS system
                // Query all entities with MeshComponent and TransformComponent
                auto view = ECS::View<MeshComponent, TransformComponent>();
                for (auto entity : view) {
                    auto& mesh = view.get<MeshComponent>(entity);
                    auto& transform = view.get<TransformComponent>(entity);
                    
                    // Skip if not visible
                    if (!mesh.visible) {
                        continue;
                    }
                    
                    // Get world matrix from transform component
                    glm::mat4 worldMatrix = transform.GetWorldMatrix();
                    
                    // Set model matrix for this entity
                    g_phongShader->SetMat4("uModel", worldMatrix);
                    
                    // Draw mesh
                    if (mesh.meshBufferIndex < g_meshBuffers.size()) {
                        const auto& buffers = g_meshBuffers[mesh.meshBufferIndex];
                        if (buffers.IsValid()) {
                            glBindVertexArray(buffers.VAO);
                            glDrawElements(GL_TRIANGLES, buffers.indexCount, GL_UNSIGNED_INT, 0);
                            glBindVertexArray(0);
                        }
                    }
                }
                
                // Phase 3: Unbind material textures
                if (g_defaultMaterial != nullptr) {
                    g_defaultMaterial->Unbind(GL_TEXTURE0, GL_TEXTURE1);
                }
            }
            else {
                // Fallback: just clear screen
                glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
        }
        
        BackEnd::EndFrame();
    }
    
    // Cleanup
    for (auto& buffer : g_meshBuffers) {
        OBJLoader::DeleteMeshBuffers(buffer);
    }
    g_meshBuffers.clear();
    g_loadedMeshes.clear();
    
    // ECS: Cleanup entities
    g_sceneEntities.clear();
    ECS::Clear();
    std::cout << "ECS System cleaned up\n";
    
    // Phase 3: Cleanup material
    g_defaultMaterial.reset();
    g_defaultMaterial = nullptr;
    
    // Phase 4: Cleanup lights
    LightManager::Cleanup();
    
    if (g_phongShader != nullptr) {
        delete g_phongShader;
        g_phongShader = nullptr;
    }
    
    if (g_camera != nullptr) {
        delete g_camera;
        g_camera = nullptr;
    }

    BackEnd::CleanUp();
}