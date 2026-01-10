#pragma once
#include "../Common.h"

// MipMap settings - can be scene-wide or per-entity
namespace MipmapSettings {
    
    // Scene-wide mipmap settings
    void SetGlobalMipmapEnabled(bool enabled);
    bool GetGlobalMipmapEnabled();
    
    // Set default mipmap generation for new textures
    void SetDefaultGenerateMipmaps(bool generate);
    bool GetDefaultGenerateMipmaps();
    
    // Set mipmap filtering mode
    // GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR
    void SetMipmapFilterMode(GLenum filterMode);
    GLenum GetMipmapFilterMode();
    
    // Initialize mipmap settings with defaults
    void Initialize();
}
