#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;

out vec3 vWorldPos;
out vec3 vWorldNormal;
out vec2 vUV;

void main()
{
    vWorldPos = vec3(uModel * vec4(aPos, 1.0));
    vWorldNormal = normalize(mat3(transpose(inverse(uModel))) * aNormal);
    vUV = aUV;

    gl_Position = uMVP * vec4(aPos, 1.0);
}