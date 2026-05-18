#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D uDiffuseMap;
uniform int      uHasTexture;
uniform vec3 uLightColor = vec3(1.0, 0.95, 0.8);
uniform vec3 uViewPos;

void main() {
    vec3 baseColor;
    if (uHasTexture == 1) {
        baseColor = texture(uDiffuseMap, TexCoord).rgb;
    } else {
        // Generate a warm yellow-orange gradient using noise-like patterns
        float fresnel = 1.0 - abs(dot(normalize(Normal), normalize(uViewPos - FragPos)));
        fresnel = pow(fresnel, 3.0);
        baseColor = mix(vec3(1.0, 0.85, 0.2), vec3(1.0, 0.95, 0.7), fresnel);
    }

    // Sun is self-illuminating — no external light needed
    vec3 result = baseColor * uLightColor * 1.5;

    FragColor = vec4(result, 1.0);
}
