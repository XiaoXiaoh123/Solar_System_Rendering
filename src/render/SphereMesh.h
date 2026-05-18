#pragma once

#include "Mesh.h"

class SphereMesh {
public:
    // Generate a UV sphere with given radius, sectors (longitude), stacks (latitude)
    static Mesh generate(float radius = 1.0f, int sectors = 64, int stacks = 32);
};
