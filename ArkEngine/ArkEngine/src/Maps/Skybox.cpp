#include "Skybox.h"
#include "Utility/Util.h"
#include "ECS/Common/Basic_Mesh.h"

SkyBox::SkyBox()
{
    m_pSkyboxTechnique = NULL;
    m_pCubemapTex = NULL;
    m_pMesh = NULL;
}


SkyBox::~SkyBox()
{
    SAFE_DELETE(m_pSkyboxTechnique);
    SAFE_DELETE(m_pCubemapTex);
    SAFE_DELETE(m_pMesh);
}


void SkyBox::Init(const string& Directory,
    const string& PosXFilename,
    const string& NegXFilename,
    const string& PosYFilename,
    const string& NegYFilename,
    const string& PosZFilename,
    const string& NegZFilename)
{
    InitTechnique();

    m_pCubemapTex = new CubemapTexture(Directory,
        PosXFilename,
        NegXFilename,
        PosYFilename,
        NegYFilename,
        PosZFilename,
        NegZFilename);

    LoadTextureAndMesh();
}


void SkyBox::Init(const std::string& EctTextureFilename)
{
    InitTechnique();

    m_pCubemapTex = new CubemapEctTexture(EctTextureFilename);

    LoadTextureAndMesh();
}


void SkyBox::InitTechnique()
{
    m_pSkyboxTechnique = new SkyboxTechnique();

    if (!m_pSkyboxTechnique->Init()) {
        printf("Error initializing the skybox technique\n");
        exit(1);
    }

    m_pSkyboxTechnique->Enable();
    m_pSkyboxTechnique->SetTextureUnit(0);
}


void SkyBox::LoadTextureAndMesh()
{
    if(!m_pCubemapTex->Load()) {
        printf("Error loading the skybox cubemap texture\n");
        return;
	}

    m_pMesh = new BasicMesh();

    m_pMesh->LoadMesh("../Content/box.obj");
}

void SkyBox::Render(const Matrix4f& VP)
{
    m_pSkyboxTechnique->Enable();

    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);

    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

    glCullFace(GL_FRONT);

    glDepthFunc(GL_LEQUAL);

    static float r = 0.0f;
    Matrix4f Rotation;
    Rotation.InitRotateTransform(0.0f, r, 0.0f);
    // r += 0.01f;

    Matrix4f WVP = VP * Rotation;
    m_pSkyboxTechnique->SetWVP(WVP);
    m_pCubemapTex->Bind(GL_TEXTURE0);
    m_pMesh->Render();

    glCullFace(OldCullFaceMode);

    glDepthFunc(OldDepthFuncMode);
}