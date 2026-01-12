#pragma once
#include <string>
#include <cstdint>

class Texture {
public:
    Texture() = default;
    ~Texture();

    // Load 2D texture from file
    // srgb: use GL_SRGB8_ALPHA8 for correct gamma
    // generateMipmaps: auto-generate mipmaps
    bool Load2D(const std::string& path, bool srgb = true, bool generateMipmaps = true);

    // For procedural textures/cpu data
    bool Create2D(uint32_t width, uint32_t height, int internalFormat = 0x8C43 /*GL_SRGB8_ALPHA8*/, bool generateMipmaps = true);

    void BindUnit(unsigned unit) const;
    void Unbind() const;

    unsigned Handle() const { return m_handle; }
    int Width()  const { return m_width; }
    int Height() const { return m_height; }

private:
    unsigned m_handle = 0;
    int m_width = 0;
    int m_height = 0;
    bool m_hasMipmaps = false;
}