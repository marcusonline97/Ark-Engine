#version 450

out vec3 WorldPos;

uniform mat4 gVP = mat4(1.0);
uniform float gGridSize = 100.0;

// Origin of the grid plane in world-space. Should be snapped on CPU.
uniform vec3 gGridOrigin = vec3(0.0, 0.0, 0.0);

const vec3 Pos[4] = vec3[4](
    vec3(-1.0, 0.0, -1.0),
    vec3( 1.0, 0.0, -1.0),
    vec3( 1.0, 0.0,  1.0),
    vec3(-1.0, 0.0,  1.0)
);

const int Indices[6] = int[6](0, 2, 1, 2, 0, 3);

void main()
{
    int Index = Indices[gl_VertexID];

    vec3 vPos3 = Pos[Index] * gGridSize;

    // Keep grid locked to world Y=0, but allow (snapped) XZ origin.
    vPos3.x += gGridOrigin.x;
    vPos3.y = 0.0;
    vPos3.z += gGridOrigin.z;

    vec4 vPos4 = vec4(vPos3, 1.0);

    gl_Position = gVP * vPos4;
    WorldPos = vPos3;
}