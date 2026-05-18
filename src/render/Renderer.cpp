#include "Renderer.h"
#include "../core/Camera.h"
#include "../scene/SolarSystem.h"

#include <glad/gl.h>

Renderer::Renderer() {
    setupOpenGLState();
}

void Renderer::setupOpenGLState() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
}

void Renderer::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawSkybox(Camera& camera, Skybox& skybox, float aspectRatio) {
    skybox.draw(camera.getViewMatrix(), camera.getProjectionMatrix(aspectRatio));
}

void Renderer::drawSolarSystem(SolarSystem& solarSystem, Camera& camera, float aspectRatio) {
    solarSystem.drawAll(camera, aspectRatio);
}

void Renderer::setViewport(int width, int height) {
    glViewport(0, 0, width, height);
}
