#pragma once

#include <cstdint>

class Framebuffer
{
public:
    Framebuffer() = default;
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    bool Create(uint32_t width, uint32_t height);
    void Destroy();

    bool Resize(uint32_t width, uint32_t height);

    void Bind() const;
    static void Unbind();

    uint32_t GetColorTextureId() const { return m_colorTex; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

private:
    bool Allocate(uint32_t width, uint32_t height);

private:
    uint32_t m_fbo = 0;
    uint32_t m_colorTex = 0;
    uint32_t m_depthRbo = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};

