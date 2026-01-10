#include "Material.h"
#include "../BackEnd/BackEnd.h"
#include "../API/OpenGL/GL_BackEnd.h"
#include <iostream>
#include <cstring>

// Static default textures (currently unused, kept for future use)
std::shared_ptr<Texture> Material::s_defaultWhiteTexture = nullptr;
std::shared_ptr<Texture> Material::s_defaultBlackTexture = nullptr;

void Material::CreateDefaultTextures() {
    // Placeholder for future default texture creation
    // Default textures are handled in shader via fallback colors
}

Material::Material() 
    : _name("UnnamedMaterial"),
      _diffuseTexture(nullptr),
      _specularTexture(nullptr),
      _shininess(32.0f),
      _mipmapEnabled(true),
      _useDefaultDiffuse(true) {
}

Material::Material(const std::string& name)
    : _name(name),
      _diffuseTexture(nullptr),
      _specularTexture(nullptr),
      _shininess(32.0f),
      _mipmapEnabled(true),
      _useDefaultDiffuse(true) {
}

Material::~Material() {
    // Shared pointers will handle cleanup automatically
}

void Material::SetDiffuseTexture(std::shared_ptr<Texture> texture) {
    _diffuseTexture = texture;
    
    // Configure mipmaps if texture is valid
    if (_diffuseTexture != nullptr && _mipmapEnabled && BackEnd::GetAPI() == API::OPENGL) {
        OpenGLTexture& glTexture = _diffuseTexture->GetGLTexture();
        GLuint textureID = glTexture.GetID();
        if (textureID != 0) {
            TextureLoader::SetMipmapEnabled(textureID, _mipmapEnabled);
        }
    }
}

void Material::SetDiffuseTexture(const std::string& filepath) {
    std::shared_ptr<Texture> texture = TextureLoader::LoadTexture(filepath, _mipmapEnabled);
    if (texture != nullptr) {
        SetDiffuseTexture(texture);
    }
}

void Material::SetSpecularTexture(std::shared_ptr<Texture> texture) {
    _specularTexture = texture;
    
    // Configure mipmaps if texture is valid
    if (_specularTexture != nullptr && _mipmapEnabled && BackEnd::GetAPI() == API::OPENGL) {
        OpenGLTexture& glTexture = _specularTexture->GetGLTexture();
        GLuint textureID = glTexture.GetID();
        if (textureID != 0) {
            TextureLoader::SetMipmapEnabled(textureID, _mipmapEnabled);
        }
    }
}

void Material::SetSpecularTexture(const std::string& filepath) {
    std::shared_ptr<Texture> texture = TextureLoader::LoadTexture(filepath, _mipmapEnabled);
    if (texture != nullptr) {
        SetSpecularTexture(texture);
    }
}

// Static default textures (created once, reused)
static GLuint s_defaultWhiteTexture = 0;
static GLuint s_defaultBlackTexture = 0;
static bool s_defaultTexturesCreated = false;

static void CreateDefaultTextures() {
    if (s_defaultTexturesCreated || BackEnd::GetAPI() != API::OPENGL) {
        return;
    }
    
    // Create default white texture
    unsigned char whiteData[4] = { 255, 255, 255, 255 };
    glGenTextures(1, &s_defaultWhiteTexture);
    glBindTexture(GL_TEXTURE_2D, s_defaultWhiteTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Create default black texture
    unsigned char blackData[4] = { 0, 0, 0, 255 };
    glGenTextures(1, &s_defaultBlackTexture);
    glBindTexture(GL_TEXTURE_2D, s_defaultBlackTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    s_defaultTexturesCreated = true;
}

void Material::Bind(GLuint diffuseUnit, GLuint specularUnit) const {
    if (BackEnd::GetAPI() != API::OPENGL) {
        return;
    }
    
    // Create default textures if needed
    CreateDefaultTextures();
    
    // Bind diffuse texture
    if (_diffuseTexture != nullptr) {
        TextureLoader::BindTexture(_diffuseTexture, diffuseUnit);
    }
    else {
        // Bind default white texture
        glActiveTexture(diffuseUnit);
        glBindTexture(GL_TEXTURE_2D, s_defaultWhiteTexture);
    }
    
    // Bind specular texture
    if (_specularTexture != nullptr) {
        TextureLoader::BindTexture(_specularTexture, specularUnit);
    }
    else {
        // Bind default black texture (no specular)
        glActiveTexture(specularUnit);
        glBindTexture(GL_TEXTURE_2D, s_defaultBlackTexture);
    }
}

void Material::Unbind(GLuint diffuseUnit, GLuint specularUnit) const {
    if (BackEnd::GetAPI() != API::OPENGL) {
        return;
    }
    
    TextureLoader::UnbindTexture(diffuseUnit);
    TextureLoader::UnbindTexture(specularUnit);
}
