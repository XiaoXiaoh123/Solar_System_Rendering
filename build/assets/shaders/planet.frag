#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D uDiffuseMap;
uniform int      uHasTexture;

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;
uniform float uAmbientStrength;

void main() {
    vec3 baseColor;
    if (uHasTexture == 1) {
        baseColor = texture(uDiffuseMap, TexCoord).rgb;
    } else {
        baseColor = vec3(0.6, 0.6, 0.7);
    }

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);

    // Ambient
    vec3 ambient = uAmbientStrength * uLightColor;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    // Specular (Blinn-Phong)
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * uLightColor * 0.5;

    // Distance attenuation
    float distance = length(uLightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.001 * distance + 0.00001 * distance * distance);

    vec3 result = (ambient + attenuation * (diffuse + specular)) * baseColor;

    FragColor = vec4(result, 1.0);
}
