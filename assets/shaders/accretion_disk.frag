#version 460 core

in vec3 WorldPos;
in vec3 LocalPos;
in vec3 Normal;
in vec2 DiskCoord;
in float WarpHeight;

out vec4 FragColor;

uniform vec3 uCameraPos;
uniform vec3 uDiskNormal;
uniform float uTime;
uniform float uInnerRadius;
uniform float uOuterRadius;
uniform float uSpin;
uniform float uTemperatureInner;
uniform float uTemperatureOuter;
uniform float uTurbulence;
uniform float uDopplerBoost;
uniform float uDiskIntensity;
uniform float uDiskThickness;
uniform float uDiskWarp;
uniform float uSelfShadow;
uniform float uBackLightStrength;
uniform float uPlasmaContrast;
uniform int uDebugMode;

float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float layeredNoise(vec2 p) {
    float cellA = hash12(floor(p * 14.0));
    float cellB = hash12(floor(p * 31.0 + vec2(7.0, 3.0)));
    float n = 0.0;
    n += sin(p.x * 2.7 + p.y * 8.0 + uTime * 1.7) * 0.34;
    n += sin(p.x * 6.1 - p.y * 3.5 - uTime * 2.3) * 0.24;
    n += sin(p.x * 13.0 + p.y * 1.7 + cellA * 6.2831853) * 0.20;
    n += (cellA - 0.5) * 0.26;
    n += (cellB - 0.5) * 0.16;
    return n;
}

vec3 safeNormalize(vec3 v, vec3 fallback) {
    float len = length(v);
    return len > 0.00001 ? v / len : fallback;
}

void main() {
    float radial = clamp(DiskCoord.x, 0.0, 1.0);
    float spinDir = sign(abs(uSpin) < 0.001 ? 1.0 : uSpin);
    float angle = DiskCoord.y * 6.28318530718 + uTime * spinDir;
    float radius = mix(uInnerRadius, uOuterRadius, radial);
    float temperature = mix(uTemperatureInner, uTemperatureOuter, pow(radial, 0.72));
    float innerHot = 1.0 - smoothstep(0.06, 0.82, radial);

    vec2 spiralUv = vec2(angle * 1.35 - radial * 9.0 + uTime * 0.65 * spinDir,
                         radial * 8.0 + radius * 0.015);
    float broadNoise = layeredNoise(spiralUv);
    float fineNoise = layeredNoise(vec2(angle * 3.8 + uTime * 1.1 * spinDir,
                                        radial * 21.0 - uTime * 0.7));
    float spiral = sin(angle * 3.0 - radial * 18.0 + broadNoise * 1.5);
    float lanes = 0.78;
    lanes += broadNoise * 0.46 * uTurbulence;
    lanes += spiral * 0.20 * uTurbulence;
    lanes += fineNoise * 0.28 * uTurbulence;
    lanes = max(0.04, 1.0 + (lanes - 1.0) * (0.45 + max(uPlasmaContrast, 0.0)));

    vec3 outerColor = vec3(1.1, 0.24, 0.045);
    vec3 midColor = vec3(1.65, 0.74, 0.18);
    vec3 innerColor = vec3(2.8, 2.15, 1.05);
    vec3 color = mix(outerColor, midColor, smoothstep(0.12, 0.78, temperature));
    color = mix(color, innerColor, smoothstep(0.78, 1.35, temperature));

    vec3 diskNormal = normalize(uDiskNormal);
    vec3 radialOnPlane = WorldPos - diskNormal * dot(WorldPos, diskNormal);
    vec3 radialDir = safeNormalize(radialOnPlane, safeNormalize(WorldPos, vec3(1.0, 0.0, 0.0)));
    vec3 tangent = safeNormalize(cross(diskNormal, radialDir), vec3(1.0, 0.0, 0.0)) * spinDir;
    vec3 viewDir = safeNormalize(uCameraPos - WorldPos, vec3(0.0, 0.0, 1.0));
    float doppler = dot(tangent, viewDir);
    float boost = pow(max(0.05, 1.0 + doppler * uDopplerBoost), 3.0);
    boost = clamp(boost, 0.08, 4.8);

    float blueShift = smoothstep(0.02, 0.82, doppler) * clamp(uDopplerBoost, 0.0, 1.5);
    float redShift = smoothstep(0.02, 0.72, -doppler) * clamp(uDopplerBoost, 0.0, 1.5);
    color *= mix(vec3(1.0), vec3(0.62, 0.88, 1.46), blueShift * 0.55);
    color *= mix(vec3(1.0), vec3(1.24, 0.64, 0.38), redShift * 0.45);

    float radialFacing = dot(radialDir, viewDir);
    float nearSide = smoothstep(0.06, 0.58, radialFacing);
    float farSide = smoothstep(0.02, 0.62, -radialFacing);
    float edgeView = pow(1.0 - abs(dot(viewDir, diskNormal)), 1.25);
    float innerMask = 1.0 - smoothstep(0.68, 1.0, radial);
    float nearOcclusion = nearSide * edgeView * clamp(uSelfShadow, 0.0, 1.0) *
                          (0.20 + 0.55 * innerHot) * innerMask;
    float farLift = farSide * edgeView * uBackLightStrength *
                    (0.18 + 0.58 * innerHot) * innerMask;
    float innerBite = mix(1.0, mix(0.50, 1.0, smoothstep(0.015, 0.20, radial)),
                          clamp(uSelfShadow, 0.0, 1.0));
    float visibility = clamp((1.0 - nearOcclusion) * (1.0 + farLift), 0.05, 1.85);

    float thicknessAmount = clamp(uDiskThickness / 8.0, 0.0, 1.4);
    float thicknessBand = smoothstep(0.02, 0.20, radial) *
                          (1.0 - smoothstep(0.76, 1.0, radial));
    float edgeGlow = edgeView * thicknessAmount * thicknessBand *
                     (0.25 + 0.55 * farSide + 0.18 * abs(WarpHeight));

    float innerFade = smoothstep(0.0, 0.08, radial);
    float outerFade = 1.0 - smoothstep(0.82, 1.0, radial);
    float opacity = 0.28 + lanes * 0.42 + edgeGlow * 0.36;
    float alpha = clamp(innerFade * outerFade * opacity *
                        (1.0 - nearOcclusion * 0.45), 0.0, 0.95);

    if (uDebugMode == 1) {
        FragColor = vec4(vec3(temperature), 1.0);
        return;
    }
    if (uDebugMode == 2) {
        FragColor = vec4(doppler > 0.0 ? vec3(doppler, doppler * 0.55, 0.08)
                                        : vec3(0.08, 0.22, -doppler), 1.0);
        return;
    }
    if (uDebugMode == 3) {
        FragColor = vec4(vec3(alpha), 1.0);
        return;
    }
    if (uDebugMode == 4) {
        FragColor = vec4(nearSide, farSide, edgeView, 1.0);
        return;
    }

    vec3 emission = color * lanes * boost * visibility * innerBite * uDiskIntensity;
    emission += color * edgeGlow * (0.55 + uBackLightStrength) * (1.0 + innerHot);
    FragColor = vec4(emission, alpha);
}
