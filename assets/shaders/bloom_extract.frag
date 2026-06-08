#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uThreshold;

void main() {
    vec3 color = texture(uScene, TexCoord).rgb;
    float brightness = max(max(color.r, color.g), color.b);
    float bloomAmount = smoothstep(uThreshold, uThreshold + 0.65, brightness);
    FragColor = vec4(color * bloomAmount, 1.0);
}
