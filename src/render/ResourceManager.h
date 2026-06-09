#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

#include <array>
#include <map>
#include <memory>
#include <string>

class ResourceManager {
public:
    enum class SphereLod {
        Low = 0,
        Medium = 1,
        High = 2
    };

    Shader& getShader(const std::string& vertexPath,
                      const std::string& fragmentPath);
    Texture& getTexture(const std::string& path, bool sRGB = false);
    Mesh& getSphereMesh(SphereLod lod);

    bool reloadShaders(std::string* errorMessage = nullptr);
    void clear();

private:
    struct ShaderRecord {
        std::string vertexPath;
        std::string fragmentPath;
        std::unique_ptr<Shader> shader;
    };

    static std::string shaderKey(const std::string& vertexPath,
                                 const std::string& fragmentPath);
    static std::string textureKey(const std::string& path, bool sRGB);
    static std::size_t lodIndex(SphereLod lod);

    std::map<std::string, ShaderRecord> m_shaders;
    std::map<std::string, std::unique_ptr<Texture>> m_textures;
    std::array<std::unique_ptr<Mesh>, 3> m_sphereMeshes;
};
