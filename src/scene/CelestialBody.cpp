#include "CelestialBody.h"
#include "../render/Shader.h"
#include "../render/SphereMesh.h"
#include "../core/Time.h"
#include <glm/gtc/matrix_transform.hpp>

CelestialBody::CelestialBody(const CelestialParams& params)
    : m_params(params)
{
    m_mesh = SphereMesh::generate(params.radius);
    if (!params.texturePath.empty()) {
        try {
            m_texture.load(params.texturePath, true);
        } catch (...) {
            // texture missing — will render with default color
        }
    }
    m_orbitAngle = static_cast<float>(rand()) / RAND_MAX * 6.28318f; // random start
}

void CelestialBody::update(const Time& time) {
    float dt = time.getDeltaTime();
    if (m_params.orbitRadius > 0.0f) {
        m_orbitAngle += m_params.orbitSpeed * dt;
    }
    m_rotationAngle += m_params.rotationSpeed * dt;
}

glm::vec3 CelestialBody::getWorldPosition() const {
    if (m_params.orbitRadius <= 0.0f) {
        return glm::vec3(0.0f); // star at origin
    }
    return glm::vec3(
        cos(m_orbitAngle) * m_params.orbitRadius,
        0.0f,
        sin(m_orbitAngle) * m_params.orbitRadius
    );
}

glm::mat4 CelestialBody::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);

    // Orbit: translate then rotate
    if (m_params.orbitRadius > 0.0f) {
        model = glm::translate(model, glm::vec3(m_params.orbitRadius, 0.0f, 0.0f));
        model = glm::rotate(model, m_orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // Axial tilt
    if (m_params.axialTilt != 0.0f) {
        model = glm::rotate(model, glm::radians(m_params.axialTilt), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // Self rotation
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
