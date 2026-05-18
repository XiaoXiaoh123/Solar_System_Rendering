#pragma once

#include <string>
#include <glad/gl.h>

class Texture {
public:
    Texture() = default;
    explicit Texture(const std::string& path, bool sRGB = false);
    ~Texture();

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;

    void load(const std::string& path, bool sRGB = false);
    void bind(unsigned int unit = 0) const;
    void unbind() const;

    unsigned int getId()      const { return m_textureId; }
    bool         isValid()    const { return m_textureId != 0; }

private:
    unsigned int m_textureId = 0;
};
