#version 460 core

in vec3 TexCoords;

out vec4 FragColor;

uniform sampler2D uEquirectangularMap;

const float PI = 3.14159265359;

void main() {
    vec3 dir = normalize(TexCoords);
    float u = 0.5 + atan(dir.z, dir.x) / (2.0 * PI);
    float v = 0.5 - asin(clamp(dir.y, -1.0, 1.0)) / PI;
    FragColor = texture(uEquirectangularMap, vec2(u, v));
}
