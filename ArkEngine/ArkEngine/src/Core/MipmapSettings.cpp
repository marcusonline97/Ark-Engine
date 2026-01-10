#include "MipmapSettings.h"
#include "../BackEnd/BackEnd.h"

namespace MipmapSettings {
    
    static bool s_globalMipmapEnabled = true;
    static bool s_defaultGenerateMipmaps = true;
    static GLenum s_mipmapFilterMode = GL_LINEAR_MIPMAP_LINEAR;
    
    void Initialize() {
        s_globalMipmapEnabled = true;
        s_defaultGenerateMipmaps = true;
        s_mipmapFilterMode = GL_LINEAR_MIPMAP_LINEAR;
    }
    
    void SetGlobalMipmapEnabled(bool enabled) {
        s_globalMipmapEnabled = enabled;
    }
    
    bool GetGlobalMipmapEnabled() {
        return s_globalMipmapEnabled;
    }
    
    void SetDefaultGenerateMipmaps(bool generate) {
        s_defaultGenerateMipmaps = generate;
    }
    
    bool GetDefaultGenerateMipmaps() {
        return s_defaultGenerateMipmaps;
    }
    
    void SetMipmapFilterMode(GLenum filterMode) {
        s_mipmapFilterMode = filterMode;
    }
    
    GLenum GetMipmapFilterMode() {
        return s_mipmapFilterMode;
    }
}
