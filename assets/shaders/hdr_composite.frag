#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform sampler2D uBloom;
uniform int uBloomEnabled;
uniform float uExposure;
uniform float uBloomStrength;

void main() {
    vec3 hdrColor = texture(uScene, TexCoord).rgb;
    vec3 bloomColor = texture(uBloom, TexCoord).rgb;

    if (uBloomEnabled == 1) {
        hdrColor += bloomColor * uBloomStrength;
    }

    vec3 mapped = vec3(1.0) - exp(-hdrColor * uExposure);
    mapped = pow(mapped, vec3(1.0 / 2.2));
    FragColor = vec4(mapped, 1.0);
}
