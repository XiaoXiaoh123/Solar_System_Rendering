#include "Planet.h"
#include "../render/Shader.h"

Planet::Planet(const CelestialParams& params) : CelestialBody(params) {}

void Planet::draw(Shader& shader) {
    // Uniforms (uLightPos, uLightColor, uAmbientStrength, uViewPos) are set
    // by SolarSystem::drawAll — do NOT override them here.
    CelestialBody::draw(shader);
}
