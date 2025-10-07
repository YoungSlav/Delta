#version 450
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;

// set=1 bindings must match your lightingSetLayout
layout(set=1, binding=0) uniform sampler2D gAlbedo;
layout(set=1, binding=1) uniform sampler2D gNormal;

void main() {
vec3 albedo = texture(gAlbedo, vUV).rgb;
vec3 normalV = normalize(texture(gNormal, vUV).xyz);
vec3 L = normalize(vec3(0.4, 0.7, 0.2));
float NdotL = max(dot(normalV, L), 0.0);
outColor = vec4(albedo * NdotL, 1.0);
}