#include "SolarSystem.h"
#include "SolarSystemConfig.h"
#include "../core/Camera.h"
#include "../core/Time.h"
#include "../render/ResourceManager.h"
#include "../utils/Constants.h"
#include "../render/Shader.h"
#include <glad/gl.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {

constexpr float AU_KM = 149597870.7f;
constexpr float AU_RENDER_SCALE = Constants::EARTH_ORBIT;

struct AtmosphereProfile {
    glm::vec3 rayleighColor;
    glm::vec3 mieColor;
    float densityFalloff;
    float scatteringStrength;
    float alphaStrength;
    float terminatorWidth;
    float edgeStrength;
    float sunsetStrength;
    float backscatterStrength;
};

AtmosphereProfile profileForBody(const CelestialBody& body) {
    const std::string& name = body.getName();

    if (name == "Venus") {
        return {
            glm::vec3(1.0f, 0.78f, 0.42f),
            glm::vec3(1.0f, 0.50f, 0.18f),
            1.35f,
            1.55f,
            0.42f,
            0.34f,
            1.45f,
            1.10f,
            0.75f
        };
    }

    if (name == "Mars") {
        return {
            glm::vec3(0.96f, 0.40f, 0.18f),
            glm::vec3(1.0f, 0.36f, 0.12f),
            0.45f,
            0.48f,
            0.18f,
            0.18f,
            0.58f,
            0.55f,
            0.25f
        };
    }

    return {
        glm::vec3(0.28f, 0.55f, 1.0f),
        glm::vec3(1.0f, 0.46f, 0.18f),
        0.95f,
        1.35f,
        0.34f,
        0.24f,
        1.15f,
        0.85f,
        1.15f
    };
}

} // namespace

SolarSystem::SolarSystem(ResourceManager& resources)
    : m_resources(resources)
{
    m_planetShader = &m_resources.getShader("assets/shaders/planet.vert",
                                            "assets/shaders/planet.frag");
    m_sunShader = &m_resources.getShader("assets/shaders/sun.vert",
                                         "assets/shaders/sun.frag");
    m_atmosphereShader = &m_resources.getShader("assets/shaders/atmosphere.vert",
                                                "assets/shaders/atmosphere.frag");

    std::vector<std::pair<std::string, CelestialBody*>> bodiesByName;
    std::string moonParentName = "Earth";
    glm::vec3 moonOrbitColor(0.4f, 0.4f, 0.5f);
    bool drawMoonOrbit = false;

    std::vector<BodyConfig> configs =
        SolarSystemConfig::load("assets/config/solar_system.ini");

    for (const BodyConfig& config : configs) {
        switch (config.type) {
        case BodyConfig::Type::Star:
            if (m_star) {
                throw std::runtime_error("Solar system config contains more than one star");
            }
            m_star = std::make_unique<Star>(m_resources, config.params);
            bodiesByName.emplace_back(config.params.name, m_star.get());
            break;

        case BodyConfig::Type::Planet: {
            auto planet = std::make_unique<Planet>(m_resources, config.params);
            CelestialBody* planetPtr = planet.get();

            if (config.params.name == "Earth") {
                m_earth = planetPtr;
            }
            if (config.drawOrbit && config.params.orbitRadius > 0.0f) {
                m_orbits.push_back(std::make_unique<Orbit>(m_resources,
                                                           config.params.orbit,
                                                           config.params.orbitRadius,
                                                           config.orbitColor));
                m_orbitBodies.push_back(planetPtr);
            }

            bodiesByName.emplace_back(config.params.name, planetPtr);
            m_planets.push_back(std::move(planet));
            break;
        }

        case BodyConfig::Type::Moon:
            if (m_moon) {
                throw std::runtime_error("Solar system config contains more than one moon");
            }
            m_moon = std::make_unique<Planet>(m_resources, config.params);
            bodiesByName.emplace_back(config.params.name, m_moon.get());
            moonParentName = config.parentName.empty() ? "Earth" : config.parentName;
            moonOrbitColor = config.orbitColor;
            drawMoonOrbit = config.drawOrbit && config.params.orbitRadius > 0.0f;
            break;
        }
    }

    if (!m_star) {
        throw std::runtime_error("Solar system config must contain one star");
    }

    if (m_moon) {
        CelestialBody* parent = nullptr;
        for (const auto& body : bodiesByName) {
            if (body.first == moonParentName) {
                parent = body.second;
                break;
            }
        }
        if (!parent) {
            throw std::runtime_error("Moon parent not found in solar system config: " +
                                     moonParentName);
        }
        m_moon->setParent(parent);

        if (drawMoonOrbit) {
            m_moonOrbit = std::make_unique<Orbit>(m_resources,
                                                  m_moon->getParams().orbit,
                                                  m_moon->getOrbitRadius(),
                                                  moonOrbitColor);
        }
    }

    applyScaleMode();
}

