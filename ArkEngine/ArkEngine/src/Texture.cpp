#include "stb_image.h"
#include <glad/glad.h>
#include "Texture.h"
#include <stdexcept>

Texture::Texture(const std::string& path, bool srgb)
{
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 0);
    if (!data) throw std::runtime_error("Failed to load texture: " + path);

    GLenum format = (m_Channels == 1) ? GL_RED : (m_Channels == 3) ? GL_RGB : GL_RGBA;
    GLenum internal = format;
    if (srgb && (format == GL_RGB)) internal = GL_SRGB8;
    if (srgb && (format == GL_RGBA)) internal = GL_SRGB8_ALPHA8;

    glGenTextures(1, &m_Id);
    glBindTexture(GL_TEXTURE_2D, m_Id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, internal, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
}

Texture::~Texture()
{
    if (m_Id) glDeleteTextures(1, &m_Id);
}