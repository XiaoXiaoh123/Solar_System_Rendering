#pragma once

#include <string>
#include <glm/glm.hpp>
#include "../render/Mesh.h"
#include "../render/Texture.h"

class Shader;
class Time;

struct OrbitalElements {
    float semiMajorAxis          = 0.0f;
    float eccentricity           = 0.0f;
    float inclination            = 0.0f;  // degrees
    float longitudeAscendingNode = 0.0f;  // degrees
    float argumentPeriapsis      = 0.0f;  // degrees
    float meanAnomalyAtEpoch     = 0.0f;  // degrees
};

struct CelestialParams {
    std::string name;
    float       radius            = 1.0f;
    float       orbitRadius       = 0.0f;   // 0 = stationary (sun)
    float       realRadiusKm      = 0.0f;
    float       semiMajorAxisAU   = 0.0f;
    OrbitalElements orbit;
    float       orbitPeriodDays   = 0.0f;   // real orbital period in Earth days
    float       rotationPeriodHours = 0.0f; // real rotation period in hours
    float       axialTilt         = 0.0f;   // degrees
    std::string texturePath;
    bool        hasAtmosphere     = false;
    float       atmosphereScale   = 1.05f;   // atmosphere radius = planet radius * scale
};

class CelestialBody {
public:
    CelestialBody(const CelestialParams& params);
    virtual ~CelestialBody() = default;

    virtual void update(const Time& time);
    virtual void draw(Shader& shader);
    virtual void drawAtmosphere(Shader& shader);

    const std::string& getName()        const { return m_params.name; }
    float              getOrbitAngle()  const { return m_orbitAngle; }
    float              getOrbitRadius() const { return m_renderSemiMajorAxis; }
    glm::vec3          getParentWorldPosition() const;
    glm::vec3          getWorldPosition() const;
    glm::mat4          getModelMatrix()   const;
    const CelestialParams& getParams()    const { return m_params; }
    bool               hasAtmosphere()    const { return m_hasAtmosphere; }

    void setParent(CelestialBody* parent) { m_parent = parent; }
    void setRenderScale(float radius, float semiMajorAxis);

    float getBaseRotationSpeed()           const { return m_rotationSpeed; }
    float getRotationSpeedMultiplier()     const { return m_rotationSpeedMultiplier; }
    void  setRotationSpeedMultiplier(float m)    { m_rotationSpeedMultiplier = m; }

protected:
    CelestialParams m_params;
    Mesh            m_mesh;
    Texture         m_texture;
    float           m_renderRadius         = 1.0f;
    float           m_renderSemiMajorAxis  = 0.0f;
    float           m_renderAtmosphereRadius = 0.0f;
    float           m_orbitAngle     = 0.0f;
    float           m_rotationAngle  = 0.0f;
    float           m_orbitSpeed             = 0.0f;
    float           m_rotationSpeed          = 0.0f;
    float           m_rotationSpeedMultiplier = 1.0f;
    CelestialBody*  m_parent                 = nullptr;

    // Atmosphere
    bool            m_hasAtmosphere    = false;
    float           m_atmosphereRadius = 0.0f;
    Mesh            m_atmosphereMesh;
};
