#include "Camera/BasicCamera.h"
#include "Maps/Techniques/Skybox_Technique.h"
#include "Rendering/Texture/Cubemap_Texture.h"

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

    SkyboxTechnique* m_pSkyboxTechnique = NULL;
    BaseCubmapTexture* m_pCubemapTex = NULL;
    BasicMesh* m_pMesh = NULL;
};