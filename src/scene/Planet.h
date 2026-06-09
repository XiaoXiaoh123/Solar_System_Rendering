#pragma once

#include "CelestialBody.h"

class Planet : public CelestialBody {
public:
    Planet(ResourceManager& resources, const CelestialParams& params);

    void draw(Shader& shader, const glm::vec3& cameraPosition) override;
};
