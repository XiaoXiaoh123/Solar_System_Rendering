#pragma once

#include "../render/Mesh.h"
#include "../render/Renderer.h"
#include <glm/glm.hpp>

class Camera;
class ResourceManager;
class Shader;
class Time;

struct BlackHoleParams {
    float eventHorizonRadius = 12.0f;
    float photonSphereRadius = 18.0f;
    float spin = 0.6f;
    float diskInnerRadius = 18.0f;
    float diskOuterRadius = 85.0f;
    float diskInclination = 16.0f;
    float temperatureInner = 1.0f;
    float temperatureOuter = 0.25f;
    float turbulence = 0.55f;
    float rotationSpeed = 0.35f;
    float dopplerBoost = 0.45f;
    float diskIntensity = 1.45f;
    float diskThickness = 2.8f;
    float diskWarp = 0.35f;
    float selfShadow = 0.55f;
    float backLightStrength = 0.65f;
    float plasmaContrast = 0.85f;
    float lensStrength = 0.12f;
    float ringStrength = 0.65f;
    float lensAsymmetry = 0.35f;
    float shadowSoftness = 0.18f;
    bool rayMarchLensing = true;
    int raySteps = 32;
    float rayStepScale = 0.85f;
    float massStrength = 1.0f;
    float captureRadiusScale = 1.0f;
};

class BlackHoleScene {
public:
    explicit BlackHoleScene(ResourceManager& resources);

    void update(const Time& time);
    void draw(Camera& camera, float aspectRatio);
    void drawControls(Camera& camera);
    void drawTeachingOverlay(const Camera& camera,
                             float aspectRatio,
                             int width,
                             int height) const;
    void resetCamera(Camera& camera) const;
    Renderer::LensSettings lensSettings(const Camera& camera,
                                        float aspectRatio) const;

private:
    void loadConfig(const char* path);
    void rebuildDiskMesh();
    void applyParameterPreset(int presetIndex);
    void applyDebugPreset(int presetIndex);
    void applyCameraPreset(Camera& camera, int presetIndex) const;

    ResourceManager& m_resources;
    Shader* m_horizonShader = nullptr;
    Shader* m_diskShader = nullptr;
    Mesh m_diskMesh;
    BlackHoleParams m_params;
    float m_diskPhase = 0.0f;
    int m_debugMode = 0;
    int m_lensDebugMode = 0;
    int m_parameterPresetIndex = 0;
    int m_debugPresetIndex = 0;
    bool m_lensEnabled = true;
    bool m_showTeachingPanel = true;
    bool m_showLabels = true;
};
