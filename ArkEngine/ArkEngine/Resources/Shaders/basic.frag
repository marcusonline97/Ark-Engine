#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

uniform sampler2D uDiffuseTexture;   // Texture unit 0 (GL_TEXTURE0)
uniform sampler2D uSpecularTexture;  // Texture unit 1 (GL_TEXTURE1)
uniform float uShininess;
uniform vec3 uColor;  // Fallback color if no texture

out vec4 FragColor;

void main() {
    // Sample diffuse texture
    vec4 diffuseColor = texture(uDiffuseTexture, UV);
    
    // If texture alpha is very low, use fallback color
    if (diffuseColor.a < 0.01) {
        diffuseColor = vec4(uColor, 1.0);
    }
    else if (length(diffuseColor.rgb) < 0.01) {
        // If texture is essentially black, use fallback color
        diffuseColor = vec4(uColor, 1.0);
    }
    
    // Sample specular texture (for now, just use the intensity)
    vec3 specularColor = texture(uSpecularTexture, UV).rgb;
    
    // Simple output: diffuse color modulated by specular
    // Full Phong lighting will be added in Phase 4
    vec3 finalColor = diffuseColor.rgb * (1.0 + specularColor * uShininess * 0.1);
    
    FragColor = vec4(finalColor, diffuseColor.a);
}
