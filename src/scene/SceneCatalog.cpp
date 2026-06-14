#include "SceneCatalog.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

namespace {

const std::array<SceneDescriptor, 2> kScenes = {{
    {SceneId::SolarSystem, "Solar System", true},
    {SceneId::BlackHole, "Black Hole", true}
}};

std::string normalizedSceneName(const char* text) {
    std::string value = text ? text : "";
    value.erase(std::remove_if(value.begin(), value.end(), [](char c) {
        return c == '_' || c == '-' || c == ' ';
    }), value.end());
    std::transform(value.begin(), value.end(), value.begin(), [](char c) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    });
    return value;
}

} // namespace

SceneId SceneCatalog::defaultScene() {
    const std::string scene =
        normalizedSceneName(std::getenv("SOLAR_SYSTEM_SCENE"));
    if (scene == "blackhole" || scene == "black") {
        return SceneId::BlackHole;
    }
    return SceneId::SolarSystem;
}

const std::array<SceneDescriptor, 2>& SceneCatalog::entries() {
    return kScenes;
}

const SceneDescriptor& SceneCatalog::descriptor(SceneId id) {
    for (const SceneDescriptor& scene : kScenes) {
        if (scene.id == id) {
            return scene;
        }
    }
    return kScenes[0];
}

int SceneCatalog::indexOf(SceneId id) {
    for (std::size_t i = 0; i < kScenes.size(); ++i) {
        if (kScenes[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return 0;
}
