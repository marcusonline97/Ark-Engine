#include "Rendering/Framebuffer.h"

#include <algorithm>
#include <glad/glad.h>

Framebuffer::~Framebuffer()
{
    Destroy();
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
{
    *this = std::move(other);
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this == &other) return *this;
    Destroy();

    m_fbo = other.m_fbo;
    m_colorTex = other.m_colorTex;
    m_depthRbo = other.m_depthRbo;
    m_width = other.m_width;
    m_height = other.m_height;

    other.m_fbo = 0;
    other.m_colorTex = 0;
    other.m_depthRbo = 0;
    other.m_width = 0;
    other.m_height = 0;

    return *this;
}

bool Framebuffer::Create(uint32_t width, uint32_t height)
{
    Destroy();
    return Allocate(width, height);
}

void Framebuffer::Destroy()
{
    if (m_depthRbo)
    {
        glDeleteRenderbuffers(1, &m_depthRbo);
        m_depthRbo = 0;
    }
    if (m_colorTex)
    {
        glDeleteTextures(1, &m_colorTex);
        m_colorTex = 0;
    }
    if (m_fbo)
    {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    m_width = 0;
    m_height = 0;
}

bool Framebuffer::Resize(uint32_t width, uint32_t height)
{
    width = std::max<uint32_t>(1, width);
    height = std::max<uint32_t>(1, height);

    if (m_fbo && width == m_width && height == m_height)
        return true;

    return Create(width, height);
}

bool Framebuffer::Allocate(uint32_t width, uint32_t height)
{
    width = std::max<uint32_t>(1, width);
    height = std::max<uint32_t>(1, height);

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Color texture
    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

    // Depth (renderbuffer)
    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        Destroy();
        return false;
    }

    m_width = width;
    m_height = height;
    return true;
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

