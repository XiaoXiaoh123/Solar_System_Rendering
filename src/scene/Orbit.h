#pragma once

#include "../render/Mesh.h"
#include "../render/ResourceManager.h"
#include "CelestialBody.h"
#include <glm/glm.hpp>

class Orbit {
public:
    Orbit(ResourceManager& resources,
          float radius,
          const glm::vec3& color = glm::vec3(0.3f, 0.3f, 0.5f));
    Orbit(ResourceManager& resources,
          const OrbitalElements& elements,
          float semiMajorAxis,
          const glm::vec3& color = glm::vec3(0.3f, 0.3f, 0.5f));
    ~Orbit() = default;

    void updatePath(const OrbitalElements& elements, float semiMajorAxis);
    void draw(const glm::mat4& view, const glm::mat4& projection,
              const glm::vec3& center = glm::vec3(0.0f));

private:
    Mesh     m_mesh;
    Shader*  m_shader = nullptr;
    glm::vec3 m_color;
};
