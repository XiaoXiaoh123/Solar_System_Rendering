#pragma once

#include "CelestialBody.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct BodyConfig {
    enum class Type {
        Star,
        Planet,
        Moon
    };

    Type            type       = Type::Planet;
    CelestialParams params;
    std::string     parentName;
    bool            drawOrbit  = true;
    glm::vec3       orbitColor = glm::vec3(0.3f, 0.3f, 0.5f);
};

class SolarSystemConfig {
public:
    static std::vector<BodyConfig> load(const std::string& path);
    static std::vector<BodyConfig> defaults();
};
