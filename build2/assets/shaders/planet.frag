#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D uDiffuseMap;
uniform int      uHasTexture;
uniform int      uDebugUnlit;   // 1 = raw texture, no lighting

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

    // Diagnostic: output raw texture without any lighting
    if (uDebugUnlit == 1) {
        FragColor = vec4(baseColor, 1.0);
        return;
    }

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
