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
uniform float uRayleighScaleHeight;
uniform float uMieScaleHeight;

const int   NUM_SAMPLES   = 12;
const int   NUM_LIGHT_SAMPLES = 6;
const float PI = 3.14159265359;

// Henyey-Greenstein phase function for Mie scattering
float hgPhase(float cosTheta, float g) {
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * PI * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

// Rayleigh phase function
float rayleighPhase(float cosTheta) {
    return 3.0 / (16.0 * PI) * (1.0 + cosTheta * cosTheta);
}

// Optical depth from a point to space along a given direction
// Uses exponential atmosphere assumption
float opticalDepth(vec3 pos, vec3 dir, float stepSize, float scaleHeight,
                   vec3 planetCenter, float planetRadius) {
    float depth = 0.0;
    float maxDist = length(pos - planetCenter) * 5.0; // far enough to exit atmosphere

    for (int i = 0; i < NUM_LIGHT_SAMPLES; i++) {
        float t = (float(i) + 0.5) * stepSize;
        if (t > maxDist) break;
        vec3 p = pos + dir * t;
        float h = length(p - planetCenter) - planetRadius;
        if (h < 0.0) return 1e10; // hit planet
        depth += exp(-h / scaleHeight) * stepSize;
    }
    return depth;
}

void main() {
    vec3 rayOrigin = uViewPos;
    vec3 rayDir = normalize(vWorldPos - uViewPos);

    // --- Ray-sphere intersection with outer atmosphere shell ---
    vec3 oc = rayOrigin - uPlanetCenter;
    float b = dot(oc, rayDir);
    float c = dot(oc, oc) - uAtmosphereRadius * uAtmosphereRadius;
    float disc = b * b - c;

    if (disc < 0.0) discard;

    float tNear = -b - sqrt(disc);
    float tFar  = -b + sqrt(disc);
    tNear = max(tNear, 0.0);
    if (tNear >= tFar) discard;

    // --- Ray-sphere intersection with planet (inner boundary) ---
    float cPlanet = dot(oc, oc) - uPlanetRadius * uPlanetRadius;
    float discPlanet = b * b - cPlanet;
    float tPlanet = discPlanet > 0.0 ? abs(-b - sqrt(discPlanet)) : tFar;

    // The visible atmosphere segment
    float tStart = tNear;
    float tEnd = min(tFar, tPlanet);
    if (tEnd <= tStart) discard;

    float segmentLen = tEnd - tStart;
    float stepSize = segmentLen / float(NUM_SAMPLES);

    vec3 lightDir = normalize(uLightPos - uPlanetCenter);
    float cosViewLight = dot(rayDir, lightDir);

    float totalRayleigh = 0.0;
    float totalMie = 0.0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        float t = tStart + (float(i) + 0.5) * stepSize;
        vec3 samplePos = rayOrigin + t * rayDir;
        float h = length(samplePos - uPlanetCenter) - uPlanetRadius;

        float rayleighDensity = exp(-h / uRayleighScaleHeight);
        float mieDensity      = exp(-h / uMieScaleHeight);

        // Optical depth from sample to sun (simplified)
        float lightStep = 50.0; // large step for performance
        float tauRayleigh = opticalDepth(samplePos, lightDir, lightStep,
                                         uRayleighScaleHeight, uPlanetCenter, uPlanetRadius);
        float tauMie      = opticalDepth(samplePos, lightDir, lightStep,
                                         uMieScaleHeight, uPlanetCenter, uPlanetRadius);

        float attenuation = exp(-tauRayleigh - tauMie);

        totalRayleigh += rayleighDensity * attenuation * stepSize;
        totalMie      += mieDensity      * attenuation * stepSize;
    }

    // Phase functions
    float phaseRayleigh = rayleighPhase(cosViewLight);
    float phaseMie      = hgPhase(cosViewLight, 0.76);

    // Scattering result
    vec3 scattering = totalRayleigh * uRayleighColor * phaseRayleigh
                    + totalMie * phaseMie;

    // Edge glow (atmosphere looks brighter at the limb from outside)
    float viewAngle = abs(dot(normalize(vWorldPos - uPlanetCenter), rayDir));
    float edgeBoost = 1.0 + pow(1.0 - viewAngle, 3.0) * 3.0;

    scattering *= edgeBoost;

    // Tone mapping to avoid overexposure
    scattering = scattering / (1.0 + scattering);

    float alpha = clamp(length(scattering) * 2.5, 0.0, 1.0);
    FragColor = vec4(scattering, alpha);
}
