#include "SceneCatalog.h"

namespace {

const std::array<SceneDescriptor, 2> kScenes = {{
    {SceneId::SolarSystem, "Solar System", true},
    {SceneId::BlackHole, "Black Hole (planned)", false}
}};

} // namespace

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
