#pragma once
#include <string>

class Texture
{
public:
    explicit Texture(const std::string& path, bool srgb = true);
    ~Texture();

    unsigned int GetId() const { return m_Id; }
    int Width() const { return m_Width; }
    int Height() const { return m_Height; }
    int Channels() const { return m_Channels; }

private:
    unsigned int m_Id = 0;
    int m_Width = 0, m_Height = 0, m_Channels = 0;
};