#include "Planet.h"
#include "../render/Shader.h"

Planet::Planet(const CelestialParams& params) : CelestialBody(params) {}

void Planet::draw(Shader& shader) {
    shader.setVec3("uLightPos", glm::vec3(0.0f)); // sun at origin
    shader.setVec3("uLightColor", glm::vec3(1.0f, 0.95f, 0.8f));
    shader.setFloat("uAmbientStrength", 0.08f);
    shader.setVec3("uViewPos", glm::vec3(0.0f)); // set by renderer via shader

    CelestialBody::draw(shader);
}
