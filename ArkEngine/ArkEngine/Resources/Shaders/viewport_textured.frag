#version 460 core

in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec2 vUV;

out vec4 FragColor;

uniform vec3 u_Tint;
uniform vec3 uCameraPos;

uniform float uAmbientStrength;
uniform float uExposure;

uniform int uMaterialPreset;

uniform bool uHasAlbedoTexture;
layout(binding = 0) uniform sampler2D uAlbedoTexture;

#define MAX_POINT_LIGHTS 16
uniform int uPointLightCount;
uniform vec3 uPointLightPos[MAX_POINT_LIGHTS];
uniform vec3 uPointLightColor[MAX_POINT_LIGHTS];
uniform float uPointLightIntensity[MAX_POINT_LIGHTS];
uniform float uPointLightRadius[MAX_POINT_LIGHTS];

vec3 TonemapReinhard(vec3 c)
{
    return c / (c + vec3(1.0));
}

vec3 GammaCorrect(vec3 c)
{
    return pow(max(c, vec3(0.0)), vec3(1.0 / 2.2));
}

void main()
{
    vec3 N = normalize(vWorldNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);

    vec3 baseColor = u_Tint;
    if (uHasAlbedoTexture)
        baseColor *= texture(uAlbedoTexture, vUV).rgb;

    // Preset tuning (cheap + visible)
    float specPow = 32.0;
    float specStrength = 0.25;
    float emissiveBoost = 0.0;

    // 0 Default, 1 Matte, 2 Glossy, 3 Metal, 4 Emissive
    if (uMaterialPreset == 1) { specPow = 8.0;  specStrength = 0.05; }
    if (uMaterialPreset == 2) { specPow = 96.0; specStrength = 0.40; }
    if (uMaterialPreset == 3) { specPow = 160.0; specStrength = 0.65; }
    if (uMaterialPreset == 4) { specPow = 16.0; specStrength = 0.10; emissiveBoost = 1.25; }

    vec3 ambient = uAmbientStrength * baseColor;
    vec3 color = ambient + baseColor * emissiveBoost;

    for (int i = 0; i < uPointLightCount; ++i)
    {
        vec3 Lvec = uPointLightPos[i] - vWorldPos;
        float dist = length(Lvec);

        float r = max(uPointLightRadius[i], 0.0001);
        float atten = clamp(1.0 - (dist / r), 0.0, 1.0);
        atten = atten * atten;

        vec3 L = (dist > 0.00001) ? (Lvec / dist) : vec3(0.0, 1.0, 0.0);

        float NdotL = max(dot(N, L), 0.0);

        vec3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), specPow);

        vec3 lightCol = uPointLightColor[i] * uPointLightIntensity[i];

        vec3 diff = baseColor * NdotL;
        vec3 specCol = spec * vec3(specStrength);

        color += (diff + specCol) * lightCol * atten;
    }

    color *= uExposure;
    color = TonemapReinhard(color);
    color = GammaCorrect(color);

    FragColor = vec4(color, 1.0);
}