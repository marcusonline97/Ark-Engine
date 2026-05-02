#include "Ark_camera_api.h"
#include "Ark_new_lighting.h"
#include "Ark_skinning_technique.h"
#include "Ark_basic_mesh.h"
#include "Ark_skinned_mesh.h"
#include "Ark_shadow_mapping_technique.h"


class PhongRenderer {
public:

    PhongRenderer();

    ~PhongRenderer();

    void InitPhongRenderer(int SubTech = LightingTechnique::SUBTECH_DEFAULT);

    void StartShadowPass();

    void SetCamera(const CameraAPI* pCamera) { m_pCamera = pCamera; }

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

    const CameraAPI* m_pCamera = NULL;
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
};