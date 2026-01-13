#version 460 core

in vec3 vColor;
out vec4 FragColor;

uniform vec3 u_Tint;

void main()
{
    FragColor = vec4(vColor * u_Tint, 1.0);
}