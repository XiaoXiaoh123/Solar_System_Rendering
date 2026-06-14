#pragma once

#include "Shader.h"
#include "Skybox.h"
#include <glm/glm.hpp>

class Camera;
class SolarSystem;
class ResourceManager;

class Renderer {
public:
    struct PostProcessSettings {
        bool  bloomEnabled   = true;
        float exposure       = 1.05f;
        float bloomThreshold = 1.05f;
        float bloomStrength  = 0.55f;
        int   blurPasses     = 6;
    };

    struct LensSettings {
        bool enabled = false;
        bool rayMarchEnabled = false;
        glm::vec2 center = glm::vec2(0.5f);
        glm::mat4 viewProjection = glm::mat4(1.0f);
        glm::mat4 inverseViewProjection = glm::mat4(1.0f);
        glm::vec3 cameraPosition = glm::vec3(0.0f);
        glm::vec3 blackHoleCenter = glm::vec3(0.0f);
        float eventHorizonRadius = 0.0f;
        float photonSphereRadius = 0.0f;
        float worldEventHorizonRadius = 0.0f;
        float worldPhotonSphereRadius = 0.0f;
        float strength = 0.0f;
        float ringStrength = 0.0f;
        float spin = 0.0f;
        float asymmetry = 0.0f;
        float frameDragging = 0.55f;
        float ringAsymmetry = 0.65f;
        float shadowOffset = 0.35f;
        float shadowSoftness = 0.12f;
        float stepScale = 0.85f;
        float massStrength = 1.0f;
        float captureRadiusScale = 1.0f;
        float aspectRatio = 1.0f;
        int raySteps = 32;
        int debugMode = 0;
    };

    explicit Renderer(ResourceManager& resources);
    ~Renderer();

    void beginScene(int width, int height);
    void endScene(const PostProcessSettings& settings,
                  const LensSettings& lensSettings);
    void clear();
    void drawSkybox(Camera& camera, Skybox& skybox, float aspectRatio);
    void drawSolarSystem(SolarSystem& solarSystem, Camera& camera, float aspectRatio);
    void setViewport(int width, int height);

private:
    void setupOpenGLState();
    void setupFullscreenQuad();
    void releasePostProcessTargets();
    bool ensurePostProcessTargets(int width, int height);
    void drawFullscreenQuad();

    ResourceManager& m_resources;
    Shader* m_extractShader = nullptr;
    Shader* m_blurShader = nullptr;
    Shader* m_compositeShader = nullptr;
    Shader* m_lensShader = nullptr;

    unsigned int m_hdrFbo = 0;
    unsigned int m_hdrColorTexture = 0;
    unsigned int m_depthRbo = 0;
    unsigned int m_lensFbo = 0;
    unsigned int m_lensTexture = 0;
    unsigned int m_pingpongFbo[2] = {0, 0};
    unsigned int m_pingpongTexture[2] = {0, 0};
    unsigned int m_quadVao = 0;
    unsigned int m_quadVbo = 0;

    int  m_targetWidth = 0;
    int  m_targetHeight = 0;
    int  m_bloomWidth = 0;
    int  m_bloomHeight = 0;
    bool m_postProcessReady = false;
};
