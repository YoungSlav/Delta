#version 450
layout(location=0) out vec2 vUV;

void main()
{
    // 3 vertices: (0,0), (2,0), (0,2) cover the screen
    vec2 v = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vUV = v;
    gl_Position = vec4(v * 2.0 - 1.0, 0.0, 1.0);
}