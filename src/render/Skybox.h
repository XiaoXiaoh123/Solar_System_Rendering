#pragma once

#include "Shader.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

class Skybox {
public:
    Skybox();
    ~Skybox();

    void load(const std::vector<std::string>& facePaths);
    void draw(const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int m_cubemapTexture = 0;
    Shader       m_shader;
    Mesh         m_cubeMesh;
};
