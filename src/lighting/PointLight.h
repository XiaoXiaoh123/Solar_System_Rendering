#pragma once

#include <glm/glm.hpp>

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float     intensity;
    float     linearAttenuation  = 0.001f;
    float     quadraticAttenuation = 0.00001f;

    PointLight(const glm::vec3& pos, const glm::vec3& col, float intens = 1.0f)
        : position(pos), color(col), intensity(intens) {}
};
