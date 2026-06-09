#include "ResourceManager.h"
#include "SphereMesh.h"
#include "../utils/Constants.h"

#include <sstream>
#include <utility>

std::string ResourceManager::shaderKey(const std::string& vertexPath,
                                       const std::string& fragmentPath) {
    return vertexPath + "|" + fragmentPath;
}

std::string ResourceManager::textureKey(const std::string& path, bool sRGB) {
    return path + (sRGB ? "|srgb" : "|linear");
}

std::size_t ResourceManager::lodIndex(SphereLod lod) {
    return static_cast<std::size_t>(lod);
}

Shader& ResourceManager::getShader(const std::string& vertexPath,
                                   const std::string& fragmentPath) {
    const std::string key = shaderKey(vertexPath, fragmentPath);
    auto found = m_shaders.find(key);
    if (found != m_shaders.end()) {
        return *found->second.shader;
    }

    ShaderRecord record;
    record.vertexPath = vertexPath;
    record.fragmentPath = fragmentPath;
    record.shader = std::make_unique<Shader>();
    record.shader->load(vertexPath, fragmentPath);

    auto inserted = m_shaders.emplace(key, std::move(record));
    return *inserted.first->second.shader;
}

Texture& ResourceManager::getTexture(const std::string& path, bool sRGB) {
    const std::string key = textureKey(path, sRGB);
    auto found = m_textures.find(key);
    if (found != m_textures.end()) {
        return *found->second;
    }

    auto texture = std::make_unique<Texture>();
    texture->load(path, sRGB);

    auto inserted = m_textures.emplace(key, std::move(texture));
    return *inserted.first->second;
}

Mesh& ResourceManager::getSphereMesh(SphereLod lod) {
    const std::size_t index = lodIndex(lod);
    if (!m_sphereMeshes[index]) {
        int sectors = Constants::SPHERE_LOD_HIGH_SECTORS;
        int stacks = Constants::SPHERE_LOD_HIGH_STACKS;

        if (lod == SphereLod::Low) {
            sectors = Constants::SPHERE_LOD_LOW_SECTORS;
            stacks = Constants::SPHERE_LOD_LOW_STACKS;
        } else if (lod == SphereLod::Medium) {
            sectors = Constants::SPHERE_LOD_MEDIUM_SECTORS;
            stacks = Constants::SPHERE_LOD_MEDIUM_STACKS;
        }

        m_sphereMeshes[index] = std::make_unique<Mesh>(
            SphereMesh::generate(1.0f, sectors, stacks));
    }

    return *m_sphereMeshes[index];
}

bool ResourceManager::reloadShaders(std::string* errorMessage) {
    std::ostringstream errors;
    bool ok = true;

    for (auto& entry : m_shaders) {
        ShaderRecord& record = entry.second;
        try {
            record.shader->load(record.vertexPath, record.fragmentPath);
        } catch (const std::exception& e) {
            ok = false;
            errors << record.vertexPath << " + " << record.fragmentPath
                   << ": " << e.what() << '\n';
        }
    }

    if (errorMessage) {
        *errorMessage = ok ? std::string() : errors.str();
    }
    return ok;
}

void ResourceManager::clear() {
    for (auto& mesh : m_sphereMeshes) {
        mesh.reset();
    }
    m_textures.clear();
    m_shaders.clear();
}
