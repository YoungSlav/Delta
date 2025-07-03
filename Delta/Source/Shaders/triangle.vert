#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec4 inColor;


layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outTangent;
layout(location = 2) out vec3 outBitangent;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec2 outTexCoords;
layout(location = 5) out vec4 outColor;

layout(set = 0, binding = 0) uniform Matrices
{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

void main()
{
    outPosition = vec3(ubo.model * vec4(inPosition, 1.0));
	outTexCoords = inTexCoords;

	vec3 T = normalize(vec3(ubo.model * vec4(inTangent, 0.0)));
	vec3 N = normalize(vec3(ubo.model * vec4(inNormal, 0.0)));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	
	outTangent = T;
	outBitangent = B;
	outNormal = N;

	gl_Position = ubo.projection * ubo.view * vec4(outPosition, 1.0);
	outColor = inColor;
}