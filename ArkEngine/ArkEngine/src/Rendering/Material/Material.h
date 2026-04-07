#pragma once

#include "Math/3DMath_util.h"
#include "Rendering/Texture/Texture.h"

struct PBRMaterial
{
    float Roughness = 0.0f;
    bool IsMetal = false;
    Vector3f Color = Vector3f(0.0f, 0.0f, 0.0f);

    // Aliases into Material::pTextures[] (non-owning).
    Texture* pAlbedo = nullptr;
    Texture* pRoughness = nullptr;
    Texture* pMetallic = nullptr;
    Texture* pNormalMap = nullptr;
    Texture* pAO = nullptr;
    Texture* pEmissive = nullptr;
};

enum TEXTURE_TYPE {
    TEX_TYPE_BASE = 0,      // Base color / diffuse / albedo
    TEX_TYPE_SPECULAR = 1,
    TEX_TYPE_NORMAL = 2,
    TEX_TYPE_METALNESS = 3,
    TEX_TYPE_EMISSIVE = 4,
    TEX_TYPE_NORMAL_CAMERA = 5,
    TEX_TYPE_EMISSION_COLOR = 6,
    TEX_TYPE_ROUGHNESS = 7,
    TEX_TYPE_AMBIENT_OCCLUSION = 8,
    TEX_TYPE_CLEARCOAT = 9,
    TEX_TYPE_CLEARCOAT_ROUGHNESS = 10,
    TEX_TYPE_CLEARCOAT_NORMAL = 11,
    TEX_TYPE_NUM = 12
};

class Material {

public:

    std::string m_name;

    Vector4f AmbientColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f DiffuseColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f SpecularColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f BaseColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f EmissiveColor = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    Vector4f MetallicRoughnessNormalOcclusion = Vector4f(1.0f);
    Vector4f ClearCoatTransmissionThickness = Vector4f(1.0f);

    // Owns all textures.
    Texture* pTextures[TEX_TYPE_NUM] = { 0 };

    // Non-owning aliases into pTextures[].
    PBRMaterial PBRmaterial;

    float m_transparencyFactor = 1.0f;
    float m_alphaTest = 0.0f;
    u32 m_flags = 0;

    void SyncPBRTextureAliases()
    {
        PBRmaterial.pAlbedo = pTextures[TEX_TYPE_BASE];
        PBRmaterial.pRoughness = pTextures[TEX_TYPE_ROUGHNESS];
        PBRmaterial.pMetallic = pTextures[TEX_TYPE_METALNESS];
        PBRmaterial.pNormalMap = pTextures[TEX_TYPE_NORMAL];
        PBRmaterial.pAO = pTextures[TEX_TYPE_AMBIENT_OCCLUSION];
        PBRmaterial.pEmissive = pTextures[TEX_TYPE_EMISSIVE];
    }

    ~Material()
    {
        for (Texture*& pTex : pTextures) {
            delete pTex;
            pTex = nullptr;
        }

        // Avoid dangling aliases (debug safety).
        SyncPBRTextureAliases();
    }
};