#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

// Material properties
uniform sampler2D uDiffuseTexture;   // Texture unit 0
uniform sampler2D uSpecularTexture;  // Texture unit 1
uniform float uShininess;
uniform vec3 uColor;  // Fallback color if no texture

// Camera
uniform vec3 uViewPos;

// Light structure (matches GPULightData C++ struct)
// Note: Using vec4 for proper std140 alignment
struct Light {
    vec4 position;      // xyz = position, w = light type (0.0=Point, 1.0=Directional, 2.0=Spot)
    vec4 direction;     // xyz = direction (normalized), w = intensity
    vec4 color;         // rgb = color, w = constant attenuation
    vec4 attenuation;   // x = linear, y = quadratic, z = cutOff (cos), w = outerCutOff (cos)
};

// Maximum number of lights
#define MAX_LIGHTS 32

uniform Light uLights[MAX_LIGHTS];
uniform int uLightCount;

// Calculate point light contribution
vec3 CalculatePointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * diffuseColor * light.direction.w;
    
    // Specular (Phong)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    vec3 specular = spec * light.color.rgb * specularColor * light.direction.w;
    
    // Attenuation
    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.color.w + light.attenuation.x * distance + light.attenuation.y * (distance * distance));
    
    return (diffuse + specular) * attenuation;
}

// Calculate directional light contribution
vec3 CalculateDirectionalLight(Light light, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
    vec3 lightDir = normalize(-light.direction.xyz);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * diffuseColor * light.direction.w;
    
    // Specular (Phong)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    vec3 specular = spec * light.color.rgb * specularColor * light.direction.w;
    
    return diffuse + specular;
}

// Calculate spot light contribution
vec3 CalculateSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.attenuation.z - light.attenuation.w;
    float intensity = clamp((theta - light.attenuation.w) / epsilon, 0.0, 1.0);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * diffuseColor * light.direction.w * intensity;
    
    // Specular (Phong)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    vec3 specular = spec * light.color.rgb * specularColor * light.direction.w * intensity;
    
    // Attenuation
    float distance = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.color.w + light.attenuation.x * distance + light.attenuation.y * (distance * distance));
    
    return (diffuse + specular) * attenuation;
}

void main() {
    // Sample textures (if texture is not bound, this will return (0,0,0,1))
    vec4 diffuseTex = texture(uDiffuseTexture, UV);
    vec4 specularTex = texture(uSpecularTexture, UV);
    
    // Use fallback color if texture appears to be default/unbound (black or very dark)
    // Check if texture is essentially black (default unbound texture)
    bool isDefaultDiffuse = (length(diffuseTex.rgb) < 0.01);
    bool isDefaultSpecular = (length(specularTex.rgb) < 0.01);
    
    vec3 diffuseColor = isDefaultDiffuse ? uColor : diffuseTex.rgb;
    vec3 specularColor = isDefaultSpecular ? vec3(0.2) : specularTex.rgb; // Default specular if no texture
    
    // Normalize normal (might be needed due to interpolation)
    vec3 norm = normalize(Normal);
    
    // View direction (from fragment to camera)
    vec3 viewDir = normalize(uViewPos - FragPos);
    
    // Ambient lighting (small constant)
    vec3 ambient = 0.1 * diffuseColor;
    
    // Accumulate light contributions
    vec3 result = ambient;
    
    // Loop through active lights
    int lightCount = min(uLightCount, MAX_LIGHTS);
    for (int i = 0; i < lightCount; i++) {
        float lightType = uLights[i].position.w;
        
        if (abs(lightType - 0.0) < 0.1) {
            // Point light (type = 0.0)
            result += CalculatePointLight(uLights[i], norm, FragPos, viewDir, diffuseColor, specularColor);
        }
        else if (abs(lightType - 1.0) < 0.1) {
            // Directional light (type = 1.0)
            result += CalculateDirectionalLight(uLights[i], norm, viewDir, diffuseColor, specularColor);
        }
        else if (abs(lightType - 2.0) < 0.1) {
            // Spot light (type = 2.0)
            result += CalculateSpotLight(uLights[i], norm, FragPos, viewDir, diffuseColor, specularColor);
        }
    }
    
    // Tone mapping and gamma correction (simple version)
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0 / 2.2));
    
    FragColor = vec4(result, diffuseTex.a);
}