void SolarSystem::update(const Time& time) {
    if (m_star) {
        m_star->update(time);
    }
    for (auto& planet : m_planets) {
        planet->update(time);
    }
    if (m_moon) {
        m_moon->update(time);
    }
}

std::vector<CelestialBody*> SolarSystem::getBodies() const {
    std::vector<CelestialBody*> bodies;
    if (m_star) {
        bodies.push_back(m_star.get());
    }
    for (const auto& planet : m_planets) {
        bodies.push_back(planet.get());
    }
    if (m_moon) {
        bodies.push_back(m_moon.get());
    }
    return bodies;
}

void SolarSystem::drawAll(Camera& camera, float aspectRatio) {
    glm::mat4 view       = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);

    for (auto& orbit : m_orbits) {
        orbit->draw(view, projection);
    }
    // Moon orbit centered on Earth
    if (m_moonOrbit && m_moon) {
        m_moonOrbit->draw(view, projection, m_moon->getParentWorldPosition());
    }

    m_sunShader->use();
    m_sunShader->setMat4("uView", view);
    m_sunShader->setMat4("uProjection", projection);
    m_sunShader->setInt("uDebugMode", m_debugMode);
    if (m_star) {
        m_star->draw(*m_sunShader, camera.getPosition());
    }

    m_planetShader->use();
    m_planetShader->setMat4("uView", view);
    m_planetShader->setMat4("uProjection", projection);
    m_planetShader->setVec3("uLightPos", glm::vec3(0.0f));
    m_planetShader->setVec3("uLightColor", glm::vec3(1.0f, 0.95f, 0.8f));
    m_planetShader->setInt("uDebugMode", m_debugMode);
    m_planetShader->setFloat("uAmbientStrength", m_ambientStrength);

    for (auto& planet : m_planets) {
        planet->draw(*m_planetShader, camera.getPosition());
    }
    if (m_moon) {
        m_moon->draw(*m_planetShader, camera.getPosition());
    }

    // --- Atmosphere pass (transparent overlay) ---
    if (m_showAtmosphere) {
        GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDepthMask(GL_FALSE); // don't write depth for transparent atmosphere

        m_atmosphereShader->use();
        m_atmosphereShader->setMat4("uView", view);
        m_atmosphereShader->setMat4("uProjection", projection);
        m_atmosphereShader->setVec3("uViewPos", camera.getPosition());
        m_atmosphereShader->setVec3("uLightPos", glm::vec3(0.0f));
        auto drawAtmosphere = [&](CelestialBody* body) {
            if (!body || !body->hasAtmosphere()) return;

            AtmosphereProfile profile = profileForBody(*body);
            m_atmosphereShader->setVec3("uRayleighColor", profile.rayleighColor);
            m_atmosphereShader->setVec3("uMieColor", profile.mieColor);
            m_atmosphereShader->setFloat("uDensityFalloff", profile.densityFalloff);
            m_atmosphereShader->setFloat("uScatteringStrength",
                                        profile.scatteringStrength * m_atmosphereTuning.intensity);
            m_atmosphereShader->setFloat("uAlphaStrength",
                                        profile.alphaStrength * m_atmosphereTuning.intensity);
            m_atmosphereShader->setFloat("uTerminatorWidth",
                                        m_atmosphereTuning.terminatorWidth);
            m_atmosphereShader->setFloat("uEdgeStrength",
                                        profile.edgeStrength * m_atmosphereTuning.edgeStrength);
            m_atmosphereShader->setFloat("uSunsetStrength",
                                        profile.sunsetStrength * m_atmosphereTuning.sunsetStrength);
            m_atmosphereShader->setFloat("uBackscatterStrength",
                                        profile.backscatterStrength * m_atmosphereTuning.backscatterStrength);

            body->drawAtmosphere(*m_atmosphereShader, camera.getPosition());
        };

        for (auto& planet : m_planets) {
            drawAtmosphere(planet.get());
        }
        drawAtmosphere(m_moon.get());

        glDepthMask(GL_TRUE);
        if (cullWasEnabled) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } else {
            glDisable(GL_CULL_FACE);
        }
        glDisable(GL_BLEND);
    }
}

