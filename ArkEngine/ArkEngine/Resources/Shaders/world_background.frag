#version 450 core

layout(location = 0) out vec4 FragColor;

uniform vec3 uTopColor;
uniform vec3 uHorizonColor;
uniform vec3 uBottomColor;
uniform float uViewportHeight;

void main()
{
    float t = clamp(gl_FragCoord.y / max(uViewportHeight, 1.0), 0.0, 1.0);
    vec3 color = (t < 0.5)
        ? mix(uBottomColor, uHorizonColor, t * 2.0)
        : mix(uHorizonColor, uTopColor, (t - 0.5) * 2.0);

    FragColor = vec4(color, 1.0);
}
