#version 450
layout(location=0) in vec3 vNormalV;
layout(location=1) in vec2 vUV;

layout(location=0) out vec4 outG0; // albedo.rgb, metallic
layout(location=1) out vec4 outG1; // normal.xyz (view), roughness

layout(set=1, binding=0) uniform sampler2D uAlbedo;

void main()
{
    vec3 albedo = texture(uAlbedo, vUV).rgb;
    outG0 = vec4(albedo, 0.0); // metallic placeholder
    outG1 = vec4(normalize(vNormalV), 1.0); // roughness placeholder
}