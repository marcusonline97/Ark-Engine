#pragma once 

#include <glad/glad.h>
#include "Math/3DMath_util.h"

// TODO: try to merge with the Framebuffer class
class FramebufferObject
{
public:
    FramebufferObject();

    ~FramebufferObject();

    bool Init(unsigned int Width, unsigned int Height, bool ForPCF = false);

    void BindForWriting();

    void BindForReading(GLenum TextureUnit);

private:

    bool InitNonDSA(unsigned int Width, unsigned int Height, bool ForPCF = false);
    bool InitDSA(unsigned int Width, unsigned int Height, bool ForPCF = false);

    void BindForReadingNonDSA(GLenum TextureUnit);
    void BindForReadingDSA(GLenum TextureUnit);

    uint m_width = 0;
    uint m_height = 0;
    GLuint m_fbo;
    GLuint m_depthBuffer;
};


class CascadedShadowMapFBO
{
public:
    CascadedShadowMapFBO();

    ~CascadedShadowMapFBO();

    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);

    void BindForWriting(uint CascadeIndex);

    void BindForReading();

private:
    GLuint m_fbo;
    GLuint m_shadowMap[3];
};