#pragma once

#include "Shader.h"
#include "Skybox.h"

class Camera;
class SolarSystem;

class Renderer {
public:
    struct PostProcessSettings {
        bool  bloomEnabled   = true;
        float exposure       = 1.05f;
        float bloomThreshold = 1.05f;
        float bloomStrength  = 0.55f;
        int   blurPasses     = 6;
    };

    Renderer();
    ~Renderer();

    void beginScene(int width, int height);
    void endScene(const PostProcessSettings& settings);
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

    Shader m_extractShader;
    Shader m_blurShader;
    Shader m_compositeShader;

    unsigned int m_hdrFbo = 0;
    unsigned int m_hdrColorTexture = 0;
    unsigned int m_depthRbo = 0;
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
