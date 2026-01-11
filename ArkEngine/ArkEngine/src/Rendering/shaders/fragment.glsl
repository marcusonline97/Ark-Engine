#version 450 core

in vec3 vColor;
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uUseTexture; // 1.0 = use texture, 0.0 = use vertex color
uniform vec3 uTint;        // tint multiplier

void main()
{
    vec4 texColor = texture(uTexture, vTexCoord);
    vec4 vertColor = vec4(vColor, 1.0);
    vec4 base = mix(vertColor, texColor, uUseTexture);
    FragColor = vec4(base.rgb * uTint, base.a);
}