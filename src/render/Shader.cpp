#include "Shader.h"
#include "../utils/Paths.h"

#include <glad/gl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstring>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    load(vertexPath, fragmentPath);
}

Shader::~Shader() {
    if (m_programId) {
        glDeleteProgram(m_programId);
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_programId(other.m_programId),
      m_uniformLocationCache(other.m_uniformLocationCache),
      m_uniformLocationCount(other.m_uniformLocationCount) {
    other.m_programId = 0;
    other.m_uniformLocationCount = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_programId) glDeleteProgram(m_programId);
        m_programId = other.m_programId;
        m_uniformLocationCache = other.m_uniformLocationCache;
        m_uniformLocationCount = other.m_uniformLocationCount;
        other.m_programId = 0;
        other.m_uniformLocationCount = 0;
    }
    return *this;
}

static std::string readFile(const std::string& path) {
    std::string resolvedPath = Paths::resolve(path);
    std::ifstream file(resolvedPath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path +
                                 " (resolved: " + resolvedPath + ")");
    }
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

void Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
    if (m_programId) {
        glDeleteProgram(m_programId);
        m_programId = 0;
        m_uniformLocationCount = 0;
    }

    std::string vertSource, fragSource;
    try {
        vertSource = readFile(vertexPath);
        fragSource = readFile(fragmentPath);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Shader file error: ") + e.what());
    }

    unsigned int vertShader = compileShader(GL_VERTEX_SHADER, vertSource);
    unsigned int fragShader = compileShader(GL_FRAGMENT_SHADER, fragSource);

    m_programId = glCreateProgram();
    glAttachShader(m_programId, vertShader);
    glAttachShader(m_programId, fragShader);
    glLinkProgram(m_programId);

    int success;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(m_programId, 1024, nullptr, infoLog);
        throw std::runtime_error("Shader program link error:\n" + std::string(infoLog));
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::string typeName = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        throw std::runtime_error("Shader compile error (" + typeName + "):\n" + std::string(infoLog));
    }

    return shader;
}

void Shader::use() const {
    glUseProgram(m_programId);
}

void Shader::unuse() const {
    glUseProgram(0);
}

int Shader::getUniformLocation(const std::string& name) const {
    for (std::size_t i = 0; i < m_uniformLocationCount; ++i) {
        if (std::strcmp(m_uniformLocationCache[i].name, name.c_str()) == 0) {
            return m_uniformLocationCache[i].location;
        }
    }

    int location = glGetUniformLocation(m_programId, name.c_str());
    if (m_uniformLocationCount < m_uniformLocationCache.size()) {
        UniformLocationEntry& entry = m_uniformLocationCache[m_uniformLocationCount++];
        std::strncpy(entry.name, name.c_str(), sizeof(entry.name) - 1);
        entry.name[sizeof(entry.name) - 1] = '\0';
        entry.location = location;
    }
    return location;
}

void Shader::setBool(const std::string& name, bool value) const {
    int location = getUniformLocation(name);
    if (location >= 0) glUniform1i(location, static_cast<int>(value));
}
void Shader::setInt(const std::string& name, int value) const {
    int location = getUniformLocation(name);
    if (location >= 0) glUniform1i(location, value);
}
void Shader::setFloat(const std::string& name, float value) const {
    int location = getUniformLocation(name);
    if (location >= 0) glUniform1f(location, value);
}
void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    int location = getUniformLocation(name);
    if (location >= 0) glUniform2fv(location, 1, &value[0]);
}
void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    int location = getUniformLocation(name);
    if (location >= 0) glUniform3fv(location, 1, &value[0]);
}
void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    int location = getUniformLocation(name);
    if (location >= 0) glUniform4fv(location, 1, &value[0]);
}
void Shader::setMat3(const std::string& name, const glm::mat3& value) const {
    int location = getUniformLocation(name);
    if (location >= 0) glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
}
void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    int location = getUniformLocation(name);
    if (location >= 0) glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}
