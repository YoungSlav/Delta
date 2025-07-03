#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inTangent;
layout(location = 2) in vec3 inBitangent;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec2 inTexCoords;
layout(location = 5) in vec4 inVertColor;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = inVertColor;
}