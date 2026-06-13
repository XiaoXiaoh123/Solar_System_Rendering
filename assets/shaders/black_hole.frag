#version 460 core

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform float uEventHorizonRadius;
uniform float uPhotonSphereRadius;
uniform int uDebugMode;

void main() {
    if (uDebugMode == 1) {
        float r = length(WorldPos) / max(uPhotonSphereRadius, 0.001);
        FragColor = vec4(vec3(clamp(1.0 - r, 0.0, 1.0)), 1.0);
        return;
    }
    if (uDebugMode == 2) {
        vec3 n = abs(normalize(Normal));
        FragColor = vec4(n, 1.0);
        return;
    }
    if (uDebugMode == 3) {
        FragColor = vec4(TexCoord, 0.0, 1.0);
        return;
    }

    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
