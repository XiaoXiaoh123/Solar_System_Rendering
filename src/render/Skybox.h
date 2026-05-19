#pragma once

#include "Shader.h"
#include <string>

class Skybox {
public:
    Skybox();
    ~Skybox();

    void load(const std::string& equirectPath);
    void draw(const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int m_texture = 0;
    Shader       m_shader;
};
