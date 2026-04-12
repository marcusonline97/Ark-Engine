#include <stdio.h>
#include <glad/glad.h>

#include "Utility/Util.h"
#include "IO_Buffer.h"
#include "Rendering/Texture/Texture.h"


IOBuffer::IOBuffer()
{
    m_fbo = 0;
    m_texture = 0;
    m_depth = 0;
    m_internalType = GL_NONE;
}

IOBuffer::~IOBuffer()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_texture != 0) {
        glDeleteTextures(1, &m_texture);
    }

    if (m_depth != 0) {
        glDeleteTextures(1, &m_depth);
    }
}


bool IOBuffer::Init(uint WindowWidth, uint WindowHeight, bool WithDepth, GLenum InternalType)
{
    m_internalType = InternalType;

    GLenum Format = GL_NONE;
    GLenum Type = GL_NONE;

    switch (InternalType) {
    case GL_RGB32F:
        Format = GL_RGB;
        Type = GL_FLOAT;
        break;
    case GL_R32F:
        Format = GL_RED;
        Type = GL_FLOAT;
        break;
    case GL_NONE:
        break;
    default:
        OGLDEV_ERROR0("Invalid internal type");
        return false;
    }

    // Create the FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create the textures
    if (InternalType != GL_NONE) {
        glGenTextures(1, &m_texture);

        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalType, WindowWidth, WindowHeight, 0, Format, Type, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

        GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };

        glDrawBuffers(ARRAY_SIZE_IN_ELEMENTS(DrawBuffers), DrawBuffers);
    }

    // Create the depth buffer 
    if (WithDepth) {
        glGenTextures(1, &m_depth);

        // depth
        glBindTexture(GL_TEXTURE_2D, m_depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth, 0);
    }

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }

    // restore default FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return true;
}


void IOBuffer::BindForWriting()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}


void IOBuffer::BindForReading(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);

    if (m_internalType == GL_NONE) {
        glBindTexture(GL_TEXTURE_2D, m_depth);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, m_texture);
    }
}