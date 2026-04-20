#version 450 core

out vec2 vNdc;

void main()
{
    vec2 clipPos = vec2(
        (gl_VertexID == 1) ? 3.0 : -1.0,
        (gl_VertexID == 2) ? 3.0 : -1.0);

    vNdc = clipPos;
    gl_Position = vec4(clipPos, 0.0, 1.0);
}
