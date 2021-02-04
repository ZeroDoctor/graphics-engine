#version 450
#extension GL_ARB_separate_shader_objects : enable

// layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
// layout(location = 1) in vec2 fragTex;

layout(location = 0) out vec4 outColor;

void main() {
    // vec4 texel = texture(texSampler, fragTex);
    // outColor.rgb = texel.rgb * fragColor.rbg * fragColor.a * texel.a;
    // outColor.a = texel.a * fragColor.a;
    // outColor = vec4(fragColor * texture(texSampler, fragTex).rgb, 1.0);
    outColor = vec4(fragColor, 1.0) * 1.5;
}