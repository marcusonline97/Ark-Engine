#version 330 core

#define MAX_LIGHTS 8

struct Light
{
    vec3  color;
    vec3  position;
    vec3  direction;
    float intensity;
    float range;
    int   type;       // 0 = directional, 1 = point
};

uniform Light  uLights[MAX_LIGHTS];
uniform int    uLightCount;
uniform vec3   uCameraPos;

out vec4 FragColor;

in vec2 vUV;
in vec3 vNormal;
in vec3 vFragPos;

uniform sampler2D baseColorTexture;
uniform sampler2D specularMap;
uniform int       uHasSpecularMap;
uniform float     uSpecularStrength;

void main()
{
    vec3 norm    = normalize(vNormal);
    vec3 viewDir = normalize(uCameraPos - vFragPos);

    vec4 texColor = texture(baseColorTexture, vUV);

    const float ambientStrength = 0.15;
    vec3 result = vec3(0.0);

    for (int i = 0; i < uLightCount; ++i)
    {
        vec3  lightDir;
        float attenuation = 1.0;
        vec3  radiance    = uLights[i].color * uLights[i].intensity;

        if (uLights[i].type == 1)
        {
            // Point light
            vec3  toLight = uLights[i].position - vFragPos;
            float dist    = length(toLight);
            if (dist > uLights[i].range) continue;
            lightDir    = normalize(toLight);
            attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);
        }
        else
        {
            // Directional light
            lightDir = normalize(-uLights[i].direction);
        }

        float diff    = max(dot(norm, lightDir), 0.0);
        vec3  diffuse = diff * radiance;

        vec3  reflectDir = reflect(-lightDir, norm);
        float spec       = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        float specStr    = uSpecularStrength;
        if (uHasSpecularMap != 0)
            specStr *= texture(specularMap, vUV).r;
        vec3 specular = specStr * spec * radiance;

        result += (diffuse + specular) * attenuation;
    }

    vec3 ambientColor = (uLightCount > 0) ? uLights[0].color : vec3(1.0);
    result += ambientStrength * ambientColor;

    FragColor = texColor * vec4(result, 1.0);
}