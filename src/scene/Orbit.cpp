#include "Orbit.h"
#include <glad/gl.h>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <cmath>

Orbit::Orbit(float radius, const glm::vec3& color)
    : m_color(color)
{
    const int segments = 256;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        Vertex v;
        v.position = glm::vec3(cos(angle) * radius, 0.0f, sin(angle) * radius);
        v.normal   = glm::vec3(0.0f, 1.0f, 0.0f);
        v.texCoord = glm::vec2(0.0f);
        vertices.push_back(v);
        if (i < segments) {
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    }

    m_mesh.upload(vertices, indices);
    m_shader.load("assets/shaders/orbit.vert", "assets/shaders/orbit.frag");
}

void Orbit::draw(const glm::mat4& view, const glm::mat4& projection) {
    m_shader.use();
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", projection);
    m_shader.setMat4("uModel", glm::mat4(1.0f));
    m_shader.setVec3("uColor", m_color);

    glLineWidth(1.5f);
    m_mesh.bind();
    glDrawElements(GL_LINES, m_mesh.getIndexCount(), GL_UNSIGNED_INT, 0);
    m_mesh.unbind();
    glLineWidth(1.0f);
}
