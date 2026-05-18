#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <stdexcept>

Texture::Texture(const std::string& path, bool sRGB) {
    load(path, sRGB);
}

Texture::~Texture() {
    if (m_textureId) {
        glDeleteTextures(1, &m_textureId);
    }
}

Texture::Texture(Texture&& other) noexcept : m_textureId(other.m_textureId) {
    other.m_textureId = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (m_textureId) glDeleteTextures(1, &m_textureId);
        m_textureId = other.m_textureId;
        other.m_textureId = 0;
    }
    return *this;
}

void Texture::load(const std::string& path, bool sRGB) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        throw std::runtime_error("Failed to load texture: " + path);
    }

    GLenum internalFormat, dataFormat;
    if (channels == 1) {
        internalFormat = GL_RED;
        dataFormat     = GL_RED;
    } else if (channels == 3) {
        internalFormat = sRGB ? GL_SRGB : GL_RGB;
        dataFormat     = GL_RGB;
    } else if (channels == 4) {
        internalFormat = sRGB ? GL_SRGB_ALPHA : GL_RGBA;
        dataFormat     = GL_RGBA;
    } else {
        stbi_image_free(data);
        throw std::runtime_error("Unsupported texture format: " + path);
    }

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
