#include "Planet.h"
#include "../render/Shader.h"

Planet::Planet(ResourceManager& resources, const CelestialParams& params)
    : CelestialBody(resources, params) {}

void Planet::draw(Shader& shader, const glm::vec3& cameraPosition) {
    CelestialBody::draw(shader, cameraPosition);
}
