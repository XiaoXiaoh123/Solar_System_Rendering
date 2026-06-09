#include "Star.h"
#include "../render/Shader.h"

Star::Star(ResourceManager& resources, const CelestialParams& params)
    : CelestialBody(resources, params) {}

void Star::draw(Shader& shader, const glm::vec3& cameraPosition) {
    shader.setVec3("uLightColor", glm::vec3(1.0f, 0.95f, 0.8f));
    CelestialBody::draw(shader, cameraPosition);
}
