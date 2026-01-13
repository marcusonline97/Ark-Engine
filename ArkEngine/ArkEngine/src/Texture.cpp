#include "Texture.h"
#include <glad/glad.h>
#include <stb_image.h>
#include <cmath>

Texture::~Texture() {
    if (m_handle) {
        glDeleteTextures(1, &m_handle);
        m_handle = 0;
    }
}

bool Texture::Load2D(const std::string& path, bool srgb, bool generateMipmaps, bool flipY) {
    int w, h, channels;
    stbi_set_flip_vertically_on_load(flipY ? 1 : 0);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if (!data) return false;

    if (m_handle)
    {
        glDeleteTextures(1, &m_handle);
        m_handle = 0;
    }

    glGenTextures(1, &m_handle);
    glBindTexture(GL_TEXTURE_2D, m_handle);

    GLenum internal = (channels == 4)
        ? (srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8)
        : (srgb ? GL_SRGB8 : GL_RGB8);
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, internal, w, h, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (generateMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
        m_hasMipmaps = true;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    m_width = w; m_height = h;
    return true;
}

bool Texture::Create2D(uint32_t width, uint32_t height, int internalFormat, bool generateMipmaps) {
    glGenTextures(1, &m_handle);
    glBindTexture(GL_TEXTURE_2D, m_handle);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, (int)width, (int)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (generateMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
        m_hasMipmaps = true;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    m_width = (int)width; m_height = (int)height;
    return true;
}

void Texture::BindUnit(unsigned unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_handle);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}