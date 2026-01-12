#include "Texture.h"
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::~Texture() {
    if (m_handle) {
        glDeleteTextures(1, &m_handle);
        m_handle = 0;
    }
}

bool Texture::Load2D(const std::string& path, bool srgb, bool generateMipmaps) {
    int w, h, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if (!data) return false;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);

    // Choose internal format (sRGB for albedo)
    GLenum internal = (channels == 4)
        ? (srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8)
        : (srgb ? GL_SRGB8 : GL_RGB8);
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    int mipLevels = generateMipmaps ? (int)std::floor(std::log2(std::max(w, h))) + 1 : 1;
    glTextureStorage2D(m_handle, mipLevels, internal, w, h);
    glTextureSubImage2D(m_handle, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, data);

    // Sampler state
    glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (generateMipmaps) {
        glGenerateTextureMipmap(m_handle);
        m_hasMipmaps = true;
    }

    stbi_image_free(data);
    m_width = w; m_height = h;
    return true;
}

bool Texture::Create2D(uint32_t width, uint32_t height, int internalFormat, bool generateMipmaps) {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
    int mipLevels = generateMipmaps ? (int)std::floor(std::log2(std::max(width, height))) + 1 : 1;
    glTextureStorage2D(m_handle, mipLevels, internalFormat, (int)width, (int)height);

    glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    m_width = (int)width; m_height = (int)height;
    m_hasMipmaps = generateMipmaps;
    return true;
}

void Texture::BindUnit(unsigned unit) const {
    glBindTextureUnit(unit, m_handle);
}

void Texture::Unbind() const {
    // not strictly necessary with DSA
}