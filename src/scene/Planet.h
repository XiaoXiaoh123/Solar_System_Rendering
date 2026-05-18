#pragma once

#include "CelestialBody.h"

class Planet : public CelestialBody {
public:
    Planet(const CelestialParams& params);

    void draw(Shader& shader) override;
};
