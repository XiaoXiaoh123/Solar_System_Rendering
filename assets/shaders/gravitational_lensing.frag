#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform int uRayMarchEnabled;
uniform vec2 uCenter;
uniform mat4 uViewProjection;
uniform mat4 uInverseViewProjection;
uniform vec3 uCameraPos;
uniform vec3 uBlackHoleCenter;
uniform float uEventHorizonRadius;
uniform float uPhotonSphereRadius;
uniform float uWorldEventHorizonRadius;
uniform float uWorldPhotonSphereRadius;
uniform float uLensStrength;
uniform float uRingStrength;
uniform float uSpin;
uniform float uLensAsymmetry;
uniform float uShadowSoftness;
uniform float uStepScale;
uniform float uMassStrength;
uniform float uCaptureRadiusScale;
uniform float uAspectRatio;
uniform int uRaySteps;
uniform int uDebugMode;

vec2 toLensSpace(vec2 uv) {
    vec2 d = uv - uCenter;
    d.x *= uAspectRatio;
    return d;
}

vec2 fromLensSpace(vec2 p) {
    p.x /= uAspectRatio;
    return uCenter + p;
}

vec3 safeNormalize(vec3 v, vec3 fallback) {
    float len = length(v);
    return len > 0.00001 ? v / len : fallback;
}

vec3 reconstructRay(vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 farWorld = uInverseViewProjection * vec4(ndc, 1.0, 1.0);
    farWorld /= max(farWorld.w, 0.00001);
    return safeNormalize(farWorld.xyz - uCameraPos, vec3(0.0, 0.0, -1.0));
}

vec2 projectRayDirection(vec3 direction) {
    float distanceToCenter = length(uBlackHoleCenter - uCameraPos);
    float sampleDistance = max(distanceToCenter, uWorldPhotonSphereRadius * 9.0);
    vec4 clip = uViewProjection *
                vec4(uCameraPos + direction * sampleDistance, 1.0);
    if (clip.w <= 0.0001) {
        return TexCoord;
    }

    vec2 uv = clip.xy / clip.w * 0.5 + 0.5;
    return clamp(uv, vec2(0.001), vec2(0.999));
}

vec2 radialLensUv(vec2 lensPos,
                  vec2 dir,
                  float r,
                  float eventRadius,
                  float photonRadius,
                  float influence,
                  out float bendField) {
    float bend = uLensStrength * photonRadius * photonRadius /
                 max(r * r, photonRadius * photonRadius * 0.12);
    bend *= influence;
    bend *= smoothstep(eventRadius * 0.92, photonRadius * 0.9, r);

    vec2 tangent = vec2(-dir.y, dir.x);
    float spinInfluence = influence * smoothstep(eventRadius * 0.9,
                                                 photonRadius * 2.4,
                                                 r);
    float spinBend = clamp(uSpin, -1.0, 1.0) * uLensAsymmetry *
                     photonRadius * 0.38 * spinInfluence /
                     max(1.0 + r / photonRadius, 1.0);

    bendField = abs(bend) + abs(spinBend);
    return fromLensSpace(lensPos + dir * bend + tangent * spinBend);
}

vec2 rayMarchLensUv(float screenInfluence,
                    out float closestDistance,
                    out float captured,
                    out float bendField) {
    vec3 direction = reconstructRay(TexCoord);
    vec3 toCenterFromCamera = uBlackHoleCenter - uCameraPos;
    float centerDistance = length(toCenterFromCamera);
    float closestAlongRay = max(dot(toCenterFromCamera, direction), 0.0);
    float marchRadius = max(uWorldPhotonSphereRadius * 5.5,
                            uWorldEventHorizonRadius * 8.0);
    float startT = max(0.0, closestAlongRay - marchRadius);
    float endT = closestAlongRay + marchRadius;

    int steps = clamp(uRaySteps, 8, 48);
    float stepLength = (endT - startT) / float(steps);
    stepLength *= clamp(uStepScale, 0.35, 1.5);

    vec3 position = uCameraPos + direction * startT;
    closestDistance = centerDistance;
    captured = 0.0;
    bendField = 0.0;

    for (int i = 0; i < 48; ++i) {
        if (i >= steps) {
            break;
        }

        vec3 toCenter = uBlackHoleCenter - position;
        float distanceToCenter = max(length(toCenter), 0.001);
        closestDistance = min(closestDistance, distanceToCenter);

        if (distanceToCenter <
            uWorldEventHorizonRadius * uCaptureRadiusScale) {
            captured = 1.0;
            break;
        }

        vec3 radial = toCenter / distanceToCenter;
        float localInfluence = 1.0 - smoothstep(uWorldPhotonSphereRadius * 0.75,
                                                uWorldPhotonSphereRadius * 6.2,
                                                distanceToCenter);
        float bend = uMassStrength * uLensStrength *
                     uWorldEventHorizonRadius * uWorldEventHorizonRadius *
                     stepLength /
                     max(distanceToCenter * distanceToCenter * distanceToCenter,
                         uWorldEventHorizonRadius *
                         uWorldEventHorizonRadius *
                         uWorldEventHorizonRadius * 0.18);
        bend *= localInfluence * screenInfluence * 3.25;

        vec3 spinAxis = vec3(0.0, 1.0, 0.0);
        vec3 tangent = safeNormalize(cross(spinAxis, radial),
                                     vec3(1.0, 0.0, 0.0));
        float spinBend = bend * clamp(uSpin, -1.0, 1.0) *
                         uLensAsymmetry * 0.22;
        direction = safeNormalize(direction + radial * bend +
                                  tangent * spinBend, direction);
        bendField += abs(bend) + abs(spinBend);
        position += direction * stepLength;
    }

    return projectRayDirection(direction);
}

