#include "Skybox.h"
#include "ResourceManager.h"
#include "Shader.h"
#include "Texture.h"

#include <glad/gl.h>

void Skybox::setupMesh() {
    if (m_meshReady) return;

    float skyboxVertices[] = {
            -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    m_meshReady = true;
}

Skybox::Skybox(ResourceManager& resources)
    : m_resources(resources),
      m_shader(&m_resources.getShader("assets/shaders/skybox.vert",
                                      "assets/shaders/skybox.frag"))
{
}

void Skybox::load(const std::string& equirectPath) {
    m_texture = &m_resources.getTexture(equirectPath, true);
}

void Skybox::draw(const glm::mat4& view, const glm::mat4& projection) {
    if (!m_texture || !m_texture->isValid()) return;

    setupMesh();

    glDepthFunc(GL_LEQUAL);
    m_shader->use();

    // Strip translation from view matrix so skybox stays centered on camera
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
    m_shader->setMat4("uView", viewNoTranslation);
    m_shader->setMat4("uProjection", projection);

    m_texture->bind(0);
    m_shader->setInt("uEquirectangularMap", 0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

Skybox::~Skybox() {
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}
