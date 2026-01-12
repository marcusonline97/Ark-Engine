#version 460 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;   // bound to unit 0
uniform int uUseTexture;
uniform vec3 uTint;

void main() {
    vec3 base = uUseTexture == 1 ? texture(uTexture, vUV).rgb : vec3(1.0);
    FragColor = vec4(base * uTint, 1.0);
}