void main() {
    vec2 lensPos = toLensSpace(TexCoord);
    float r = length(lensPos);
    float eventRadius = max(uEventHorizonRadius, 0.0005);
    float photonRadius = max(uPhotonSphereRadius, eventRadius + 0.0005);
    vec2 dir = r > 0.00001 ? lensPos / r : vec2(1.0, 0.0);

    float screenInfluence = 1.0 - smoothstep(photonRadius * 0.9,
                                             photonRadius * 4.2,
                                             r);
    float radialBend = 0.0;
    vec2 radialUv = radialLensUv(lensPos, dir, r, eventRadius,
                                 photonRadius, screenInfluence,
                                 radialBend);
    radialUv = clamp(radialUv, vec2(0.001), vec2(0.999));

    float closestDistance = uWorldPhotonSphereRadius * 12.0;
    float captured = 0.0;
    float rayBend = radialBend;
    vec2 sampleUv = radialUv;
    if (uRayMarchEnabled == 1 && screenInfluence > 0.001) {
        sampleUv = rayMarchLensUv(screenInfluence, closestDistance,
                                  captured, rayBend);
    }

    vec3 color = texture(uScene, sampleUv).rgb;

    float ringWidth = max(photonRadius * 0.08, 0.002);
    float screenRing = exp(-pow((r - photonRadius) / ringWidth, 2.0));
    float outerRing = exp(-pow((r - photonRadius * 1.18) /
                               max(ringWidth * 1.7, 0.002), 2.0));
    float worldRingWidth = max(uWorldPhotonSphereRadius * 0.10, 0.01);
    float worldRing = exp(-pow((closestDistance - uWorldPhotonSphereRadius) /
                               worldRingWidth, 2.0)) * screenInfluence;
    float ring = max(screenRing, worldRing);

    vec2 tangent2 = vec2(-dir.y, dir.x);
    float ringSkew = 1.0 + clamp(uSpin, -1.0, 1.0) * uLensAsymmetry *
                     dot(tangent2, vec2(1.0, 0.0)) * 0.45;
    ringSkew = clamp(ringSkew, 0.45, 1.65);
    vec3 ringColor = vec3(1.5, 0.82, 0.38) * ring * ringSkew +
                     vec3(0.55, 0.72, 1.25) * outerRing * 0.25;

    vec2 shadowOffset = vec2(clamp(uSpin, -1.0, 1.0) * uLensAsymmetry *
                             eventRadius * 0.24, 0.0);
    float shadowR = length(lensPos - shadowOffset);
    float softness = clamp(uShadowSoftness, 0.02, 0.6);
    float screenShadow = smoothstep(eventRadius * (1.0 - softness),
                                    eventRadius * (1.0 + softness),
                                    shadowR);
    float rayShadow = smoothstep(uWorldEventHorizonRadius *
                                 uCaptureRadiusScale * (1.0 - softness),
                                 uWorldEventHorizonRadius *
                                 uCaptureRadiusScale * (1.0 + softness),
                                 closestDistance);
    float shadow = min(screenShadow, mix(1.0, rayShadow, screenInfluence));
    shadow *= 1.0 - captured;

    if (uDebugMode == 1) {
        FragColor = vec4(vec3(clamp(rayBend * 12.0, 0.0, 1.0)), 1.0);
        return;
    }
    if (uDebugMode == 2) {
        FragColor = vec4(vec3(ring), 1.0);
        return;
    }
    if (uDebugMode == 3) {
        FragColor = vec4(vec3(shadow), 1.0);
        return;
    }
    if (uDebugMode == 4) {
        float closest = 1.0 - smoothstep(uWorldEventHorizonRadius,
                                         uWorldPhotonSphereRadius * 4.0,
                                         closestDistance);
        FragColor = vec4(captured, closest, screenInfluence, 1.0);
        return;
    }

    color = color * shadow + ringColor * uRingStrength;
    FragColor = vec4(color, 1.0);
}
