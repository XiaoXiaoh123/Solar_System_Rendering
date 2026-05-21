#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D uDiffuseMap;
uniform int      uHasTexture;
uniform int      uDebugMode;   // 0=normal lit, 1=raw texture, 2=UV visualization, 3=normal viz

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform float uAmbientStrength;

void main() {
    vec3 baseColor;
    if (uHasTexture == 1) {
        baseColor = texture(uDiffuseMap, TexCoord).rgb;
    } else {
        float hue = 0.5 + 0.5 * sin(FragPos.x * 0.05 + FragPos.z * 0.05);
        baseColor = mix(vec3(0.3, 0.5, 0.9), vec3(0.9, 0.5, 0.3), hue);
    }

    // uDebugMode: 0=normal, 1=raw texture, 2=UV coords, 3=normals
    if (uDebugMode == 1) {
        FragColor = vec4(baseColor, 1.0);
        return;
    }
    if (uDebugMode == 2) {
        // Red = U (longitude), Green = V (latitude), Blue = seam indicator
        float seam = fract(TexCoord.x * 64.0) < 0.5 ? 0.3 : 1.0;
        FragColor = vec4(TexCoord.x, TexCoord.y, seam, 1.0);
        return;
    }
    if (uDebugMode == 3) {
        // RGB = world-space normal (should be static when paused)
        vec3 n = abs(normalize(Normal));
        FragColor = vec4(n, 1.0);
        return;
    }

    // Normal lit path (uDebugMode == 0)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);

    vec3 ambient = uAmbientStrength * uLightColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    float distance = length(uLightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.0005 * distance + 0.000003 * distance * distance);

    vec3 result = (ambient + attenuation * diffuse) * baseColor;

    FragColor = vec4(result, 1.0);
}
