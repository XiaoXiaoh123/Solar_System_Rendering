#pragma once

#include "Shader.h"
#include "Skybox.h"

class Camera;
class SolarSystem;

class Renderer {
public:
    Renderer();

    void clear();
    void drawSkybox(Camera& camera, Skybox& skybox, float aspectRatio);
    void drawSolarSystem(SolarSystem& solarSystem, Camera& camera, float aspectRatio);
    void setViewport(int width, int height);

private:
    void setupOpenGLState();
};
