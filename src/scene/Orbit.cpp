#include "Orbit.h"
#include "../utils/Constants.h"

#include <glad/gl.h>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

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

} // namespace

Orbit::Orbit(float radius, const glm::vec3& color)
    : m_color(color)
{
    OrbitalElements elements;
    elements.semiMajorAxis = radius;
    updatePath(elements, radius);
    m_shader.load("assets/shaders/orbit.vert", "assets/shaders/orbit.frag");
}

Orbit::Orbit(const OrbitalElements& elements, float semiMajorAxis, const glm::vec3& color)
    : m_color(color)
{
    updatePath(elements, semiMajorAxis);
    m_shader.load("assets/shaders/orbit.vert", "assets/shaders/orbit.frag");
}

void Orbit::updatePath(const OrbitalElements& elements, float semiMajorAxis) {
    const int segments = Constants::ORBIT_SEGMENTS;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    vertices.reserve(static_cast<std::size_t>(segments + 1));
    indices.reserve(static_cast<std::size_t>(segments * 2));

    float a = std::max(semiMajorAxis, 0.0f);
    float e = std::clamp(elements.eccentricity, 0.0f, 0.95f);
    float b = a * std::sqrt(std::max(0.0f, 1.0f - e * e));

    for (int i = 0; i <= segments; ++i) {
        float E = 2.0f * glm::pi<float>() * static_cast<float>(i) /
                  static_cast<float>(segments);
        Vertex v;
        v.position = rotateOrbitPlane(glm::vec3(a * (std::cos(E) - e),
                                                0.0f,
                                                b * std::sin(E)),
                                      elements);
        v.normal   = glm::vec3(0.0f, 1.0f, 0.0f);
        v.texCoord = glm::vec2(0.0f);
        vertices.push_back(v);
        if (i < segments) {
            indices.push_back(static_cast<unsigned int>(i));
            indices.push_back(static_cast<unsigned int>(i + 1));
        }
    }

    m_mesh.upload(vertices, indices);
}

void Orbit::draw(const glm::mat4& view, const glm::mat4& projection,
                 const glm::vec3& center) {
    m_shader.use();
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", projection);
    m_shader.setMat4("uModel", glm::translate(glm::mat4(1.0f), center));
    m_shader.setVec3("uColor", m_color);

    glLineWidth(1.5f);
    m_mesh.bind();
    glDrawElements(GL_LINES, m_mesh.getIndexCount(), GL_UNSIGNED_INT, 0);
    m_mesh.unbind();
    glLineWidth(1.0f);
}
