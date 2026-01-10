#include "TextureLoader.h"
#include "../BackEnd/BackEnd.h"
#include "../API/OpenGL/GL_BackEnd.h"
#include "MipmapSettings.h"
#include <iostream>
#include <vector>

namespace TextureLoader {
    
    std::shared_ptr<Texture> LoadTexture(const std::string& filepath, bool generateMipmaps) {
        if (BackEnd::GetAPI() != API::OPENGL) {
            std::cout << "WARNING: TextureLoader currently only supports OpenGL\n";
            return nullptr;
        }
        
        // Use global mipmap setting if generateMipmaps is not explicitly set
        // Check if global mipmaps are enabled
        bool shouldGenerateMipmaps = generateMipmaps && MipmapSettings::GetGlobalMipmapEnabled();
        
        auto texture = std::make_shared<Texture>();
        
        // Try multiple possible paths
        std::vector<std::string> possiblePaths = {
            filepath,
            "Resources/Textures/" + filepath,
            "res/textures/" + filepath,
            "Resources/Models/" + filepath,
            "res/models/" + filepath,
        };
        
        bool loaded = false;
        for (const auto& path : possiblePaths) {
            texture->Load(path);
            // Check if texture was loaded successfully by checking if width > 0
            if (texture->GetWidth() > 0) {
                loaded = true;
                break;
            }
        }
        
        if (!loaded) {
            std::cout << "WARNING: Failed to load texture: " << filepath << "\n";
            std::cout << "  Tried paths:\n";
            for (const auto& path : possiblePaths) {
                std::cout << "    - " << path << "\n";
            }
            return nullptr;
        }
        
        // If OpenGL, configure mipmaps
        if (BackEnd::GetAPI() == API::OPENGL && shouldGenerateMipmaps) {
            OpenGLTexture& glTexture = texture->GetGLTexture();
            GLuint textureID = glTexture.GetID();
            if (textureID != 0) {
                GLenum filterMode = MipmapSettings::GetMipmapFilterMode();
                SetTextureParameters(textureID, shouldGenerateMipmaps, GL_REPEAT, GL_REPEAT, filterMode, GL_LINEAR);
            }
        }
        
        return texture;
    }
    
    void BindTexture(std::shared_ptr<Texture> texture, GLenum slot) {
        if (texture == nullptr || BackEnd::GetAPI() != API::OPENGL) {
            return;
        }
        
        OpenGLTexture& glTexture = texture->GetGLTexture();
        GLuint textureID = glTexture.GetID();
        
        if (textureID == 0) {
            std::cout << "WARNING: Attempting to bind invalid texture\n";
            return;
        }
        
        // Activate texture unit and bind texture
        glActiveTexture(slot);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
    
    void UnbindTexture(GLenum slot) {
        if (BackEnd::GetAPI() != API::OPENGL) {
            return;
        }
        
        glActiveTexture(slot);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void SetTextureParameters(GLuint textureID, bool generateMipmaps, 
                              GLenum wrapS, GLenum wrapT,
                              GLenum minFilter, GLenum magFilter) {
        if (BackEnd::GetAPI() != API::OPENGL || textureID == 0) {
            return;
        }
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Set wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
        
        // Set filtering parameters
        if (generateMipmaps) {
            // Use provided min filter (should be a mipmap filter mode)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
            
            // Generate mipmaps
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            // No mipmaps, use linear filtering
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        }
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void SetMipmapEnabled(GLuint textureID, bool enabled) {
        if (BackEnd::GetAPI() != API::OPENGL || textureID == 0) {
            return;
        }
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        if (enabled) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
