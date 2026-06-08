#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader() = default;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;
    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;

    void load(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;
    void unuse() const;

    unsigned int getId() const { return m_programId; }

    // Uniform setters
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat3(const std::string& name, const glm::mat3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    struct UniformLocationEntry {
        char name[64] = {};
        int location = -1;
    };

    unsigned int compileShader(unsigned int type, const std::string& source);
    int getUniformLocation(const std::string& name) const;

    unsigned int m_programId = 0;
    mutable std::array<UniformLocationEntry, 64> m_uniformLocationCache{};
    mutable std::size_t m_uniformLocationCount = 0;
};
