#include "Skybox.h"
#include "../utils/Paths.h"

#include <glad/gl.h>
#include <stb_image.h>
#include <iostream>
#include <stdexcept>

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

Skybox::Skybox() {
    m_shader.load("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
}

void Skybox::load(const std::string& equirectPath) {
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    std::string resolvedPath = Paths::resolve(equirectPath);
    unsigned char* data = stbi_load(resolvedPath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        throw std::runtime_error("Failed to load skybox texture: " + equirectPath +
                                 " (resolved: " + resolvedPath + ")");
    }

    GLenum internalFormat, dataFormat;
    if (channels == 3) {
        internalFormat = GL_SRGB;
        dataFormat     = GL_RGB;
    } else if (channels == 4) {
        internalFormat = GL_SRGB_ALPHA;
        dataFormat     = GL_RGBA;
    } else {
        stbi_image_free(data);
        throw std::runtime_error("Unsupported skybox texture format: " + equirectPath +
                                 " (resolved: " + resolvedPath + ")");
    }

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                 dataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

void Skybox::draw(const glm::mat4& view, const glm::mat4& projection) {
    setupMesh();

    glDepthFunc(GL_LEQUAL);
    m_shader.use();

    // Strip translation from view matrix so skybox stays centered on camera
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
    m_shader.setMat4("uView", viewNoTranslation);
    m_shader.setMat4("uProjection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    m_shader.setInt("uEquirectangularMap", 0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

Skybox::~Skybox() {
    if (m_texture) glDeleteTextures(1, &m_texture);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}
