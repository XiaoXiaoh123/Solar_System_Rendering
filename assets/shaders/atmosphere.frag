#version 460 core

in vec3 vWorldPos;
in vec3 vWorldNormal;

out vec4 FragColor;

uniform vec3 uPlanetCenter;
uniform vec3 uViewPos;
uniform vec3 uLightPos;
uniform float uPlanetRadius;
uniform float uAtmosphereRadius;

uniform vec3 uRayleighColor;
uniform vec3 uMieColor;
uniform float uDensityFalloff;
uniform float uScatteringStrength;
uniform float uAlphaStrength;
uniform float uTerminatorWidth;
uniform float uEdgeStrength;
uniform float uSunsetStrength;
uniform float uBackscatterStrength;

void main() {
    vec3 normal = normalize(vWorldPos - uPlanetCenter);
    vec3 viewDir = normalize(uViewPos - vWorldPos);
    vec3 lightDir = normalize(uLightPos - uPlanetCenter);

    float ndotvRaw = dot(normal, viewDir);
    float ndotv = abs(ndotvRaw);
    float ndotl = dot(normal, lightDir);

    float limb = 1.0 - ndotv;
    float rim = limb * limb;
    rim *= rim;
    rim *= uEdgeStrength;

    float daylight = smoothstep(-uTerminatorWidth, uTerminatorWidth, ndotl);
    float night = 1.0 - daylight;
    float sunsetBand = 1.0 - smoothstep(0.0, max(0.025, uTerminatorWidth * 1.35), abs(ndotl));
    float forwardScatter = clamp(dot(viewDir, lightDir), 0.0, 1.0);
    float backScatter = clamp(dot(-viewDir, lightDir), 0.0, 1.0);
    forwardScatter = forwardScatter * forwardScatter;
    backScatter = backScatter * backScatter;

    float opticalDepth = rim * (0.45 + clamp(uDensityFalloff, 0.0, 2.0) * 0.55);
    float sunSide = 0.18 + daylight * 0.82;

    vec3 color = uRayleighColor * opticalDepth * sunSide;
    color += uMieColor * sunsetBand * rim * uSunsetStrength * (0.55 + forwardScatter * 0.45);
    color += uRayleighColor * opticalDepth * night * backScatter * uBackscatterStrength * 1.65;
    color *= uScatteringStrength;

    float alpha = opticalDepth * (0.12 + daylight * 0.74) * uAlphaStrength;
    alpha += sunsetBand * rim * uSunsetStrength * 0.09;
    alpha += opticalDepth * night * backScatter * uBackscatterStrength * 0.16;
    alpha = clamp(alpha, 0.0, 0.46);

    FragColor = vec4(color, alpha);
}
