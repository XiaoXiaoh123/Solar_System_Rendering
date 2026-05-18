#include "CelestialBody.h"
#include "../render/Shader.h"
#include "../render/SphereMesh.h"
#include "../core/Time.h"
#include "../utils/Constants.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

CelestialBody::CelestialBody(const CelestialParams& params)
    : m_params(params)
{
    m_mesh = SphereMesh::generate(params.radius);

    if (!params.texturePath.empty()) {
        try {
            m_texture.load(params.texturePath, true);
        } catch (...) {}
    }

    // Random starting orbital phase
    m_orbitAngle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;

    // Convert real periods to simulation angular velocities (rad/s sim-time).
    // 1 real second = DAYS_PER_SECOND simulation Earth-days.
    //
    // Orbit:  2*PI * DAYS_PER_SECOND / orbitPeriodDays
    //   e.g. Earth: 2*PI * 30 / 365.25 ≈ 0.516 rad/s → orbit in ~12 real seconds
    //
    // Rotation:  real period = R_hours * 3600 seconds.
    //   sim speed  = 2*PI / (R_hours*3600) * DAYS_PER_SECOND * 86400
    //              = 2*PI * DAYS_PER_SECOND * 24 / R_hours
    constexpr float PI2 = 2.0f * 3.14159265f;

    if (m_params.orbitPeriodDays > 0.0f) {
        m_orbitSpeed = PI2 * Constants::DAYS_PER_SECOND / m_params.orbitPeriodDays;
    }

    if (std::abs(m_params.rotationPeriodHours) > 0.001f) {
        m_rotationSpeed = PI2 * Constants::DAYS_PER_SECOND * 24.0f
                          / m_params.rotationPeriodHours;
    }

}

void CelestialBody::update(const Time& time) {
    float dt = time.getDeltaTime();
    m_orbitAngle    += m_orbitSpeed    * dt;
    m_rotationAngle += m_rotationSpeed * dt;
}

glm::vec3 CelestialBody::getWorldPosition() const {
    if (m_params.orbitRadius <= 0.0f) {
        return glm::vec3(0.0f);
    }
    return glm::vec3(
        cos(m_orbitAngle) * m_params.orbitRadius,
        0.0f,
        sin(m_orbitAngle) * m_params.orbitRadius
    );
}

glm::mat4 CelestialBody::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);

    if (m_params.orbitRadius > 0.0f) {
        // GLM: model = model * newMatrix, so v' = model * v.
        // We want: 1) translate to orbit, 2) rotate around origin.
        //   model = R_orbit * T_orbit
        //   v' = R_orbit * T_orbit * v → translate then orbit
        model = glm::rotate(model, m_orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(m_params.orbitRadius, 0.0f, 0.0f));
    }

    if (m_params.axialTilt != 0.0f) {
        model = glm::rotate(model, glm::radians(m_params.axialTilt), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    model = glm::rotate(model, m_rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    return model;
}

void CelestialBody::draw(Shader& shader) {
    shader.setMat4("uModel", getModelMatrix());

    if (m_texture.isValid()) {
        m_texture.bind(0);
        shader.setInt("uHasTexture", 1);
    } else {
        shader.setInt("uHasTexture", 0);
    }

    m_mesh.draw();
}
