#version 460

layout(location = 0) out vec4 FragColor;

in vec2 vUV;

void main()
{
    // fract() makes tiling visible when UVs are outside [0..1].
    vec2 uv = fract(vUV);

    // R = U, G = V
    FragColor = vec4(uv.x, uv.y, 0.0, 1.0);
}