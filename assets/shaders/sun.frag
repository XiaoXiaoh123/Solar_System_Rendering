#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D uDiffuseMap;
uniform int uHasTexture;
uniform int uDebugMode;
uniform vec3 uLightColor = vec3(1.0, 0.95, 0.8);

void main() {
    vec3 baseColor;
    if (uHasTexture == 1) {
        baseColor = texture(uDiffuseMap, TexCoord).rgb;
    } else {
        float fresnel = 1.0 - abs(normalize(Normal).y);
        fresnel = pow(fresnel, 3.0);
        baseColor = mix(vec3(1.0, 0.88, 0.3), vec3(1.0, 0.97, 0.8), fresnel);
    }

    if (uDebugMode == 1) {
        FragColor = vec4(baseColor, 1.0);
        return;
    }
    if (uDebugMode == 2) {
        float seam = fract(TexCoord.x * 64.0) < 0.5 ? 0.3 : 1.0;
        FragColor = vec4(TexCoord.x, TexCoord.y, seam, 1.0);
        return;
    }
    if (uDebugMode == 3) {
        vec3 n = abs(normalize(Normal));
        FragColor = vec4(n, 1.0);
        return;
    }

    vec3 result = baseColor * uLightColor * 3.2;
    FragColor = vec4(result, 1.0);
}
