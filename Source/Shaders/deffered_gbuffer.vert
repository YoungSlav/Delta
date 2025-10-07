#version 450

layout(push_constant) uniform PushConstants
{
	mat4 model;
} pc;

layout(std140, set=0, binding=0) uniform Camera
{
    mat4 view;
    mat4 proj;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec4 inColor;

layout(location=0) out vec3 vNormal;
layout(location=1) out vec2 vUV;

void main()
{
    mat3 nm = mat3(camera.view) * mat3(pc.model);
    
    vNormal = normalize(nm * inNormal);
    vUV = inTexCoords;

    gl_Position = camera.proj * camera.view * pc.model * vec4(inPosition, 1.0);
}