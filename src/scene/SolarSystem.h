#pragma once

#include <vector>
#include <memory>
#include "Star.h"
#include "Planet.h"
#include "Orbit.h"

class Camera;
class Shader;
class ResourceManager;

struct AtmosphereTuning {
    float intensity           = 1.0f;
    float edgeStrength        = 1.0f;
    float sunsetStrength      = 1.0f;
    float backscatterStrength = 1.0f;
    float terminatorWidth     = 0.28f;
};

enum class ScaleMode {
    Artistic = 0,
    Real = 1,
    Logarithmic = 2
};

class SolarSystem {
public:
    explicit SolarSystem(ResourceManager& resources);

    void update(const class Time& time);
    void drawAll(Camera& camera, float aspectRatio);

    CelestialBody* getSun()     { return m_star.get(); }
    CelestialBody* getEarth()   { return m_earth; }
    const Star*    getStar()    const { return m_star.get(); }
    const std::vector<std::unique_ptr<Planet>>& getPlanets() const { return m_planets; }
    std::vector<std::unique_ptr<Planet>>&       getPlanets()       { return m_planets; }
    Planet*        getMoon()    const { return m_moon.get(); }
    std::vector<CelestialBody*> getBodies() const;

    void  setTimeScale(float scale);
    float getTimeScale() const { return m_timeScale; }
    void  setAmbientStrength(float s) { m_ambientStrength = s; }
    void  setShowAtmosphere(bool show)  { m_showAtmosphere = show; }
    bool  getShowAtmosphere() const     { return m_showAtmosphere; }
    AtmosphereTuning&       getAtmosphereTuning()       { return m_atmosphereTuning; }
    const AtmosphereTuning& getAtmosphereTuning() const { return m_atmosphereTuning; }
    void      setScaleMode(ScaleMode mode);
    ScaleMode getScaleMode() const { return m_scaleMode; }
    void      setDebugMode(int mode);
    int       getDebugMode() const { return m_debugMode; }

private:
    void applyScaleMode();
    float scaledRadiusFor(const CelestialBody& body) const;
    float scaledOrbitFor(const CelestialBody& body) const;

    std::unique_ptr<Star>                m_star;
    std::vector<std::unique_ptr<Planet>> m_planets;
    std::unique_ptr<Planet>              m_moon;
    std::vector<std::unique_ptr<Orbit>>  m_orbits;
    std::vector<CelestialBody*>          m_orbitBodies;
    std::unique_ptr<Orbit>               m_moonOrbit;
    CelestialBody*                       m_earth = nullptr;
    ResourceManager&                     m_resources;

    Shader* m_planetShader = nullptr;
    Shader* m_sunShader = nullptr;
    Shader* m_atmosphereShader = nullptr;

    float m_timeScale         = 1.0f;
    float m_ambientStrength   = 0.08f;
    bool  m_showAtmosphere    = true;
    int   m_debugMode         = 0;
    AtmosphereTuning m_atmosphereTuning;
    ScaleMode m_scaleMode = ScaleMode::Artistic;
};
