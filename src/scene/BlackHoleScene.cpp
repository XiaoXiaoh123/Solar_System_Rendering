#include "BlackHoleScene.h"
#include "../core/Camera.h"
#include "../core/Time.h"
#include "../render/ResourceManager.h"
#include "../render/Shader.h"
#include "../utils/Paths.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "imgui.h"

namespace {

void logBlackHoleInit(const std::string& msg) {
    try {
        std::ofstream log(Paths::writableLogPath("SolarSystem.log"), std::ios::app);
        log << "blackhole: " << msg << '\n';
    } catch (...) {}
}

std::string trim(const std::string& text) {
    std::size_t first = 0;
    while (first < text.size() &&
           std::isspace(static_cast<unsigned char>(text[first]))) {
        ++first;
    }

    std::size_t last = text.size();
    while (last > first &&
           std::isspace(static_cast<unsigned char>(text[last - 1]))) {
        --last;
    }

    return text.substr(first, last - first);
}

bool tryParseFloat(const std::string& value, float& out) {
    try {
        std::size_t consumed = 0;
        out = std::stof(value, &consumed);
        return consumed > 0;
    } catch (...) {
        return false;
    }
}

void assignParam(BlackHoleParams& params,
                 const std::string& key,
                 float value) {
    if (key == "eventHorizonRadius") params.eventHorizonRadius = value;
    else if (key == "photonSphereRadius") params.photonSphereRadius = value;
    else if (key == "spin") params.spin = value;
    else if (key == "diskInnerRadius") params.diskInnerRadius = value;
    else if (key == "diskOuterRadius") params.diskOuterRadius = value;
    else if (key == "diskInclination") params.diskInclination = value;
    else if (key == "temperatureInner") params.temperatureInner = value;
    else if (key == "temperatureOuter") params.temperatureOuter = value;
    else if (key == "turbulence") params.turbulence = value;
    else if (key == "rotationSpeed") params.rotationSpeed = value;
    else if (key == "dopplerBoost") params.dopplerBoost = value;
    else if (key == "diskIntensity") params.diskIntensity = value;
    else if (key == "diskThickness") params.diskThickness = value;
    else if (key == "diskWarp") params.diskWarp = value;
    else if (key == "selfShadow") params.selfShadow = value;
    else if (key == "backLightStrength") params.backLightStrength = value;
    else if (key == "plasmaContrast") params.plasmaContrast = value;
    else if (key == "lensStrength") params.lensStrength = value;
    else if (key == "ringStrength") params.ringStrength = value;
    else if (key == "lensAsymmetry") params.lensAsymmetry = value;
    else if (key == "shadowSoftness") params.shadowSoftness = value;
    else if (key == "rayMarchLensing") params.rayMarchLensing = value > 0.5f;
    else if (key == "raySteps") params.raySteps = static_cast<int>(value);
    else if (key == "rayStepScale") params.rayStepScale = value;
    else if (key == "massStrength") params.massStrength = value;
    else if (key == "captureRadiusScale") params.captureRadiusScale = value;
}

bool projectToUv(const Camera& camera,
                 float aspectRatio,
                 const glm::vec3& worldPos,
                 glm::vec2& uv) {
    glm::vec4 clip = camera.getProjectionMatrix(aspectRatio) *
                     camera.getViewMatrix() *
                     glm::vec4(worldPos, 1.0f);
    if (clip.w <= 0.001f) return false;

    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    if (ndc.z < -1.0f || ndc.z > 1.0f) return false;

    uv = glm::vec2(ndc.x * 0.5f + 0.5f,
                   ndc.y * 0.5f + 0.5f);
    return true;
}

float projectedRadius(const Camera& camera,
                      float aspectRatio,
                      float worldRadius,
                      const glm::vec2& centerUv) {
    glm::vec3 front = glm::normalize(camera.getFront());
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::cross(front, worldUp);
    if (glm::length(right) < 0.001f) {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    } else {
        right = glm::normalize(right);
    }
    glm::vec3 up = glm::normalize(glm::cross(right, front));

    glm::vec2 uvRight;
    glm::vec2 uvUp;
    float radius = 0.0f;
    if (projectToUv(camera, aspectRatio, right * worldRadius, uvRight)) {
        glm::vec2 d = uvRight - centerUv;
        d.x *= aspectRatio;
        radius = std::max(radius, glm::length(d));
    }
    if (projectToUv(camera, aspectRatio, up * worldRadius, uvUp)) {
        glm::vec2 d = uvUp - centerUv;
        d.x *= aspectRatio;
        radius = std::max(radius, glm::length(d));
    }

    return radius;
}

BlackHoleParams presetParams(int presetIndex) {
    BlackHoleParams params;
    if (presetIndex == 1) {
        params.spin = 0.35f;
        params.diskInnerRadius = 20.0f;
        params.diskOuterRadius = 98.0f;
        params.diskInclination = 18.0f;
        params.temperatureInner = 1.1f;
        params.temperatureOuter = 0.22f;
        params.turbulence = 0.42f;
        params.dopplerBoost = 0.35f;
        params.diskIntensity = 1.3f;
        params.diskThickness = 2.2f;
        params.diskWarp = 0.22f;
        params.backLightStrength = 0.95f;
        params.lensStrength = 0.23f;
        params.ringStrength = 1.15f;
        params.lensAsymmetry = 0.18f;
        params.shadowSoftness = 0.24f;
        params.raySteps = 40;
        params.rayStepScale = 0.78f;
        params.massStrength = 1.35f;
    } else if (presetIndex == 2) {
        params.spin = 0.95f;
        params.diskInclination = 14.0f;
        params.temperatureInner = 1.25f;
        params.temperatureOuter = 0.18f;
        params.turbulence = 0.72f;
        params.rotationSpeed = 0.55f;
        params.dopplerBoost = 0.95f;
        params.diskIntensity = 1.65f;
        params.diskThickness = 2.6f;
        params.diskWarp = 0.42f;
        params.selfShadow = 0.62f;
        params.backLightStrength = 0.72f;
        params.plasmaContrast = 1.18f;
        params.lensStrength = 0.16f;
        params.ringStrength = 0.82f;
        params.lensAsymmetry = 0.78f;
        params.shadowSoftness = 0.16f;
        params.raySteps = 36;
        params.rayStepScale = 0.86f;
        params.massStrength = 1.05f;
    } else if (presetIndex == 3) {
        params.spin = 0.45f;
        params.diskInnerRadius = 22.0f;
        params.diskOuterRadius = 76.0f;
        params.diskInclination = 10.0f;
        params.temperatureInner = 0.82f;
        params.temperatureOuter = 0.16f;
        params.turbulence = 0.18f;
        params.rotationSpeed = 0.22f;
        params.dopplerBoost = 0.25f;
        params.diskIntensity = 1.05f;
        params.diskThickness = 0.65f;
        params.diskWarp = 0.05f;
        params.selfShadow = 0.25f;
        params.backLightStrength = 0.28f;
        params.plasmaContrast = 0.32f;
        params.lensStrength = 0.10f;
        params.ringStrength = 0.38f;
        params.lensAsymmetry = 0.12f;
        params.shadowSoftness = 0.18f;
        params.raySteps = 24;
        params.rayStepScale = 0.90f;
        params.massStrength = 0.72f;
    } else if (presetIndex == 4) {
        params.spin = 0.72f;
        params.diskInnerRadius = 18.5f;
        params.diskOuterRadius = 115.0f;
        params.diskInclination = 24.0f;
        params.temperatureInner = 1.28f;
        params.temperatureOuter = 0.32f;
        params.turbulence = 1.15f;
        params.rotationSpeed = 0.42f;
        params.dopplerBoost = 0.55f;
        params.diskIntensity = 1.9f;
        params.diskThickness = 5.6f;
        params.diskWarp = 1.0f;
        params.selfShadow = 0.78f;
        params.backLightStrength = 1.25f;
        params.plasmaContrast = 1.45f;
        params.lensStrength = 0.15f;
        params.ringStrength = 0.75f;
        params.lensAsymmetry = 0.45f;
        params.shadowSoftness = 0.22f;
        params.raySteps = 32;
        params.rayStepScale = 0.82f;
        params.massStrength = 1.0f;
    }
    return params;
}

glm::vec3 diskPoint(float radius, float angleRadians, float tiltDegrees) {
    glm::mat4 diskModel(1.0f);
    diskModel = glm::rotate(diskModel, glm::radians(tiltDegrees),
                            glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 local(std::cos(angleRadians) * radius,
                    0.0f,
                    std::sin(angleRadians) * radius);
    return glm::vec3(diskModel * glm::vec4(local, 1.0f));
}

void drawCallout(const Camera& camera,
                 float aspectRatio,
                 int width,
                 int height,
                 const glm::vec3& worldPos,
                 const ImVec2& offset,
                 ImU32 color,
                 const char* title,
                 const char* detail) {
    glm::vec2 uv;
    if (!projectToUv(camera, aspectRatio, worldPos, uv)) {
        return;
    }

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImVec2 anchor(uv.x * static_cast<float>(width),
                  (1.0f - uv.y) * static_cast<float>(height));
    ImVec2 textPos(anchor.x + offset.x, anchor.y + offset.y);
    std::string text(title);
    if (detail && detail[0] != '\0') {
        text += "\n";
        text += detail;
    }

    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 pad(7.0f, 5.0f);
    drawList->AddLine(anchor, textPos, color, 1.2f);
    drawList->AddCircleFilled(anchor, 3.0f, color, 16);
    drawList->AddRectFilled(ImVec2(textPos.x - pad.x, textPos.y - pad.y),
                            ImVec2(textPos.x + textSize.x + pad.x,
                                   textPos.y + textSize.y + pad.y),
                            IM_COL32(6, 8, 14, 185), 5.0f);
    drawList->AddRect(ImVec2(textPos.x - pad.x, textPos.y - pad.y),
                      ImVec2(textPos.x + textSize.x + pad.x,
                             textPos.y + textSize.y + pad.y),
                      IM_COL32(130, 160, 220, 105), 5.0f);
    drawList->AddText(textPos, color, text.c_str());
}

} // namespace

BlackHoleScene::BlackHoleScene(ResourceManager& resources)
    : m_resources(resources)
{
    logBlackHoleInit("constructor begin");
    logBlackHoleInit("loading horizon shader");
    m_horizonShader = &m_resources.getShader("assets/shaders/black_hole.vert",
                                             "assets/shaders/black_hole.frag");
    logBlackHoleInit("loading disk shader");
    m_diskShader = &m_resources.getShader("assets/shaders/accretion_disk.vert",
                                          "assets/shaders/accretion_disk.frag");
    logBlackHoleInit("loading config");
    loadConfig("assets/config/black_hole.ini");
    logBlackHoleInit("rebuilding disk mesh");
    rebuildDiskMesh();
    logBlackHoleInit("constructor complete");
}

void BlackHoleScene::loadConfig(const char* path) {
    std::ifstream file(Paths::resolve(path));
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::size_t comment = line.find_first_of("#;");
        if (comment != std::string::npos) {
            line = line.substr(0, comment);
        }

        std::size_t equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, equals));
        std::string valueText = trim(line.substr(equals + 1));
        float value = 0.0f;
        if (tryParseFloat(valueText, value)) {
            assignParam(m_params, key, value);
        }
    }
}

