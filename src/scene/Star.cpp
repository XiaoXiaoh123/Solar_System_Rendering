#include "Star.h"
#include "../render/Shader.h"

Star::Star(const CelestialParams& params) : CelestialBody(params) {}

void Star::draw(Shader& shader) {
    shader.setVec3("uLightColor", glm::vec3(1.0f, 0.95f, 0.8f));
    CelestialBody::draw(shader);
}
