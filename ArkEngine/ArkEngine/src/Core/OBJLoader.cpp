#include "OBJLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "../../Vendor/tinyobjloader/tiny_obj_loader.h"
#include <unordered_map>
#include <iostream>

namespace OBJLoader {

    void CalculateAABB(const std::vector<Vertex>& vertices, glm::vec3& outMin, glm::vec3& outMax) {
        if (vertices.empty()) {
            outMin = glm::vec3(0.0f);
            outMax = glm::vec3(0.0f);
            return;
        }
        
        outMin = glm::vec3(std::numeric_limits<float>::max());
        outMax = glm::vec3(-std::numeric_limits<float>::max());
        
        for (const auto& vertex : vertices) {
            outMin.x = std::min(outMin.x, vertex.position.x);
            outMin.y = std::min(outMin.y, vertex.position.y);
            outMin.z = std::min(outMin.z, vertex.position.z);
            
            outMax.x = std::max(outMax.x, vertex.position.x);
            outMax.y = std::max(outMax.y, vertex.position.y);
            outMax.z = std::max(outMax.z, vertex.position.z);
        }
    }

    void GenerateTangents(LoadedMesh& mesh) {
        std::vector<Vertex>& vertices = mesh.vertices;
        std::vector<uint32_t>& indices = mesh.indices;
        
        // Generate tangents for each triangle
        for (size_t i = 0; i < indices.size(); i += 3) {
            if (i + 2 >= indices.size()) break;
            
            uint32_t idx0 = indices[i];
            uint32_t idx1 = indices[i + 1];
            uint32_t idx2 = indices[i + 2];
            
            if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size()) {
                continue;
            }
            
            Vertex& v0 = vertices[idx0];
            Vertex& v1 = vertices[idx1];
            Vertex& v2 = vertices[idx2];
            
            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;
            glm::vec2 deltaUV1 = v1.uv - v0.uv;
            glm::vec2 deltaUV2 = v2.uv - v0.uv;
            
            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            
            glm::vec3 tangent;
            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
            tangent = glm::normalize(tangent);
            
            // Assign tangent to all three vertices (can be improved with proper averaging)
            v0.tangent = tangent;
            v1.tangent = tangent;
            v2.tangent = tangent;
        }
    }

    bool LoadOBJFile(const std::string& filepath, std::vector<LoadedMesh>& outMeshes, std::string& outError) {
        outMeshes.clear();
        outError.clear();
        
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;
        
        // Load the OBJ file using tinyobjloader
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            outError = "Failed to load OBJ file: " + filepath;
            if (!err.empty()) {
                outError += "\nError: " + err;
            }
            if (!warn.empty()) {
                std::cout << "Warning while loading " << filepath << ": " << warn << "\n";
            }
            return false;
        }
        
        if (!warn.empty()) {
            std::cout << "Warning while loading " << filepath << ": " << warn << "\n";
        }
        
        if (!err.empty()) {
            std::cout << "Error while loading " << filepath << ": " << err << "\n";
        }
        
        // Process each shape in the OBJ file as a separate mesh
        for (const auto& shape : shapes) {
            LoadedMesh mesh;
            mesh.name = shape.name.empty() ? "UnnamedMesh" : shape.name;
            
            std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            
            // Process each face (triangle)
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex;
                
                // Position (required)
                if (index.vertex_index >= 0 && size_t(index.vertex_index * 3 + 2) < attrib.vertices.size()) {
                    vertex.position.x = attrib.vertices[3 * size_t(index.vertex_index) + 0];
                    vertex.position.y = attrib.vertices[3 * size_t(index.vertex_index) + 1];
                    vertex.position.z = attrib.vertices[3 * size_t(index.vertex_index) + 2];
                }
                else {
                    outError = "Invalid vertex index in shape: " + mesh.name;
                    return false;
                }
                
                // Normal (optional - will generate if missing)
                if (index.normal_index >= 0 && size_t(index.normal_index * 3 + 2) < attrib.normals.size()) {
                    vertex.normal.x = attrib.normals[3 * size_t(index.normal_index) + 0];
                    vertex.normal.y = attrib.normals[3 * size_t(index.normal_index) + 1];
                    vertex.normal.z = attrib.normals[3 * size_t(index.normal_index) + 2];
                }
                else {
                    vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f); // Default normal (will need to generate properly)
                }
                
                // UV coordinates (optional)
                if (index.texcoord_index >= 0 && size_t(index.texcoord_index * 2 + 1) < attrib.texcoords.size()) {
                    vertex.uv.x = attrib.texcoords[2 * size_t(index.texcoord_index) + 0];
                    vertex.uv.y = 1.0f - attrib.texcoords[2 * size_t(index.texcoord_index) + 1]; // Flip V coordinate
                }
                else {
                    vertex.uv = glm::vec2(0.0f, 0.0f); // Default UV
                }
                
                vertex.tangent = glm::vec3(0.0f); // Will be calculated later
                
                // Check if this vertex already exists (deduplication)
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                
                indices.push_back(uniqueVertices[vertex]);
            }
            
            // Generate normals if missing (simple approach - could be improved)
            bool needsNormalGeneration = false;
            for (const auto& vertex : vertices) {
                if (glm::length(vertex.normal) < 0.01f) {
                    needsNormalGeneration = true;
                    break;
                }
            }
            
            if (needsNormalGeneration) {
                // Reset all normals
                for (auto& vertex : vertices) {
                    vertex.normal = glm::vec3(0.0f);
                }
                
                // Calculate normals from faces
                for (size_t i = 0; i < indices.size(); i += 3) {
                    if (i + 2 >= indices.size()) break;
                    
                    uint32_t idx0 = indices[i];
                    uint32_t idx1 = indices[i + 1];
                    uint32_t idx2 = indices[i + 2];
                    
                    if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size()) {
                        continue;
                    }
                    
                    glm::vec3 v0 = vertices[idx0].position;
                    glm::vec3 v1 = vertices[idx1].position;
                    glm::vec3 v2 = vertices[idx2].position;
                    
                    glm::vec3 edge1 = v1 - v0;
                    glm::vec3 edge2 = v2 - v0;
                    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
                    
                    vertices[idx0].normal += faceNormal;
                    vertices[idx1].normal += faceNormal;
                    vertices[idx2].normal += faceNormal;
                }
                
                // Normalize all normals
                for (auto& vertex : vertices) {
                    if (glm::length(vertex.normal) > 0.01f) {
                        vertex.normal = glm::normalize(vertex.normal);
                    }
                    else {
                        vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f); // Fallback
                    }
                }
            }
            
            // Generate tangents
            mesh.vertices = vertices;
            mesh.indices = indices;
            GenerateTangents(mesh);
            
            // Calculate AABB
            CalculateAABB(mesh.vertices, mesh.aabbMin, mesh.aabbMax);
            
            outMeshes.push_back(mesh);
        }
        
        if (outMeshes.empty()) {
            outError = "No meshes found in OBJ file: " + filepath;
            return false;
        }
        
        return true;
    }

    bool UploadMeshToGPU(const LoadedMesh& mesh, MeshBuffers& outBuffers) {
        if (mesh.vertices.empty() || mesh.indices.empty()) {
            std::cout << "Error: Cannot upload empty mesh to GPU\n";
            return false;
        }
        
        // Generate VAO, VBO, and EBO
        glGenVertexArrays(1, &outBuffers.VAO);
        glGenBuffers(1, &outBuffers.VBO);
        glGenBuffers(1, &outBuffers.EBO);
        
        if (outBuffers.VAO == 0 || outBuffers.VBO == 0 || outBuffers.EBO == 0) {
            std::cout << "Error: Failed to generate OpenGL buffers for mesh: " << mesh.name << "\n";
            DeleteMeshBuffers(outBuffers);
            return false;
        }
        
        // Bind VAO
        glBindVertexArray(outBuffers.VAO);
        
        // Upload vertex data to VBO
        glBindBuffer(GL_ARRAY_BUFFER, outBuffers.VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
        
        // Upload index data to EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outBuffers.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);
        
        // Set vertex attribute pointers
        // Position (location 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        
        // Normal (location 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        
        // UV (location 2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        
        // Tangent (location 3)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
        
        // Unbind VAO
        glBindVertexArray(0);
        
        outBuffers.indexCount = static_cast<uint32_t>(mesh.indices.size());
        
        return true;
    }

    void DeleteMeshBuffers(MeshBuffers& buffers) {
        if (buffers.VAO != 0) {
            glDeleteVertexArrays(1, &buffers.VAO);
            buffers.VAO = 0;
        }
        if (buffers.VBO != 0) {
            glDeleteBuffers(1, &buffers.VBO);
            buffers.VBO = 0;
        }
        if (buffers.EBO != 0) {
            glDeleteBuffers(1, &buffers.EBO);
            buffers.EBO = 0;
        }
        buffers.indexCount = 0;
    }
}
