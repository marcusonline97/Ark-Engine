#version 460 core

in vec3 vWorldPos;
in vec3 vWorldNormal;

out vec4 FragColor;

uniform vec3 u_Tint;
uniform vec3 uCameraPos;

#define MAX_POINT_LIGHTS 16

uniform int uPointLightCount;
uniform vec3 uPointLightPos[MAX_POINT_LIGHTS];
uniform vec3 uPointLightColor[MAX_POINT_LIGHTS];
uniform float uPointLightIntensity[MAX_POINT_LIGHTS];
uniform float uPointLightRadius[MAX_POINT_LIGHTS];

void main()
{
    vec3 N = normalize(vWorldNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);

    vec3 ambient = 0.08 * u_Tint;

    vec3 color = ambient;

    for (int i = 0; i < uPointLightCount; ++i)
    {
        vec3 Lvec = uPointLightPos[i] - vWorldPos;
        float dist = length(Lvec);

        float r = max(uPointLightRadius[i], 0.0001);
        float atten = clamp(1.0 - (dist / r), 0.0, 1.0);
        atten = atten * atten;

        vec3 L = (dist > 0.00001) ? (Lvec / dist) : vec3(0.0, 1.0, 0.0);

        float NdotL = max(dot(N, L), 0.0);

        // Simple Blinn-Phong spec
        vec3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), 32.0);

        vec3 lightCol = uPointLightColor[i] * uPointLightIntensity[i];

        vec3 diff = u_Tint * NdotL;
        vec3 specCol = spec * vec3(0.25);

        color += (diff + specCol) * lightCol * atten;
    }

    FragColor = vec4(color, 1.0);
}