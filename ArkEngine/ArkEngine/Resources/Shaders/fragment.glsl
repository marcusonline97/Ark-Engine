#version 460 core

in vec3 vColor;
in vec2 vUV;
out vec4 FragColor;

uniform vec3 u_Tint;
uniform int uUseTexture;
uniform sampler2D uTexture;

void main()
{
    vec3 base = vColor * u_Tint;
    if (uUseTexture != 0)
    {
        base *= texture(uTexture, vUV).rgb;
    }
    FragColor = vec4(base, 1.0);
}