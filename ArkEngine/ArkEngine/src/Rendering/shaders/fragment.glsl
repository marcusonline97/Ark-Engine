#version 460 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uAlbedo;     // texture unit 0
uniform sampler2D uSpecular;   // texture unit 1
uniform vec3 uColor;           // fallback color if albedo missing
uniform float uShininess;      // specular power
uniform vec3 uCameraPos;

struct DirLight {
    vec3 direction;
    vec3 color;
    float intensity;
};
uniform DirLight uLight;

void main() {
    vec3 albedoTex = texture(uAlbedo, vUV).rgb;
    bool albedoIsBlack = length(albedoTex) < 0.01;
    vec3 albedo = albedoIsBlack ? uColor : albedoTex;

    vec3 specTex = texture(uSpecular, vUV).rgb;
    float specStrength = max(max(specTex.r, specTex.g), specTex.b);

    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLight.direction);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    vec3 diffuse = albedo * NdotL * uLight.intensity * uLight.color;

    float spec = pow(NdotH, uShininess) * specStrength;
    vec3 specular = spec * uLight.color * uLight.intensity;

    vec3 ambient = 0.05 * albedo;

    vec3 color = ambient + diffuse + specular;
    FragColor = vec4(color, 1.0);
}