#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D uDiffuseMap;
uniform int      uHasTexture;
uniform vec3 uLightColor = vec3(1.0, 0.95, 0.8);

void main() {
    vec3 baseColor;
    if (uHasTexture == 1) {
        baseColor = texture(uDiffuseMap, TexCoord).rgb;
    } else {
        // View-independent color variation based on surface normal
        float fresnel = 1.0 - abs(normalize(Normal).y);
        fresnel = pow(fresnel, 3.0);
        baseColor = mix(vec3(1.0, 0.88, 0.3), vec3(1.0, 0.97, 0.8), fresnel);
    }

    // Sun is self-illuminating — no external light needed
    vec3 result = baseColor * uLightColor * 3.2;

    FragColor = vec4(result, 1.0);
}