void BlackHoleScene::rebuildDiskMesh() {
    m_params.eventHorizonRadius = std::max(0.5f, m_params.eventHorizonRadius);
    m_params.diskInnerRadius = std::max(m_params.eventHorizonRadius * 1.08f,
                                        m_params.diskInnerRadius);
    m_params.diskOuterRadius = std::max(m_params.diskInnerRadius + 2.0f,
                                        m_params.diskOuterRadius);
    m_params.diskThickness = std::max(0.0f, m_params.diskThickness);
    m_params.diskWarp = std::max(0.0f, m_params.diskWarp);
    m_params.selfShadow = std::clamp(m_params.selfShadow, 0.0f, 1.0f);
    m_params.backLightStrength = std::max(0.0f, m_params.backLightStrength);
    m_params.plasmaContrast = std::max(0.0f, m_params.plasmaContrast);
    m_params.lensAsymmetry = std::clamp(m_params.lensAsymmetry, 0.0f, 1.0f);
    m_params.shadowSoftness = std::clamp(m_params.shadowSoftness, 0.02f, 0.6f);
    m_params.raySteps = std::clamp(m_params.raySteps, 8, 48);
    m_params.rayStepScale = std::clamp(m_params.rayStepScale, 0.35f, 1.5f);
    m_params.massStrength = std::clamp(m_params.massStrength, 0.0f, 3.0f);
    m_params.captureRadiusScale = std::clamp(m_params.captureRadiusScale,
                                             0.75f, 1.6f);

    constexpr int radialSegments = 32;
    constexpr int angularSegments = 192;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    vertices.reserve((radialSegments + 1) * (angularSegments + 1));
    indices.reserve(radialSegments * angularSegments * 6);

    for (int r = 0; r <= radialSegments; ++r) {
        float radialT = static_cast<float>(r) / static_cast<float>(radialSegments);
        float radius = m_params.diskInnerRadius +
                       radialT * (m_params.diskOuterRadius - m_params.diskInnerRadius);

        for (int a = 0; a <= angularSegments; ++a) {
            float angleT = static_cast<float>(a) / static_cast<float>(angularSegments);
            float angle = angleT * 6.28318530718f;

            Vertex vertex;
            vertex.position = glm::vec3(std::cos(angle) * radius,
                                        0.0f,
                                        std::sin(angle) * radius);
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vertex.texCoord = glm::vec2(radialT, angleT);
            vertices.push_back(vertex);
        }
    }

    for (int r = 0; r < radialSegments; ++r) {
        for (int a = 0; a < angularSegments; ++a) {
            unsigned int row = static_cast<unsigned int>(r * (angularSegments + 1));
            unsigned int nextRow = static_cast<unsigned int>((r + 1) * (angularSegments + 1));
            unsigned int i0 = row + static_cast<unsigned int>(a);
            unsigned int i1 = i0 + 1;
            unsigned int i2 = nextRow + static_cast<unsigned int>(a);
            unsigned int i3 = i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    m_diskMesh.upload(vertices, indices);
}

void BlackHoleScene::update(const Time& time) {
    m_diskPhase += time.getDeltaTime() * m_params.rotationSpeed;
}

void BlackHoleScene::draw(Camera& camera, float aspectRatio) {
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);
    glm::mat4 diskModel(1.0f);
    diskModel = glm::rotate(diskModel, glm::radians(m_params.diskInclination),
                            glm::vec3(1.0f, 0.0f, 0.0f));

    GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
    GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glm::vec3 diskNormal = glm::normalize(glm::vec3(diskModel *
        glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

    m_diskShader->use();
    m_diskShader->setMat4("uModel", diskModel);
    m_diskShader->setMat4("uView", view);
    m_diskShader->setMat4("uProjection", projection);
    m_diskShader->setVec3("uCameraPos", camera.getPosition());
    m_diskShader->setVec3("uDiskNormal", diskNormal);
    m_diskShader->setFloat("uTime", m_diskPhase);
    m_diskShader->setFloat("uInnerRadius", m_params.diskInnerRadius);
    m_diskShader->setFloat("uOuterRadius", m_params.diskOuterRadius);
    m_diskShader->setFloat("uSpin", m_params.spin);
    m_diskShader->setFloat("uTemperatureInner", m_params.temperatureInner);
    m_diskShader->setFloat("uTemperatureOuter", m_params.temperatureOuter);
    m_diskShader->setFloat("uTurbulence", m_params.turbulence);
    m_diskShader->setFloat("uDopplerBoost", m_params.dopplerBoost);
    m_diskShader->setFloat("uDiskIntensity", m_params.diskIntensity);
    m_diskShader->setFloat("uDiskThickness", m_params.diskThickness);
    m_diskShader->setFloat("uDiskWarp", m_params.diskWarp);
    m_diskShader->setFloat("uSelfShadow", m_params.selfShadow);
    m_diskShader->setFloat("uBackLightStrength", m_params.backLightStrength);
    m_diskShader->setFloat("uPlasmaContrast", m_params.plasmaContrast);
    m_diskShader->setInt("uDebugMode", m_debugMode);
    m_diskMesh.draw();

    glDepthMask(GL_TRUE);
    if (cullWasEnabled) glEnable(GL_CULL_FACE);
    else glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glm::mat4 horizonModel(1.0f);
    horizonModel = glm::scale(horizonModel,
                              glm::vec3(m_params.eventHorizonRadius));

    m_horizonShader->use();
    m_horizonShader->setMat4("uModel", horizonModel);
    m_horizonShader->setMat4("uView", view);
    m_horizonShader->setMat4("uProjection", projection);
    m_horizonShader->setFloat("uEventHorizonRadius",
                              m_params.eventHorizonRadius);
    m_horizonShader->setFloat("uPhotonSphereRadius",
                              m_params.photonSphereRadius);
    m_horizonShader->setInt("uDebugMode", m_debugMode);
    m_resources.getSphereMesh(ResourceManager::SphereLod::High).draw();

    if (blendWasEnabled) glEnable(GL_BLEND);
}

void BlackHoleScene::drawControls(Camera& camera) {
    ImGui::TextColored(ImVec4(0.78f, 0.58f, 1.0f, 1.0f), "Black Hole");
    ImGui::Separator();

    ImGui::Checkbox("Teaching Panel", &m_showTeachingPanel);
    ImGui::SameLine();
    ImGui::Checkbox("Labels", &m_showLabels);

    const char* paramPresets[] = {
        "Balanced",
        "Strong Lens",
        "Fast Kerr Doppler",
        "Thin Quiet Disk",
        "Thick Turbulent Disk"
    };
    ImGui::PushItemWidth(190);
    ImGui::Combo("Parameter Preset", &m_parameterPresetIndex,
                 paramPresets, 5);
    ImGui::SameLine();
    if (ImGui::Button("Apply##bhPreset")) {
        applyParameterPreset(m_parameterPresetIndex);
    }

    const char* debugPresets[] = {
        "Final",
        "Disk Temperature",
        "Disk Doppler",
        "Disk Alpha",
        "Near/Far Sides",
        "Ray Bend",
        "Photon Ring",
        "Shadow Mask",
        "Ray Closest"
    };
    if (ImGui::Combo("Debug Preset", &m_debugPresetIndex,
                     debugPresets, 9)) {
        applyDebugPreset(m_debugPresetIndex);
    }

    ImGui::TextDisabled("Camera Preset");
    if (ImGui::Button("Teaching", ImVec2(86, 0))) {
        applyCameraPreset(camera, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Edge", ImVec2(58, 0))) {
        applyCameraPreset(camera, 1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Top", ImVec2(54, 0))) {
        applyCameraPreset(camera, 2);
    }
    if (ImGui::Button("Photon Ring", ImVec2(112, 0))) {
        applyCameraPreset(camera, 3);
    }
    ImGui::SameLine();
    if (ImGui::Button("Wide Lens", ImVec2(96, 0))) {
        applyCameraPreset(camera, 4);
    }

    ImGui::Spacing();
    const char* debugModes[] = {"Final", "Temperature", "Doppler", "Alpha", "Near/Far"};
    ImGui::Combo("Disk Debug", &m_debugMode, debugModes, 5);
    ImGui::SliderFloat("Event Horizon", &m_params.eventHorizonRadius,
                       2.0f, 40.0f, "%.2f");
    ImGui::SliderFloat("Photon Sphere", &m_params.photonSphereRadius,
                       4.0f, 70.0f, "%.2f");
    ImGui::SliderFloat("Spin", &m_params.spin, -1.0f, 1.0f, "%.2f");

    bool diskChanged = false;
    diskChanged |= ImGui::SliderFloat("Disk Inner", &m_params.diskInnerRadius,
                                      4.0f, 80.0f, "%.2f");
    diskChanged |= ImGui::SliderFloat("Disk Outer", &m_params.diskOuterRadius,
                                      12.0f, 180.0f, "%.2f");
    diskChanged |= ImGui::SliderFloat("Disk Tilt", &m_params.diskInclination,
                                      -80.0f, 80.0f, "%.1f");
    ImGui::SliderFloat("Inner Heat", &m_params.temperatureInner,
                       0.2f, 2.5f, "%.2f");
    ImGui::SliderFloat("Outer Heat", &m_params.temperatureOuter,
                       0.0f, 1.5f, "%.2f");
    ImGui::SliderFloat("Turbulence", &m_params.turbulence,
                       0.0f, 1.5f, "%.2f");
    ImGui::SliderFloat("Rotation", &m_params.rotationSpeed,
                       -2.0f, 2.0f, "%.2f");
    ImGui::SliderFloat("Doppler", &m_params.dopplerBoost,
                       0.0f, 1.5f, "%.2f");
    ImGui::SliderFloat("Disk Intensity", &m_params.diskIntensity,
                       0.1f, 4.0f, "%.2f");
    ImGui::SliderFloat("Disk Thickness", &m_params.diskThickness,
                       0.0f, 10.0f, "%.2f");
    ImGui::SliderFloat("Disk Warp", &m_params.diskWarp,
                       0.0f, 1.5f, "%.2f");
    ImGui::SliderFloat("Self Shadow", &m_params.selfShadow,
                       0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Back Light", &m_params.backLightStrength,
                       0.0f, 2.0f, "%.2f");
    ImGui::SliderFloat("Plasma Contrast", &m_params.plasmaContrast,
                       0.0f, 2.0f, "%.2f");

    ImGui::Spacing();
    ImGui::Checkbox("Lens Pass", &m_lensEnabled);
    ImGui::Checkbox("Schwarzschild March", &m_params.rayMarchLensing);
    const char* lensDebugModes[] = {"Final", "Ray Bend", "Photon Ring",
                                    "Shadow", "Ray Closest"};
    ImGui::Combo("Lens Debug", &m_lensDebugMode, lensDebugModes, 5);
    ImGui::SliderFloat("Lens Strength", &m_params.lensStrength,
                       0.0f, 0.45f, "%.3f");
    ImGui::SliderFloat("Ring Strength", &m_params.ringStrength,
                       0.0f, 2.0f, "%.2f");
    ImGui::SliderFloat("Spin Asymmetry", &m_params.lensAsymmetry,
                       0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Shadow Softness", &m_params.shadowSoftness,
                       0.02f, 0.6f, "%.2f");
    ImGui::SliderInt("Ray Steps", &m_params.raySteps, 8, 48);
    ImGui::SliderFloat("Step Scale", &m_params.rayStepScale,
                       0.35f, 1.5f, "%.2f");
    ImGui::SliderFloat("Mass Strength", &m_params.massStrength,
                       0.0f, 3.0f, "%.2f");
    ImGui::SliderFloat("Capture Radius", &m_params.captureRadiusScale,
                       0.75f, 1.6f, "%.2f");
    ImGui::PopItemWidth();

    if (diskChanged) {
        rebuildDiskMesh();
    }
}

void BlackHoleScene::applyParameterPreset(int presetIndex) {
    m_params = presetParams(presetIndex);
    rebuildDiskMesh();
}

void BlackHoleScene::applyDebugPreset(int presetIndex) {
    m_debugPresetIndex = presetIndex;
    m_debugMode = 0;
    m_lensDebugMode = 0;

    if (presetIndex >= 1 && presetIndex <= 4) {
        m_debugMode = presetIndex;
    } else if (presetIndex >= 5) {
        m_lensEnabled = true;
        m_lensDebugMode = presetIndex - 4;
    }
}

void BlackHoleScene::applyCameraPreset(Camera& camera, int presetIndex) const {
    if (presetIndex == 1) {
        camera.setPosition(glm::vec3(0.0f, 17.0f, 152.0f));
        camera.setFov(42.0f);
    } else if (presetIndex == 2) {
        camera.setPosition(glm::vec3(0.0f, 185.0f, 0.6f));
        camera.setFov(48.0f);
    } else if (presetIndex == 3) {
        camera.setPosition(glm::vec3(-44.0f, 18.0f, 66.0f));
        camera.setFov(35.0f);
    } else if (presetIndex == 4) {
        camera.setPosition(glm::vec3(0.0f, 62.0f, 240.0f));
        camera.setFov(52.0f);
    } else {
        resetCamera(camera);
        return;
    }
    camera.lookAt(glm::vec3(0.0f));
}

void BlackHoleScene::drawTeachingOverlay(const Camera& camera,
                                         float aspectRatio,
                                         int width,
                                         int height) const {
    if (m_showLabels) {
        drawCallout(camera, aspectRatio, width, height,
                    glm::vec3(m_params.eventHorizonRadius * 0.65f,
                              m_params.eventHorizonRadius * 0.45f,
                              0.0f),
                    ImVec2(18.0f, -48.0f),
                    IM_COL32(235, 205, 120, 245),
                    "Event horizon",
                    "No light escapes");
        drawCallout(camera, aspectRatio, width, height,
                    glm::vec3(m_params.photonSphereRadius, 0.0f, 0.0f),
                    ImVec2(20.0f, 18.0f),
                    IM_COL32(160, 200, 255, 235),
                    "Photon sphere",
                    "Bright ring region");
        drawCallout(camera, aspectRatio, width, height,
                    diskPoint(m_params.diskInnerRadius, 0.82f,
                              m_params.diskInclination),
                    ImVec2(-154.0f, -34.0f),
                    IM_COL32(255, 170, 90, 238),
                    "Inner disk",
                    "Hottest plasma");
        drawCallout(camera, aspectRatio, width, height,
                    diskPoint(m_params.diskOuterRadius * 0.82f, 2.55f,
                              m_params.diskInclination),
                    ImVec2(18.0f, 24.0f),
                    IM_COL32(255, 120, 90, 230),
                    "Accretion disk",
                    "Turbulent gas flow");
        drawCallout(camera, aspectRatio, width, height,
                    diskPoint(m_params.diskOuterRadius * 0.62f, -0.55f,
                              m_params.diskInclination),
                    ImVec2(18.0f, -34.0f),
                    IM_COL32(120, 185, 255, 232),
                    "Doppler side",
                    "Approaching gas shifts blue");
    }

    if (!m_showTeachingPanel) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width) - 360.0f, 70.0f),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(340.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.80f);
    if (ImGui::Begin("Black Hole Guide",
                     nullptr,
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.36f, 1.0f),
                           "Teaching View");
        ImGui::Separator();
        ImGui::TextWrapped("This scene is a real-time approximation: a dark event horizon, a photon-ring glow, a turbulent accretion disk, and a low-cost Schwarzschild-style lensing march.");
        ImGui::Spacing();
        ImGui::Text("Event horizon radius: %.2f", m_params.eventHorizonRadius);
        ImGui::Text("Photon sphere radius: %.2f", m_params.photonSphereRadius);
        ImGui::Text("Spin: %.2f", m_params.spin);
        ImGui::Text("Disk: %.1f - %.1f", m_params.diskInnerRadius,
                    m_params.diskOuterRadius);
        ImGui::Text("Lens strength: %.3f", m_params.lensStrength);
        ImGui::Text("Ray steps: %d", m_params.raySteps);
        ImGui::Text("Camera distance: %.1f",
                    glm::length(camera.getPosition()));
        ImGui::Spacing();
        ImGui::TextWrapped("Use Debug Preset to isolate temperature, Doppler shift, alpha, ray bend, photon ring, shadow mask, or closest ray approach.");
    }
    ImGui::End();
}

Renderer::LensSettings BlackHoleScene::lensSettings(const Camera& camera,
                                                    float aspectRatio) const {
    Renderer::LensSettings settings;
    if (!m_lensEnabled) {
        return settings;
    }

    glm::vec2 centerUv;
    if (!projectToUv(camera, aspectRatio, glm::vec3(0.0f), centerUv)) {
        return settings;
    }

    float eventRadius = projectedRadius(camera, aspectRatio,
                                        m_params.eventHorizonRadius,
                                        centerUv);
    float photonRadius = projectedRadius(camera, aspectRatio,
                                         m_params.photonSphereRadius,
                                         centerUv);
    if (eventRadius <= 0.0001f || photonRadius <= eventRadius) {
        return settings;
    }

    settings.enabled = true;
    settings.rayMarchEnabled = m_params.rayMarchLensing;
    settings.center = centerUv;
    settings.viewProjection = camera.getProjectionMatrix(aspectRatio) *
                              camera.getViewMatrix();
    settings.inverseViewProjection = glm::inverse(settings.viewProjection);
    settings.cameraPosition = camera.getPosition();
    settings.blackHoleCenter = glm::vec3(0.0f);
    settings.eventHorizonRadius = eventRadius;
    settings.photonSphereRadius = photonRadius;
    settings.worldEventHorizonRadius = m_params.eventHorizonRadius;
    settings.worldPhotonSphereRadius = m_params.photonSphereRadius;
    settings.strength = m_params.lensStrength;
    settings.ringStrength = m_params.ringStrength;
    settings.spin = m_params.spin;
    settings.asymmetry = m_params.lensAsymmetry;
    settings.shadowSoftness = m_params.shadowSoftness;
    settings.stepScale = m_params.rayStepScale;
    settings.massStrength = m_params.massStrength;
    settings.captureRadiusScale = m_params.captureRadiusScale;
    settings.aspectRatio = aspectRatio;
    settings.raySteps = m_params.raySteps;
    settings.debugMode = m_lensDebugMode;
    return settings;
}

void BlackHoleScene::resetCamera(Camera& camera) const {
    camera.setPosition(glm::vec3(0.0f, 38.0f, 145.0f));
    camera.setFov(45.0f);
    camera.lookAt(glm::vec3(0.0f));
}
