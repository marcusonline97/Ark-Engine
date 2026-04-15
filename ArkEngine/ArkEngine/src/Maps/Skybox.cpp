#include "Skybox.h"
#include "Common/Pipeline.h"
#include "Utility/Util.h"
#include "ECS/Common/Basic_Mesh.h"
#include <memory>

namespace
{
    struct SkyboxStateGuard
    {
        SkyboxStateGuard()
        {
            glCullFace(GL_FRONT);
            glDepthFunc(GL_LEQUAL);
        }

        ~SkyboxStateGuard()
        {
            glCullFace(GL_BACK);
            glDepthFunc(GL_LESS);
        }
    };
}

SkyBox::SkyBox()
{
}


SkyBox::~SkyBox()
{
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

    m_pCubemapTex = std::make_unique<CubemapTexture>(Directory,
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

    m_pCubemapTex = std::make_unique<CubemapEctTexture>(EctTextureFilename);

    LoadTextureAndMesh();
}


void SkyBox::InitTechnique()
{
    m_pSkyboxTechnique = std::make_unique<SkyboxTechnique>();

    if (!m_pSkyboxTechnique->Init()) {
        printf("Error initializing the skybox technique\n");
        exit(1);
    }

    m_pSkyboxTechnique->Enable();
    m_pSkyboxTechnique->SetTextureUnit(0);
}


void SkyBox::LoadTextureAndMesh()
{
    if (!m_pCubemapTex || !m_pCubemapTex->Load()) {
        printf("Error loading the skybox cubemap texture\n");
        return;
	}

    auto mesh = std::make_unique<BasicMesh>();
    mesh->LoadMesh("../Content/box.obj");
    m_pMesh = std::move(mesh);
}


void SkyBox::Render(const BasicCamera& Camera)
{
    if (!m_pSkyboxTechnique || !m_pCubemapTex || !m_pMesh)
        return;

    m_pSkyboxTechnique->Enable();
    SkyboxStateGuard stateGuard;

    static float r = 0.0f;
    Matrix4f Rotation;
    Rotation.InitRotateTransform(0.0f, r, 0.0f);
    // r += 0.01f;

    Matrix4f View;
    View.InitCameraTransform(Vector3f(0.0f, 0.0f, 0.0f), Camera.GetTarget(), Camera.GetUp());
    Matrix4f Proj;
    Proj.InitPersProjTransform(Camera.GetPersProjInfo());
    Matrix4f WVP = Proj * View * Rotation;
    m_pSkyboxTechnique->SetWVP(WVP);
    m_pCubemapTex->Bind(GL_TEXTURE0);
    m_pMesh->Render();
}


void SkyBox::Render(const Matrix4f& VP)
{
    if (!m_pSkyboxTechnique || !m_pCubemapTex || !m_pMesh)
        return;

    m_pSkyboxTechnique->Enable();
    SkyboxStateGuard stateGuard;

    static float r = 0.0f;
    Matrix4f Rotation;
    Rotation.InitRotateTransform(0.0f, r, 0.0f);
    // r += 0.01f;

    Matrix4f WVP = VP * Rotation;
    m_pSkyboxTechnique->SetWVP(WVP);
    m_pCubemapTex->Bind(GL_TEXTURE0);
    m_pMesh->Render();
}