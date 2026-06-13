#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 WorldPos;
out vec3 LocalPos;
out vec3 Normal;
out vec2 DiskCoord;
out float WarpHeight;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTime;
uniform float uSpin;
uniform float uDiskThickness;
uniform float uDiskWarp;

void main() {
    float radial = clamp(aTexCoord.x, 0.0, 1.0);
    float angle = aTexCoord.y * 6.28318530718;
    float spinDir = sign(abs(uSpin) < 0.001 ? 1.0 : uSpin);
    float warpMask = smoothstep(0.03, 0.22, radial) *
                     (1.0 - smoothstep(0.88, 1.0, radial));
    float warp = sin(angle * 2.0 - radial * 6.0 + uTime * 0.7 * spinDir) * 0.55;
    warp += sin(angle * 5.0 + radial * 11.0 - uTime * 1.3 * spinDir) * 0.20;

    vec3 local = aPos;
    local.y += warp * warpMask * uDiskThickness * uDiskWarp * 0.18;

    LocalPos = local;
    WarpHeight = warp * warpMask;
    WorldPos = vec3(uModel * vec4(local, 1.0));
    Normal = mat3(transpose(inverse(uModel))) * aNormal;
    DiskCoord = aTexCoord;
    gl_Position = uProjection * uView * vec4(WorldPos, 1.0);
}
