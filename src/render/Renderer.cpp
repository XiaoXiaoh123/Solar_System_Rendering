#include "Renderer.h"
#include "../core/Camera.h"
#include "../scene/SolarSystem.h"

#include <glad/gl.h>
#include <algorithm>
#include <stdexcept>

Renderer::Renderer() {
    m_extractShader.load("assets/shaders/bloom_extract.vert",
                         "assets/shaders/bloom_extract.frag");
    m_blurShader.load("assets/shaders/bloom_blur.vert",
                      "assets/shaders/bloom_blur.frag");
    m_compositeShader.load("assets/shaders/hdr_composite.vert",
                           "assets/shaders/hdr_composite.frag");
    setupFullscreenQuad();
    setupOpenGLState();
}

Renderer::~Renderer() {
    releasePostProcessTargets();
    if (m_quadVbo) glDeleteBuffers(1, &m_quadVbo);
    if (m_quadVao) glDeleteVertexArrays(1, &m_quadVao);
}

void Renderer::setupOpenGLState() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
}

void Renderer::setupFullscreenQuad() {
    const float quadVertices[] = {
        // pos      // uv
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVao);
    glGenBuffers(1, &m_quadVbo);
    glBindVertexArray(m_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void Renderer::releasePostProcessTargets() {
    if (m_hdrColorTexture) glDeleteTextures(1, &m_hdrColorTexture);
    if (m_depthRbo) glDeleteRenderbuffers(1, &m_depthRbo);
    if (m_hdrFbo) glDeleteFramebuffers(1, &m_hdrFbo);
    glDeleteTextures(2, m_pingpongTexture);
    glDeleteFramebuffers(2, m_pingpongFbo);

    m_hdrFbo = 0;
    m_hdrColorTexture = 0;
    m_depthRbo = 0;
    m_pingpongFbo[0] = m_pingpongFbo[1] = 0;
    m_pingpongTexture[0] = m_pingpongTexture[1] = 0;
    m_postProcessReady = false;
}

bool Renderer::ensurePostProcessTargets(int width, int height) {
    width = std::max(width, 1);
    height = std::max(height, 1);

    if (m_postProcessReady && width == m_targetWidth && height == m_targetHeight) {
        return true;
    }

    releasePostProcessTargets();

    m_targetWidth = width;
    m_targetHeight = height;
    m_bloomWidth = std::max(1, width / 2);
    m_bloomHeight = std::max(1, height / 2);

    glGenFramebuffers(1, &m_hdrFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFbo);

    glGenTextures(1, &m_hdrColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_hdrColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
                 GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_hdrColorTexture, 0);

    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, m_depthRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        throw std::runtime_error("HDR framebuffer is incomplete");
    }

    glGenFramebuffers(2, m_pingpongFbo);
    glGenTextures(2, m_pingpongTexture);
    for (int i = 0; i < 2; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFbo[i]);
        glBindTexture(GL_TEXTURE_2D, m_pingpongTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_bloomWidth, m_bloomHeight,
                     0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, m_pingpongTexture[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            throw std::runtime_error("Bloom framebuffer is incomplete");
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_postProcessReady = true;
    return true;
}

void Renderer::beginScene(int width, int height) {
    ensurePostProcessTargets(width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFbo);
    glViewport(0, 0, m_targetWidth, m_targetHeight);
    setupOpenGLState();
}

void Renderer::drawFullscreenQuad() {
    glBindVertexArray(m_quadVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::endScene(const PostProcessSettings& settings) {
    if (!m_postProcessReady) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFbo[0]);
    glViewport(0, 0, m_bloomWidth, m_bloomHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    m_extractShader.use();
    m_extractShader.setInt("uScene", 0);
    m_extractShader.setFloat("uThreshold", settings.bloomThreshold);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_hdrColorTexture);
    drawFullscreenQuad();

    bool horizontal = true;
    bool firstPass = true;
    int blurPasses = std::max(0, settings.blurPasses);
    m_blurShader.use();
    m_blurShader.setInt("uImage", 0);
    for (int i = 0; i < blurPasses; ++i) {
        int target = horizontal ? 1 : 0;
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFbo[target]);
        glViewport(0, 0, m_bloomWidth, m_bloomHeight);
        m_blurShader.setInt("uHorizontal", horizontal ? 1 : 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, firstPass ? m_pingpongTexture[0]
                                               : m_pingpongTexture[horizontal ? 0 : 1]);
        drawFullscreenQuad();
        horizontal = !horizontal;
        firstPass = false;
    }

    int bloomTextureIndex = firstPass ? 0 : (horizontal ? 0 : 1);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_targetWidth, m_targetHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_compositeShader.use();
    m_compositeShader.setInt("uScene", 0);
    m_compositeShader.setInt("uBloom", 1);
    m_compositeShader.setInt("uBloomEnabled", settings.bloomEnabled ? 1 : 0);
    m_compositeShader.setFloat("uExposure", settings.exposure);
    m_compositeShader.setFloat("uBloomStrength", settings.bloomStrength);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_hdrColorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_pingpongTexture[bloomTextureIndex]);
    drawFullscreenQuad();
    glActiveTexture(GL_TEXTURE0);

    if (depthWasEnabled) glEnable(GL_DEPTH_TEST);
    if (cullWasEnabled) glEnable(GL_CULL_FACE);
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
