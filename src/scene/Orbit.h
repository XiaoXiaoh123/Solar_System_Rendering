#pragma once

#include "../render/Mesh.h"
#include "../render/Shader.h"
#include <glm/glm.hpp>

class Orbit {
public:
    Orbit(float radius, const glm::vec3& color = glm::vec3(0.3f, 0.3f, 0.5f));
    ~Orbit() = default;

    void draw(const glm::mat4& view, const glm::mat4& projection);

private:
    Mesh     m_mesh;
    Shader   m_shader;
    glm::vec3 m_color;
};
