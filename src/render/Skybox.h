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
    void setupMesh();

    unsigned int m_texture = 0;
    unsigned int m_vao     = 0;
    unsigned int m_vbo     = 0;
    bool         m_meshReady = false;
    Shader       m_shader;
};
