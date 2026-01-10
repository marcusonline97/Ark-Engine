#pragma once
#include "../Common.h"
#include "../Renderer/Types/Texture.h"
#include <string>
#include <memory>

// Simple texture loader wrapper for Phase 3
// Leverages existing Texture class but provides simpler interface for material system
namespace TextureLoader {
    
    // Load a texture from file
    // Returns a shared pointer to the loaded texture, or nullptr on failure
    std::shared_ptr<Texture> LoadTexture(const std::string& filepath, bool generateMipmaps = true);
    
    // Bind a texture to a specific texture unit
    // For OpenGL: slot should be GL_TEXTURE0, GL_TEXTURE1, etc.
    void BindTexture(std::shared_ptr<Texture> texture, GLenum slot = GL_TEXTURE0);
    
    // Unbind texture from slot
    void UnbindTexture(GLenum slot = GL_TEXTURE0);
    
    // Set texture parameters (wrapping, filtering, mipmaps)
    void SetTextureParameters(GLuint textureID, bool generateMipmaps = true, 
                              GLenum wrapS = GL_REPEAT, GLenum wrapT = GL_REPEAT,
                              GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR, 
                              GLenum magFilter = GL_LINEAR);
    
    // Enable/disable mipmaps for a texture
    void SetMipmapEnabled(GLuint textureID, bool enabled);
}
