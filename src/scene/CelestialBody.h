#pragma once

#include <string>
#include <glm/glm.hpp>
#include "../render/Mesh.h"
#include "../render/Texture.h"

class Shader;
class Time;

struct CelestialParams {
    std::string name;
    float       radius            = 1.0f;
    float       orbitRadius       = 0.0f;   // 0 = stationary (sun)
    float       orbitPeriodDays   = 0.0f;   // real orbital period in Earth days
    float       rotationPeriodHours = 0.0f; // real rotation period in hours
    float       axialTilt         = 0.0f;   // degrees
    std::string texturePath;
};

class CelestialBody {
public:
    CelestialBody(const CelestialParams& params);
    virtual ~CelestialBody() = default;

    virtual void update(const Time& time);
    virtual void draw(Shader& shader);

    const std::string& getName()        const { return m_params.name; }
    float              getOrbitAngle()  const { return m_orbitAngle; }
    float              getOrbitRadius() const { return m_params.orbitRadius; }
    glm::vec3          getWorldPosition() const;
    glm::mat4          getModelMatrix()   const;
    const CelestialParams& getParams()    const { return m_params; }

protected:
    CelestialParams m_params;
    Mesh            m_mesh;
    Texture         m_texture;
    float           m_orbitAngle     = 0.0f;
    float           m_rotationAngle  = 0.0f;
    float           m_orbitSpeed     = 0.0f;  // rad/s (sim time, pre-scaled)
    float           m_rotationSpeed  = 0.0f;  // rad/s (sim time, pre-scaled)
};
