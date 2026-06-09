#pragma once

#include <array>
#include <cstddef>

enum class SceneId {
    SolarSystem = 0,
    BlackHole = 1
};

struct SceneDescriptor {
    SceneId id;
    const char* name;
    bool available;
};

class SceneCatalog {
public:
    static constexpr SceneId defaultScene() { return SceneId::SolarSystem; }
    static const std::array<SceneDescriptor, 2>& entries();
    static const SceneDescriptor& descriptor(SceneId id);
    static int indexOf(SceneId id);
};
