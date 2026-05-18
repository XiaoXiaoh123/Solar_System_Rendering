#pragma once

#include "CelestialBody.h"

class Star : public CelestialBody {
public:
    Star(const CelestialParams& params);

    void draw(Shader& shader) override;

    glm::vec3 getLightColor() const { return glm::vec3(1.0f, 0.95f, 0.8f); }
    float     getIntensity()  const { return 1.0f; }
};
