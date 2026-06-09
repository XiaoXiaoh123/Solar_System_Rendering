#pragma once

#include <glm/glm.hpp>
#include <string>

class ResourceManager;
class Shader;
class Texture;

class Skybox {
public:
    explicit Skybox(ResourceManager& resources);
    ~Skybox();

    void load(const std::string& equirectPath);
    void draw(const glm::mat4& view, const glm::mat4& projection);

private:
    void setupMesh();

    ResourceManager& m_resources;
    const Texture* m_texture = nullptr;
    unsigned int m_vao     = 0;
    unsigned int m_vbo     = 0;
    bool         m_meshReady = false;
    Shader*      m_shader = nullptr;
};
