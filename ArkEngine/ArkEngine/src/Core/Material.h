#pragma once
#include "../Common.h"
#include "TextureLoader.h"
#include "../Renderer/Types/Texture.h"
#include <memory>
#include <string>

// Material class for Phase 3
// Provides diffuse/albedo texture, specular texture, and shininess value
// Can be assigned per-entity (or per-mesh)
class Material {
public:
    Material();
    Material(const std::string& name);
    ~Material();
    
    // Material name
    void SetName(const std::string& name) { _name = name; }
    const std::string& GetName() const { return _name; }
    
    // Diffuse/Albedo texture
    void SetDiffuseTexture(std::shared_ptr<Texture> texture);
    void SetDiffuseTexture(const std::string& filepath);
    std::shared_ptr<Texture> GetDiffuseTexture() const { return _diffuseTexture; }
    bool HasDiffuseTexture() const { return _diffuseTexture != nullptr; }
    
    // Specular texture
    void SetSpecularTexture(std::shared_ptr<Texture> texture);
    void SetSpecularTexture(const std::string& filepath);
    std::shared_ptr<Texture> GetSpecularTexture() const { return _specularTexture; }
    bool HasSpecularTexture() const { return _specularTexture != nullptr; }
    
    // Shininess value (for Phong lighting)
    void SetShininess(float shininess) { _shininess = shininess; }
    float GetShininess() const { return _shininess; }
    
    // MipMap settings
    void SetMipmapEnabled(bool enabled) { _mipmapEnabled = enabled; }
    bool GetMipmapEnabled() const { return _mipmapEnabled; }
    
    // Bind material textures to OpenGL texture units
    // diffuseUnit: texture unit for diffuse texture (default GL_TEXTURE0)
    // specularUnit: texture unit for specular texture (default GL_TEXTURE1)
    void Bind(GLuint diffuseUnit = GL_TEXTURE0, GLuint specularUnit = GL_TEXTURE1) const;
    
    // Unbind material textures
    void Unbind(GLuint diffuseUnit = GL_TEXTURE0, GLuint specularUnit = GL_TEXTURE1) const;
    
    // Check if material is valid (has at least a diffuse texture or can use default)
    bool IsValid() const { return _diffuseTexture != nullptr || true; } // Always valid, can use default
    
    // Set default white texture if no diffuse texture is set
    void UseDefaultDiffuse(bool useDefault) { _useDefaultDiffuse = useDefault; }
    bool GetUseDefaultDiffuse() const { return _useDefaultDiffuse; }
    
private:
    std::string _name;
    std::shared_ptr<Texture> _diffuseTexture;
    std::shared_ptr<Texture> _specularTexture;
    float _shininess;
    bool _mipmapEnabled;
    bool _useDefaultDiffuse;
    
    // Default textures (for future use - currently handled via shader fallback)
    static std::shared_ptr<Texture> s_defaultWhiteTexture;
    static std::shared_ptr<Texture> s_defaultBlackTexture;
    static void CreateDefaultTextures();
};
