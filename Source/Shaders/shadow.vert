#version 450
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec4 inColor;

layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

layout(std140, set=0, binding=0) uniform Camera {
    mat4 view;
    mat4 proj;
} light;

layout(location = 0) in vec3 inPosition;

void main()
{
    gl_Position = light.proj * light.view * pc.model * vec4(inPosition, 1.0);
}
