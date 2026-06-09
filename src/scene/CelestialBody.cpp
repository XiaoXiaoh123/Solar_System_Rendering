#include "CelestialBody.h"
#include "../render/Shader.h"
#include "../render/Mesh.h"
#include "../render/Texture.h"
#include "../core/Time.h"
#include "../utils/Constants.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace {

float normalizeAngle(float angle) {
    constexpr float twoPi = 2.0f * glm::pi<float>();
    angle = std::fmod(angle, twoPi);
    return angle < 0.0f ? angle + twoPi : angle;
}

float solveEccentricAnomaly(float meanAnomaly, float eccentricity) {
    float e = std::clamp(eccentricity, 0.0f, 0.95f);
    float E = meanAnomaly;

    for (int i = 0; i < 6; ++i) {
        float f = E - e * std::sin(E) - meanAnomaly;
        float fp = 1.0f - e * std::cos(E);
        E -= f / fp;
    }

    return E;
}

glm::vec3 rotateOrbitPlane(const glm::vec3& pos, const OrbitalElements& elements) {
    glm::mat4 transform(1.0f);
    transform = glm::rotate(transform, glm::radians(elements.longitudeAscendingNode),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, glm::radians(elements.inclination),
                            glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::rotate(transform, glm::radians(elements.argumentPeriapsis),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    return glm::vec3(transform * glm::vec4(pos, 1.0f));
}

glm::vec3 orbitalPosition(float meanAnomaly,
                          float semiMajorAxis,
                          const OrbitalElements& elements) {
    if (semiMajorAxis <= 0.0f) return glm::vec3(0.0f);

    float e = std::clamp(elements.eccentricity, 0.0f, 0.95f);
    float E = solveEccentricAnomaly(normalizeAngle(meanAnomaly), e);
    float x = semiMajorAxis * (std::cos(E) - e);
    float z = semiMajorAxis * std::sqrt(std::max(0.0f, 1.0f - e * e)) * std::sin(E);
    return rotateOrbitPlane(glm::vec3(x, 0.0f, z), elements);
}

} // namespace

CelestialBody::CelestialBody(ResourceManager& resources, const CelestialParams& params)
    : m_resources(resources),
      m_params(params)
{
    m_renderRadius = params.radius;
    m_renderSemiMajorAxis = params.orbitRadius;
    m_renderAtmosphereRadius = params.radius * params.atmosphereScale;

    if (!params.texturePath.empty()) {
        try {
            m_texture = &m_resources.getTexture(params.texturePath, true);
        } catch (...) {}
    }

    m_orbitAngle = glm::radians(params.orbit.meanAnomalyAtEpoch);

    constexpr float PI2 = 2.0f * 3.14159265f;
    if (m_params.orbitPeriodDays > 0.0f) {
        m_orbitSpeed = PI2 * Constants::DAYS_PER_SECOND / m_params.orbitPeriodDays;
    }

    if (std::abs(m_params.rotationPeriodHours) > 0.001f) {
        m_rotationSpeed = PI2 * Constants::DAYS_PER_SECOND * 24.0f
                          / m_params.rotationPeriodHours;
    }

    m_hasAtmosphere = params.hasAtmosphere;
    if (m_hasAtmosphere) {
        m_atmosphereRadius = params.radius * params.atmosphereScale;
    }
}

void CelestialBody::update(const Time& time) {
    float dt = time.getDeltaTime();
    m_orbitAngle    += m_orbitSpeed    * dt;
    m_rotationAngle += m_rotationSpeed * m_rotationSpeedMultiplier * dt;
}

glm::vec3 CelestialBody::getParentWorldPosition() const {
    return m_parent ? m_parent->getWorldPosition() : glm::vec3(0.0f);
}

void CelestialBody::setRenderScale(float radius, float semiMajorAxis) {
    m_renderRadius = std::max(radius, 0.0001f);
    m_renderSemiMajorAxis = std::max(semiMajorAxis, 0.0f);
    m_renderAtmosphereRadius = m_renderRadius * m_params.atmosphereScale;
}

glm::vec3 CelestialBody::getWorldPosition() const {
    glm::vec3 localPos = orbitalPosition(m_orbitAngle, m_renderSemiMajorAxis,
                                         m_params.orbit);
    if (m_parent) {
        return m_parent->getWorldPosition() + localPos;
    }
    return localPos;
}

glm::mat4 CelestialBody::getModelMatrix() const {
    glm::mat4 model(1.0f);

    if (m_parent) {
        model = glm::translate(model, m_parent->getWorldPosition());
    }

    model = glm::translate(model, orbitalPosition(m_orbitAngle, m_renderSemiMajorAxis,
                                                 m_params.orbit));

    if (m_params.axialTilt != 0.0f) {
        model = glm::rotate(model, glm::radians(m_params.axialTilt),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    }

    model = glm::rotate(model, m_rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(m_renderRadius));
    return model;
}

ResourceManager::SphereLod CelestialBody::selectLod(const glm::vec3& cameraPosition) const {
    const float distance = std::max(glm::length(cameraPosition - getWorldPosition()), 0.001f);
    const float apparentRadius = m_renderRadius / distance;

    if (apparentRadius > 0.018f || distance < m_renderRadius * 75.0f) {
        return ResourceManager::SphereLod::High;
    }
    if (apparentRadius > 0.006f || distance < m_renderRadius * 220.0f) {
        return ResourceManager::SphereLod::Medium;
    }
    return ResourceManager::SphereLod::Low;
}

void CelestialBody::drawAtmosphere(Shader& shader, const glm::vec3& cameraPosition) {
    if (!m_hasAtmosphere) return;

    glm::mat4 model = getModelMatrix();
    if (m_renderRadius > 0.0001f) {
        model = glm::scale(model, glm::vec3(m_renderAtmosphereRadius / m_renderRadius));
    }

    shader.setMat4("uModel", model);
    shader.setVec3("uPlanetCenter", getWorldPosition());
    shader.setFloat("uPlanetRadius", m_renderRadius);
    shader.setFloat("uAtmosphereRadius", m_renderAtmosphereRadius);

    m_resources.getSphereMesh(selectLod(cameraPosition)).draw();
}

void CelestialBody::draw(Shader& shader, const glm::vec3& cameraPosition) {
    glm::mat4 model = getModelMatrix();
    shader.setMat4("uModel", model);

    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    shader.setMat3("uNormalMatrix", normalMatrix);

    if (m_texture && m_texture->isValid()) {
        m_texture->bind(0);
        shader.setInt("uDiffuseMap", 0);
        shader.setInt("uHasTexture", 1);
    } else {
        shader.setInt("uHasTexture", 0);
    }

    m_resources.getSphereMesh(selectLod(cameraPosition)).draw();
}
