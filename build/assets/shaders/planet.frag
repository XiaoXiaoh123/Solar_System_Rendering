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
        // Orbit-angle driven color so motion is visually obvious
        float hue = 0.5 + 0.5 * sin(FragPos.x * 0.05 + FragPos.z * 0.05);
        baseColor = mix(vec3(0.3, 0.5, 0.9), vec3(0.9, 0.5, 0.3), hue);
    }

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);

    vec3 ambient = uAmbientStrength * uLightColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * uLightColor * 0.5;

    float distance = length(uLightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.0005 * distance + 0.000003 * distance * distance);

    vec3 result = (ambient + attenuation * (diffuse + specular)) * baseColor;

    FragColor = vec4(result, 1.0);
}
