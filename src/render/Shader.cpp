#include "Shader.h"

#include <glad/gl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    load(vertexPath, fragmentPath);
}

Shader::~Shader() {
    if (m_programId) {
        glDeleteProgram(m_programId);
    }
}

Shader::Shader(Shader&& other) noexcept : m_programId(other.m_programId) {
    other.m_programId = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_programId) glDeleteProgram(m_programId);
        m_programId = other.m_programId;
        other.m_programId = 0;
    }
    return *this;
}

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

void Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
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

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(m_programId, name.c_str()), static_cast<int>(value));
}
void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(m_programId, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_programId, name.c_str()), value);
}
void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(m_programId, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(m_programId, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(m_programId, name.c_str()), 1, &value[0]);
}
void Shader::setMat3(const std::string& name, const glm::mat3& value) const {
    glUniformMatrix3fv(glGetUniformLocation(m_programId, name.c_str()), 1, GL_FALSE, &value[0][0]);
}
void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(m_programId, name.c_str()), 1, GL_FALSE, &value[0][0]);
}
