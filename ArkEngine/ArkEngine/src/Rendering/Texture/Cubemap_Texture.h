#include <string>
#include <vector>
#include <glad/glad.h>

#include "bitmap.h"
#include "texture.h"

class BaseCubmapTexture : public BaseTexture {
public:
    virtual ~BaseCubmapTexture() = default;

    virtual bool Load() = 0;

    virtual void Bind(GLenum TextureUnit) = 0;
};


class CubemapTexture : public BaseCubmapTexture
{
public:
    CubemapTexture(const std::string& Directory,
        const std::string& PosXFilename,
        const std::string& NegXFilename,
        const std::string& PosYFilename,
        const std::string& NegYFilename,
        const std::string& PosZFilename,
        const std::string& NegZFilename);

    CubemapTexture() {}

    ~CubemapTexture();

    virtual bool Load();

    void LoadKTX(const std::string& Filename);

    virtual void Bind(GLenum TextureUnit);

private:

    std::string m_fileNames[6];
};


// Ect - Equirectangular
class CubemapEctTexture : public BaseCubmapTexture
{
public:

    CubemapEctTexture(const std::string& Filename);

    ~CubemapEctTexture() {};

    virtual bool Load();

    virtual void Bind(GLenum TextureUnit);

private:

    void LoadCubemapData(const std::vector<Bitmap>& Cubemap);

    std::string m_filename;
};