#include "Rendering/Lightning/Lightning.h"
#include "Maps/Techniques/Skinning_Technique.h"
#include "ECS/Common/Basic_Mesh.h"
#include "ECS/SkinnedMesh/Skinned_Mesh.h"
#include "Maps/Techniques/Shadow_Mapping_Technique.h"


class PhongRenderer {
public:

    PhongRenderer();

    ~PhongRenderer();

    void InitPhongRenderer(int SubTech = LightingTechnique::SUBTECH_DEFAULT);

    void StartShadowPass();

    void SetCamera(const Vector3f& cameraPos, const Matrix4f& view, const Matrix4f& projection)
    {
        m_cameraPos = cameraPos;
        m_viewMatrix = view;
        m_projectionMatrix = projection;
        m_hasCamera = true;
    }

    void SetViewportMatrix(const Matrix4f& viewport)
    {
        m_viewportMatrix = viewport;
        m_hasViewportMatrix = true;
    }

    void SetPBR(bool IsPBR);

    //
    // Lighting
    //
    void SetDirLight(const DirectionalLight& DirLight);

    void SetPointLights(uint NumLights, const PointLight* pPointLights);

    void SetSpotLights(uint NumLights, const SpotLight* pSpotLights);

    void UpdateDirLightDir(const Vector3f& WorldDir);

    void UpdatePointLightPos(uint Index, const Vector3f& WorldPos);

    void UpdateSpotLightPosAndDir(uint Index, const Vector3f& WorldPos, const Vector3f& WorldDir);

    //
    // Fog
    //
    void SetLinearFog(float FogStart, float FogEnd, const Vector3f& FogColor);

    void SetExpFog(float FogEnd, const Vector3f& FogColor, float FogDensity);

    void SetExpSquaredFog(float FogEnd, const Vector3f& FogColor, float FogDensity);

    void SetLayeredFog(float FogTop, float FogEnd, const Vector3f& FogColor);

    void SetAnimatedFog(float FogEnd, float FogDensity, const Vector3f& FogColor);

    void UpdateAnimatedFogTime(float FogTime);

    void DisableFog();

    void ControlRimLight(bool IsEnabled);

    void ControlCellShading(bool IsEnabled);

    void SetWireframeLineWidth(float Width);
    void SetWireframeColor(const Vector4f& Color);

    void Render(BasicMesh* pMesh);

    void RenderAnimation(SkinnedMesh* pMesh, float AnimationTimeSec, int AnimationIndex = 0);

    void RenderAnimationBlended(SkinnedMesh* pMesh,
        float AnimationTimeSec,
        int StartAnimIndex,
        int EndAnimIndex,
        float BlendFactor);

    void RenderToShadowMap(BasicMesh* pMesh, const SpotLight& SpotLight);
private:

    void GetWVP(BasicMesh* pMesh, Matrix4f& WVP);

    void SwitchToLightingTech();
    void SwitchToSkinningTech();

    void RefreshLightingPosAndDirs(BasicMesh* pMesh);

    void RenderAnimationCommon(SkinnedMesh* pMesh);

    Vector3f m_cameraPos = Vector3f(0.0f, 0.0f, 0.0f);
    Matrix4f m_viewMatrix;
    Matrix4f m_projectionMatrix;
    Matrix4f m_viewportMatrix;
    bool m_hasCamera = false;
    bool m_hasViewportMatrix = false;
    int m_subTech = LightingTechnique::SUBTECH_DEFAULT;
    LightingTechnique m_lightingTech;
    SkinningTechnique m_skinningTech;
    ShadowMappingTechnique m_shadowMapTech;

    // Lighting info
    DirectionalLight m_dirLight;
    uint m_numPointLights = 0;
    PointLight m_pointLights[LightingTechnique::MAX_POINT_LIGHTS];
    uint m_numSpotLights = 0;
    SpotLight m_spotLights[LightingTechnique::MAX_SPOT_LIGHTS];
    bool m_isPBR = false;
    GLuint m_activeProgram = 0;
};