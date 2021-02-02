#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 0) in vec2 fragTex;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 texel = texture(texSampler, fragTex);
    outColor.rgb = texel.rgb * fragColor.rbg * fragColor.a * texel.a;
    outColor.a = texel.a * fragColor.a;
}