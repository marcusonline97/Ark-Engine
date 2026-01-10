#pragma once
#include "../Common.h"
#include "../Renderer/RendererCommon.h"
#include <string>
#include <vector>

// Structure to hold loaded mesh data before uploading to GPU
struct LoadedMesh {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
};

// Structure to hold OpenGL buffer handles for a mesh
struct MeshBuffers {
    GLuint VAO = 0;  // Vertex Array Object
    GLuint VBO = 0;  // Vertex Buffer Object
    GLuint EBO = 0;  // Element Buffer Object
    uint32_t indexCount = 0;
    
    bool IsValid() const {
        return VAO != 0 && VBO != 0 && EBO != 0;
    }
};

namespace OBJLoader {
    
    // Load OBJ file and return meshes (one per shape in the OBJ file)
    // Returns true on success, false on failure
    bool LoadOBJFile(const std::string& filepath, std::vector<LoadedMesh>& outMeshes, std::string& outError);
    
    // Upload mesh data to OpenGL buffers (VBO, VAO, EBO)
    // Returns true on success, false on failure
    bool UploadMeshToGPU(const LoadedMesh& mesh, MeshBuffers& outBuffers);
    
    // Clean up OpenGL buffers for a mesh
    void DeleteMeshBuffers(MeshBuffers& buffers);
    
    // Calculate AABB for a set of vertices
    void CalculateAABB(const std::vector<Vertex>& vertices, glm::vec3& outMin, glm::vec3& outMax);
    
    // Generate tangents for a mesh (needed for normal mapping)
    void GenerateTangents(LoadedMesh& mesh);
}