void SolarSystem::setTimeScale(float scale) {
    m_timeScale = scale;
}

void SolarSystem::setDebugMode(int mode) {
    m_debugMode = std::clamp(mode, 0, 3);
}

void SolarSystem::setScaleMode(ScaleMode mode) {
    if (m_scaleMode == mode) return;
    m_scaleMode = mode;
    applyScaleMode();
}

float SolarSystem::scaledRadiusFor(const CelestialBody& body) const {
    const CelestialParams& p = body.getParams();
    if (m_scaleMode == ScaleMode::Artistic || p.realRadiusKm <= 0.0f) {
        return p.radius;
    }

    if (m_scaleMode == ScaleMode::Real) {
        return std::max(0.0001f, p.realRadiusKm / AU_KM * AU_RENDER_SCALE);
    }

    float radius = std::log10(1.0f + p.realRadiusKm / 1000.0f) * 2.2f;
    if (p.name == "Sun") {
        radius *= 1.8f;
    }
    return std::max(0.25f, radius);
}

float SolarSystem::scaledOrbitFor(const CelestialBody& body) const {
    const CelestialParams& p = body.getParams();
    if (m_scaleMode == ScaleMode::Artistic || p.semiMajorAxisAU <= 0.0f) {
        return p.orbitRadius;
    }

    if (m_scaleMode == ScaleMode::Real) {
        return p.semiMajorAxisAU * AU_RENDER_SCALE;
    }

    if (p.name == "Moon") {
        return 4.0f + std::log10(1.0f + p.semiMajorAxisAU * 1000.0f) * 2.8f;
    }

    return 80.0f + std::log10(1.0f + p.semiMajorAxisAU * 2.8f) * 175.0f;
}

void SolarSystem::applyScaleMode() {
    if (m_star) {
        m_star->setRenderScale(scaledRadiusFor(*m_star), 0.0f);
    }

    for (auto& planet : m_planets) {
        planet->setRenderScale(scaledRadiusFor(*planet), scaledOrbitFor(*planet));
    }

    if (m_moon) {
        m_moon->setRenderScale(scaledRadiusFor(*m_moon), scaledOrbitFor(*m_moon));
    }

    for (std::size_t i = 0; i < m_orbits.size() && i < m_orbitBodies.size(); ++i) {
        CelestialBody* body = m_orbitBodies[i];
        if (body) {
            m_orbits[i]->updatePath(body->getParams().orbit, scaledOrbitFor(*body));
        }
    }

    if (m_moonOrbit && m_moon) {
        m_moonOrbit->updatePath(m_moon->getParams().orbit, scaledOrbitFor(*m_moon));
    }
}
