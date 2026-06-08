#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uImage;
uniform int uHorizontal;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(uImage, 0));
    vec2 direction = (uHorizontal == 1) ? vec2(texelSize.x, 0.0)
                                        : vec2(0.0, texelSize.y);

    vec3 color = texture(uImage, TexCoord).rgb * 0.227027;
    color += texture(uImage, TexCoord + direction * 1.384615).rgb * 0.316216;
    color += texture(uImage, TexCoord - direction * 1.384615).rgb * 0.316216;
    color += texture(uImage, TexCoord + direction * 3.230769).rgb * 0.070270;
    color += texture(uImage, TexCoord - direction * 3.230769).rgb * 0.070270;

    FragColor = vec4(color, 1.0);
}
