#include "Camera/BasicCamera.h"
#include "Maps/Techniques/Skybox_Technique.h"
#include "Rendering/Texture/Cubemap_Texture.h"
#include <memory>

class BasicMesh;

class SkyBox
{
public:
    SkyBox();

    ~SkyBox();

    void Init(const std::string& Directory,
        const std::string& PosXFilename,
        const std::string& NegXFilename,
        const std::string& PosYFilename,
        const std::string& NegYFilename,
        const std::string& PosZFilename,
        const std::string& NegZFilename);

    void Init(const std::string& EctTextureFilename);

    void Render(const BasicCamera& pCamera);

    void Render(const Matrix4f& VP);

private:
    void InitTechnique();
    void LoadTextureAndMesh();

    std::unique_ptr<SkyboxTechnique> m_pSkyboxTechnique;
    std::unique_ptr<BaseCubmapTexture> m_pCubemapTex;
    std::unique_ptr<BasicMesh> m_pMesh;
};