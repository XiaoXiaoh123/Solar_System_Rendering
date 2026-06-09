#include "src/core/Window.h"
#include "src/core/Camera.h"
#include "src/core/Input.h"
#include "src/core/Time.h"
#include "src/render/ResourceManager.h"
#include "src/render/Renderer.h"
#include "src/render/Skybox.h"
#include "src/scene/SceneCatalog.h"
#include "src/scene/SolarSystem.h"
#include "src/utils/Constants.h"
#include "src/utils/Paths.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl3.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>
#include <ctime>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

static void writeLog(const std::string& msg, const char* filename = "SolarSystem.log") {
    try {
        std::ofstream log(Paths::writableLogPath(filename), std::ios::app);
        log << msg << '\n';
    } catch (...) {}
}

static void showError(const std::string& msg) {
    std::cerr << "Fatal error: " << msg << std::endl;
    writeLog("Fatal error: " + msg, "SolarSystem_error.log");
#ifdef _WIN32
    MessageBoxA(nullptr, msg.c_str(), "Solar System - Fatal Error",
                MB_OK | MB_ICONERROR);
#endif
}

struct SimDate {
    int year = 2000;
    int month = 1;
    int day = 1;
};

static bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int daysInMonth(int year, int month) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) return 29;
    return days[month - 1];
}

static SimDate dateFromEpochDays(int daysSinceEpoch) {
    SimDate date;
    while (daysSinceEpoch >= (isLeapYear(date.year) ? 366 : 365)) {
        daysSinceEpoch -= isLeapYear(date.year) ? 366 : 365;
        ++date.year;
    }

    while (daysSinceEpoch >= daysInMonth(date.year, date.month)) {
        daysSinceEpoch -= daysInMonth(date.year, date.month);
        ++date.month;
    }

    date.day += daysSinceEpoch;
    return date;
}

static std::string formatDate(const SimDate& date) {
    std::ostringstream out;
    out << std::setfill('0') << std::setw(4) << date.year << "-"
        << std::setw(2) << date.month << "-"
        << std::setw(2) << date.day;
    return out.str();
}

static std::string fixedNumber(float value, int precision) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

static std::string formatAu(float au) {
    if (au <= 0.0f) return "0 AU";
    if (au < 0.01f) return fixedNumber(au, 5) + " AU";
    if (au < 1.0f) return fixedNumber(au, 3) + " AU";
    return fixedNumber(au, 2) + " AU";
}

static float solveEccentricAnomalyForInfo(float meanAnomaly, float eccentricity) {
    float e = std::clamp(eccentricity, 0.0f, 0.95f);
    float E = meanAnomaly;
    for (int i = 0; i < 6; ++i) {
        float f = E - e * std::sin(E) - meanAnomaly;
        float fp = 1.0f - e * std::cos(E);
        E -= f / fp;
    }
    return E;
}

static float currentDistanceAu(const CelestialBody* body) {
    if (!body) return 0.0f;
    const CelestialParams& p = body->getParams();
    if (p.semiMajorAxisAU <= 0.0f) return 0.0f;
    float E = solveEccentricAnomalyForInfo(body->getOrbitAngle(), p.orbit.eccentricity);
    return p.semiMajorAxisAU * (1.0f - p.orbit.eccentricity * std::cos(E));
}

static float followDistanceFor(const CelestialBody* body) {
    if (!body) return 40.0f;
    float radius = std::max(body->getRenderRadius(), 0.25f);
    return std::clamp(radius * 7.0f, 8.0f, 260.0f);
}

static glm::vec3 defaultFollowOffset(const CelestialBody* body) {
    glm::vec3 direction = glm::normalize(glm::vec3(0.0f, 0.32f, 1.0f));
    return direction * followDistanceFor(body);
}

static glm::vec3 rotateFollowOffset(const glm::vec3& offset,
                                    const glm::vec2& mouseDelta) {
    if (glm::length(offset) < 0.001f) return offset;

    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    float yaw = -mouseDelta.x * Constants::CAM_SENSITIVITY;
    float pitch = -mouseDelta.y * Constants::CAM_SENSITIVITY;

    glm::vec3 rotated = glm::vec3(
        glm::rotate(glm::mat4(1.0f), glm::radians(yaw), worldUp) *
        glm::vec4(offset, 0.0f));

    glm::vec3 viewDir = glm::normalize(-rotated);
    glm::vec3 right = glm::cross(viewDir, worldUp);
    if (glm::length(right) < 0.001f) {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    } else {
        right = glm::normalize(right);
    }

    glm::vec3 pitched = glm::vec3(
        glm::rotate(glm::mat4(1.0f), glm::radians(pitch), right) *
        glm::vec4(rotated, 0.0f));
    glm::vec3 pitchedDir = glm::normalize(pitched);
    if (std::abs(pitchedDir.y) > 0.94f) {
        return rotated;
    }

    return pitched;
}

static bool projectWorldToScreen(const glm::vec3& worldPos,
                                 const Camera& camera,
                                 float aspectRatio,
                                 int width,
                                 int height,
                                 ImVec2& screenPos,
                                 float* depth = nullptr) {
    glm::vec4 clip = camera.getProjectionMatrix(aspectRatio) *
                     camera.getViewMatrix() *
                     glm::vec4(worldPos, 1.0f);
    if (clip.w <= 0.001f) return false;

    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    if (ndc.z < -1.0f || ndc.z > 1.0f) return false;
    screenPos.x = (ndc.x * 0.5f + 0.5f) * static_cast<float>(width);
    screenPos.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * static_cast<float>(height);
    if (depth) *depth = ndc.z;
    return ndc.x >= -1.2f && ndc.x <= 1.2f && ndc.y >= -1.2f && ndc.y <= 1.2f;
}

static void drawOutlinedText(ImDrawList* drawList,
                             ImVec2 pos,
                             ImU32 color,
                             const std::string& text,
                             float fontSize = 0.0f) {
    ImU32 shadow = IM_COL32(0, 0, 0, 210);
    ImFont* font = ImGui::GetFont();
    drawList->AddText(font, fontSize, ImVec2(pos.x + 1.0f, pos.y + 1.0f),
                      shadow, text.c_str());
    drawList->AddText(font, fontSize, pos, color, text.c_str());
}

static CelestialBody* pickBodyAtScreen(SolarSystem& solarSystem,
                                       const Camera& camera,
                                       float aspectRatio,
                                       int width,
                                       int height,
                                       const glm::vec2& mousePos) {
    CelestialBody* best = nullptr;
    float bestScore = std::numeric_limits<float>::max();

    for (CelestialBody* body : solarSystem.getBodies()) {
        ImVec2 screen;
        float depth = 0.0f;
        if (!projectWorldToScreen(body->getWorldPosition(), camera, aspectRatio,
                                  width, height, screen, &depth)) {
            continue;
        }

        float cameraDistance = glm::length(camera.getPosition() - body->getWorldPosition());
        float angular = body->getRenderRadius() /
                        std::max(cameraDistance, 0.001f);
        float radiusPixels = angular /
            std::tan(glm::radians(camera.getFov()) * 0.5f) *
            static_cast<float>(height) * 0.5f;
        float pickRadius = std::clamp(radiusPixels, 10.0f, 80.0f);
        float dx = mousePos.x - screen.x;
        float dy = mousePos.y - screen.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= pickRadius) {
            float score = dist + depth * 8.0f;
            if (score < bestScore) {
                bestScore = score;
                best = body;
            }
        }
    }

    return best;
}

static void drawSceneLabels(SolarSystem& solarSystem,
                            CelestialBody* selectedBody,
                            const Camera& camera,
                            float aspectRatio,
                            int width,
                            int height) {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    for (CelestialBody* body : solarSystem.getBodies()) {
        glm::vec3 labelPos = body->getWorldPosition() +
                             glm::vec3(0.0f, body->getRenderRadius() * 1.25f, 0.0f);
        ImVec2 screen;
        if (!projectWorldToScreen(labelPos, camera, aspectRatio, width, height, screen)) {
            continue;
        }

        bool selected = body == selectedBody;
        ImU32 bodyColor = selected ? IM_COL32(255, 220, 90, 255)
                                   : IM_COL32(220, 235, 255, 230);
        if (selected) {
            drawList->AddCircle(ImVec2(screen.x - 8.0f, screen.y + 7.0f),
                                4.0f, bodyColor, 16, 1.5f);
        }

        drawOutlinedText(drawList, screen, bodyColor, body->getName(), 15.0f);

        const CelestialParams& params = body->getParams();
        if (params.semiMajorAxisAU > 0.0f) {
            std::string orbitLabel = "orbit " + formatAu(params.semiMajorAxisAU);
            drawOutlinedText(drawList, ImVec2(screen.x, screen.y + 16.0f),
                             IM_COL32(150, 190, 230, 210), orbitLabel, 12.0f);
        }
    }
}

static void drawBodyInfoPanel(CelestialBody* selectedBody,
                              const Camera& camera,
                              int width,
                              bool* open) {
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width) - 330.0f, 70.0f),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(310.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.78f);

    if (ImGui::Begin("Body Info", open,
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!selectedBody) {
            ImGui::TextDisabled("No body selected");
            ImGui::End();
            return;
        }

        const CelestialParams& p = selectedBody->getParams();
        float cameraDistance = glm::length(camera.getPosition() -
                                           selectedBody->getWorldPosition());
        ImGui::TextColored(ImVec4(1.0f, 0.88f, 0.35f, 1.0f),
                           "%s", selectedBody->getName().c_str());
        ImGui::Separator();
        if (p.realRadiusKm > 0.0f) {
            ImGui::Text("Radius: %.1f km", p.realRadiusKm);
        }
        ImGui::Text("Render radius: %.3f", selectedBody->getRenderRadius());
        if (p.semiMajorAxisAU > 0.0f) {
            ImGui::Text("Current distance: %s", formatAu(currentDistanceAu(selectedBody)).c_str());
            ImGui::Text("Semi-major axis: %s", formatAu(p.semiMajorAxisAU).c_str());
            ImGui::Text("Eccentricity: %.4f", p.orbit.eccentricity);
            ImGui::Text("Inclination: %.3f deg", p.orbit.inclination);
        }
        if (p.orbitPeriodDays > 0.0f) {
            ImGui::Text("Orbit period: %.2f days", p.orbitPeriodDays);
        }
        if (std::abs(p.rotationPeriodHours) > 0.001f) {
            ImGui::Text("Rotation period: %.2f h", p.rotationPeriodHours);
        }
        ImGui::Text("Camera distance: %.2f", cameraDistance);
    }
    ImGui::End();
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    writeLog("---- SolarSystem startup ----");
    writeLog("cwd: " + Paths::currentWorkingDirectory());
    writeLog("exe dir: " + Paths::executableDirectory());

    try {
        Window window(Constants::DEFAULT_WIDTH, Constants::DEFAULT_HEIGHT,
                      "Solar System");

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GPU:    " << glGetString(GL_RENDERER) << std::endl;
        writeLog(std::string("OpenGL: ") +
                 reinterpret_cast<const char*>(glGetString(GL_VERSION)));
        writeLog(std::string("GPU: ") +
                 reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.swapBuffers();
        window.pollEvents();

        Input input(window.getHandle());

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = nullptr;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window.getHandle(), true);
        ImGui_ImplOpenGL3_Init("#version 460");

        Camera       camera(glm::vec3(0.0f, 350.0f, 800.0f), -90.0f, -25.0f);
        Time         time;
        ResourceManager resources;
        Renderer     renderer(resources);
        SolarSystem  solarSystem(resources);
        Skybox       skybox(resources);
        try {
            skybox.load("assets/textures/skybox/starmap_2020_4k.png");
        } catch (const std::exception& e) {
            std::cerr << "Skybox: " << e.what() << std::endl;
        }

        // --- Settings state ---
        bool  showSettings     = false;
        bool  prevShowSettings = false;
        float timeScale        = Constants::DEFAULT_TIME_SCALE;
        float savedTimeScale   = timeScale;
        bool  escWasDown       = false;
        bool  f5WasDown        = false;
        bool  leftWasDown      = false;
        bool  showInfoOverlay  = false;
        bool  cursorDisabled   = true;
        bool  wantsQuit        = false;
        bool  cameraFollow     = false;
        SceneId activeScene    = SceneCatalog::defaultScene();
        CelestialBody* selectedBody = solarSystem.getEarth();
        glm::vec3 followOffset = defaultFollowOffset(selectedBody);

        float ambientStrength  = Constants::AMBIENT_STRENGTH;
        Renderer::PostProcessSettings postProcessSettings;
        std::string shaderReloadStatus;
        auto reloadShaders = [&]() {
            std::string reloadError;
            if (resources.reloadShaders(&reloadError)) {
                shaderReloadStatus = "Shaders reloaded";
            } else {
                shaderReloadStatus = "Reload failed";
                if (!reloadError.empty()) {
                    writeLog(reloadError, "SolarSystem_shader_reload.log");
                }
            }
        };
        auto applyCursorState = [&](bool disabled) {
            if (cursorDisabled == disabled) return;
            window.setCursorDisabled(disabled);
            input.resetMouse();
            cursorDisabled = disabled;
        };
        auto focusBody = [&](CelestialBody* body, bool follow) {
            if (!body) return;
            selectedBody = body;
            cameraFollow = follow;
            followOffset = defaultFollowOffset(body);
            glm::vec3 target = body->getWorldPosition();
            camera.setPosition(target + followOffset);
            camera.lookAt(target);
        };
        auto followBody = [&](CelestialBody* body) {
            focusBody(body, true);
        };

        time.setTimeScale(timeScale);

        // --- Main loop ---
        while (!window.shouldClose() && !wantsQuit) {
            time.tick();
            input.update();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // --- ESC toggles settings panel ---
            bool escDown = glfwGetKey(window.getHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS;
            if (escDown && !escWasDown) {
                showSettings = !showSettings;
            }
            escWasDown = escDown;

            bool f5Down = glfwGetKey(window.getHandle(), GLFW_KEY_F5) == GLFW_PRESS;
            if (f5Down && !f5WasDown) {
                reloadShaders();
            }
            f5WasDown = f5Down;
            bool leftDown =
                glfwGetMouseButton(window.getHandle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

            // --- Top bar ---
            {
                ImGui::SetNextWindowPos(ImVec2(10, 10));
                ImGui::SetNextWindowBgAlpha(0.35f);
                if (ImGui::Begin("##topbar", nullptr,
                                 ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoFocusOnAppearing)) {
                    if (ImGui::Button(showSettings ? "Hide Panel" : "Settings")) {
                        showSettings = !showSettings;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(showInfoOverlay ? "Hide Info" : "Info")) {
                        showInfoOverlay = !showInfoOverlay;
                    }
                    if (cameraFollow && selectedBody) {
                        ImGui::SameLine();
                        if (ImGui::Button("Stop Follow")) {
                            cameraFollow = false;
                        }
                        ImGui::SameLine();
                        ImGui::Text("Following: %s", selectedBody->getName().c_str());
                    }
                    ImGui::SameLine();

                    float simDaysFloat = time.getElapsedTime() * Constants::DAYS_PER_SECOND;
                    int simDays = std::max(0, static_cast<int>(std::floor(simDaysFloat)));
                    SimDate simDate = dateFromEpochDays(simDays);
                    const char* paused = (timeScale == 0.0f) ? "[PAUSED]" : "";
                    ImGui::Text("FPS: %.0f  dt: %.4f  scale: %.3f %s",
                                io.Framerate, time.getRawDeltaTime(),
                                timeScale, paused);
                    ImGui::Text("Date: %s  +%.1f days",
                                formatDate(simDate).c_str(), simDaysFloat);
                }
                ImGui::End();
            }

            // --- Settings panel ---
            if (showSettings) {
                ImGui::SetNextWindowPos(ImVec2(10, 60), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Once);

                if (ImGui::Begin("Control Panel", &showSettings,
                                 ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_AlwaysAutoResize)) {

                    ImGui::TextColored(ImVec4(0.85f, 0.72f, 1.0f, 1.0f), "Scene");
                    ImGui::Separator();
                    const SceneDescriptor& activeSceneDesc =
                        SceneCatalog::descriptor(activeScene);
                    ImGui::PushItemWidth(220);
                    if (ImGui::BeginCombo("Scene Select", activeSceneDesc.name)) {
                        for (const SceneDescriptor& scene : SceneCatalog::entries()) {
                            bool selected = scene.id == activeScene;
                            ImGui::BeginDisabled(!scene.available);
                            if (ImGui::Selectable(scene.name, selected) && scene.available) {
                                activeScene = scene.id;
                                cameraFollow = false;
                                showInfoOverlay = false;
                            }
                            ImGui::EndDisabled();
                            if (selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::PopItemWidth();
                    ImGui::Spacing();

                    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Time Control");
                    ImGui::Separator();

                    float orbitSec = (timeScale > 0.001f)
                        ? Constants::EARTH_ORBIT_PERIOD / (Constants::DAYS_PER_SECOND * timeScale)
                        : INFINITY;
                    ImGui::Text("1s = %.0f days | Earth orbit: ~%.1fs",
                                Constants::DAYS_PER_SECOND * timeScale, orbitSec);

                    // Play / Pause
                    if (timeScale == 0.0f) {
                        if (ImGui::Button("  Play  ", ImVec2(100, 28))) {
                            timeScale = (savedTimeScale > 0.0f) ? savedTimeScale : 1.0f;
                            savedTimeScale = timeScale;
                            time.setTimeScale(timeScale);
                        }
                    } else {
                        if (ImGui::Button("  Pause  ", ImVec2(100, 28))) {
                            savedTimeScale = timeScale;
                            timeScale      = 0.0f;
                            time.setTimeScale(0.0f);
                        }
                    }

                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    bool sliderMoved = ImGui::SliderFloat("##tsSlider", &timeScale,
                                                          0.0f, Constants::MAX_TIME_SCALE, "%.3f");
                    // Apply immediately while dragging
                    if (sliderMoved) {
                        time.setTimeScale(timeScale);
                        if (timeScale > 0.0f) savedTimeScale = timeScale;
                    }
                    ImGui::EndGroup();

                    // Verify time scale was applied
                    ImGui::TextDisabled("Time::getScale() = %.3f", time.getTimeScale());

                    // Presets
                    auto presetBtn = [&](const char* label, float val) {
                        if (ImGui::Button(label, ImVec2(40, 0))) {
                            timeScale = val;
                            time.setTimeScale(val);
                            if (val > 0.0f) savedTimeScale = val;
                        }
                    };
                    ImGui::Spacing();
                    presetBtn("0",    0.0f);  ImGui::SameLine();
                    presetBtn("0.1x", 0.1f);  ImGui::SameLine();
                    presetBtn("0.5x", 0.5f);  ImGui::SameLine();
                    presetBtn("1x",   1.0f);   ImGui::SameLine();
                    presetBtn("5x",   5.0f);   ImGui::SameLine();
                    presetBtn("10x",  10.0f);

                    ImGui::Spacing();
                    const char* scaleModes[] = {"Artistic", "Real", "Logarithmic"};
                    int scaleModeIndex = static_cast<int>(solarSystem.getScaleMode());
                    ImGui::PushItemWidth(180);
                    if (ImGui::Combo("Scale Mode", &scaleModeIndex, scaleModes, 3)) {
                        solarSystem.setScaleMode(static_cast<ScaleMode>(scaleModeIndex));
                    }
                    ImGui::PopItemWidth();

                    ImGui::Spacing();

                    // ---- Lighting ----
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.4f, 1.0f), "Lighting");
                    ImGui::Separator();
                    ImGui::SliderFloat("Ambient", &ambientStrength, 0.0f, 2.0f, "%.3f");
                    const char* debugModes[] = {"Lit", "Raw Texture", "UV", "Normals"};
                    int debugMode = solarSystem.getDebugMode();
                    ImGui::PushItemWidth(180);
                    if (ImGui::Combo("Debug View", &debugMode, debugModes, 4)) {
                        solarSystem.setDebugMode(debugMode);
                    }
                    ImGui::PopItemWidth();

                    bool showAtmosphere = solarSystem.getShowAtmosphere();
                    if (ImGui::Checkbox("Atmospheric Scattering", &showAtmosphere)) {
                        solarSystem.setShowAtmosphere(showAtmosphere);
                    }
                    AtmosphereTuning& atmosphere = solarSystem.getAtmosphereTuning();
                    ImGui::PushItemWidth(180);
                    ImGui::SliderFloat("Atmo Intensity", &atmosphere.intensity, 0.0f, 2.0f, "%.2f");
                    ImGui::SliderFloat("Edge Glow", &atmosphere.edgeStrength, 0.0f, 2.5f, "%.2f");
                    ImGui::SliderFloat("Sunset Band", &atmosphere.sunsetStrength, 0.0f, 2.5f, "%.2f");
                    ImGui::SliderFloat("Backscatter", &atmosphere.backscatterStrength, 0.0f, 2.5f, "%.2f");
                    ImGui::SliderFloat("Terminator", &atmosphere.terminatorWidth, 0.05f, 0.65f, "%.2f");
                    ImGui::PopItemWidth();

                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.25f, 1.0f), "Post FX");
                    ImGui::Separator();
                    ImGui::Checkbox("Sun Bloom / HDR", &postProcessSettings.bloomEnabled);
                    ImGui::PushItemWidth(180);
                    ImGui::SliderFloat("Exposure", &postProcessSettings.exposure, 0.35f, 2.25f, "%.2f");
                    ImGui::SliderFloat("Bloom Threshold", &postProcessSettings.bloomThreshold,
                                       0.55f, 3.0f, "%.2f");
                    ImGui::SliderFloat("Bloom Strength", &postProcessSettings.bloomStrength,
                                       0.0f, 2.0f, "%.2f");
                    ImGui::SliderInt("Bloom Radius", &postProcessSettings.blurPasses, 0, 12);
                    ImGui::PopItemWidth();

                    if (ImGui::Button("Reload Shaders", ImVec2(140, 0))) {
                        reloadShaders();
                    }
                    if (!shaderReloadStatus.empty()) {
                        ImGui::SameLine();
                        ImGui::TextDisabled("%s", shaderReloadStatus.c_str());
                    }

                    ImGui::Spacing();

                    // ---- Planets ----
                    ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "Planets");
                    ImGui::Separator();

                    auto planetSlider = [&](const char* name, CelestialBody* body) {
                        float mult = body->getRotationSpeedMultiplier();
                        float base = body->getBaseRotationSpeed();
                        ImGui::PushID(body);
                        ImGui::AlignTextToFramePadding();
                        if (selectedBody == body) {
                            ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.28f, 1.0f), "%s", name);
                        } else {
                            ImGui::Text("%s", name);
                        }
                        ImGui::SameLine(92);
                        ImGui::PushItemWidth(118);
                        if (ImGui::SliderFloat("##rot", &mult, 0.0f, 10.0f, "%.2fx")) {
                            body->setRotationSpeedMultiplier(mult);
                        }
                        ImGui::PopItemWidth();
                        ImGui::SameLine();
                        if (ImGui::Button("Select and Follow", ImVec2(132, 0))) {
                            showInfoOverlay = true;
                            followBody(body);
                        }
                        ImGui::SameLine();
                        ImGui::TextDisabled("%.2f", base * mult);
                        ImGui::PopID();
                    };

                    if (auto* sun = solarSystem.getSun()) {
                        planetSlider(sun->getName().c_str(), sun);
                    }
                    for (auto& planet : solarSystem.getPlanets()) {
                        planetSlider(planet->getName().c_str(), planet.get());
                    }
                    if (auto* moon = solarSystem.getMoon()) {
                        planetSlider(moon->getName().c_str(), moon);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::SetCursorPosX(100);
                    if (ImGui::Button("  Quit Program  ", ImVec2(120, 32))) {
                        wantsQuit = true;
                    }
                    ImGui::Spacing();
                    ImGui::TextDisabled("ESC to close");
                }
                ImGui::End();
            }

            // --- Detect showSettings changes (MUST be after panel render) ---
            // Catches ESC toggle, Hide/Settings button, and ImGui X button
            if (showSettings && !prevShowSettings) {
                savedTimeScale = timeScale;
                timeScale      = 0.0f;
                time.setTimeScale(0.0f);
            } else if (!showSettings && prevShowSettings) {
                timeScale = savedTimeScale;
                time.setTimeScale(savedTimeScale);
            }
            prevShowSettings = showSettings;
            applyCursorState(!showSettings && !showInfoOverlay);

            // --- Input: ONLY when panel is closed ---
            if (!showSettings) {
                if (!io.WantCaptureKeyboard) {
                    if (input.isKeyDown(GLFW_KEY_PERIOD)) {
                        timeScale = std::min(timeScale + time.getRawDeltaTime() * 2.0f,
                                             Constants::MAX_TIME_SCALE);
                        time.setTimeScale(timeScale);
                        savedTimeScale = timeScale;
                    }
                    if (input.isKeyDown(GLFW_KEY_COMMA)) {
                        timeScale = std::max(timeScale - time.getRawDeltaTime() * 2.0f, 0.0f);
                        time.setTimeScale(timeScale);
                        if (timeScale > 0.0f) savedTimeScale = timeScale;
                    }
                    if (input.isKeyDown(GLFW_KEY_SPACE)) {
                        if (timeScale > 0.0f) {
                            savedTimeScale = timeScale;
                            timeScale      = 0.0f;
                        } else {
                            timeScale = (savedTimeScale > 0.0f) ? savedTimeScale : 1.0f;
                            savedTimeScale = timeScale;
                        }
                        time.setTimeScale(timeScale);
                    }

                    if (!cameraFollow) {
                        float camSpeed = Constants::CAM_SPEED;
                        if (input.isKeyDown(GLFW_KEY_LEFT_SHIFT))
                            camSpeed = Constants::CAM_SPEED_FAST;
                        if (input.isKeyDown(GLFW_KEY_W)) camera.moveForward( camSpeed * time.getRawDeltaTime());
                        if (input.isKeyDown(GLFW_KEY_S)) camera.moveForward(-camSpeed * time.getRawDeltaTime());
                        if (input.isKeyDown(GLFW_KEY_A)) camera.moveRight(  -camSpeed * time.getRawDeltaTime());
                        if (input.isKeyDown(GLFW_KEY_D)) camera.moveRight(   camSpeed * time.getRawDeltaTime());
                        if (input.isKeyDown(GLFW_KEY_Q)) camera.moveUp(     -camSpeed * time.getRawDeltaTime());
                        if (input.isKeyDown(GLFW_KEY_E)) camera.moveUp(      camSpeed * time.getRawDeltaTime());
                    }
                }

                if (!io.WantCaptureMouse && !leftDown) {
                    glm::vec2 mouseDelta = input.getMouseDelta();
                    if (cameraFollow && selectedBody) {
                        followOffset = rotateFollowOffset(followOffset, mouseDelta);
                        camera.setPosition(selectedBody->getWorldPosition() + followOffset);
                        camera.lookAt(selectedBody->getWorldPosition());
                    } else {
                        camera.rotate(mouseDelta.x * Constants::CAM_SENSITIVITY,
                                     -mouseDelta.y * Constants::CAM_SENSITIVITY);
                    }
                    float scroll = io.MouseWheel;
                    if (scroll != 0.0f) {
                        if (cameraFollow && selectedBody) {
                            float scale = std::pow(0.88f, scroll);
                            float minDistance = std::max(selectedBody->getRenderRadius() * 2.2f, 2.0f);
                            float maxDistance = std::max(selectedBody->getRenderRadius() * 80.0f, 80.0f);
                            float distance = glm::length(followOffset);
                            float newDistance = std::clamp(distance * scale, minDistance, maxDistance);
                            followOffset = glm::normalize(followOffset) * newDistance;
                        } else {
                            camera.zoom(scroll * 2.0f);
                        }
                    }
                }
            }

            // --- Update & Render ---
            solarSystem.update(time);
            solarSystem.setAmbientStrength(ambientStrength);

            int framebufferWidth = window.getWidth();
            int framebufferHeight = window.getHeight();
            float aspectRatio = window.getAspectRatio();

            if (showInfoOverlay && !showSettings &&
                leftDown && !leftWasDown && !io.WantCaptureMouse) {
                if (CelestialBody* picked = pickBodyAtScreen(solarSystem, camera, aspectRatio,
                                                             framebufferWidth, framebufferHeight,
                                                             input.getMousePosition())) {
                    followBody(picked);
                }
            }

            if (cameraFollow && selectedBody) {
                glm::vec3 target = selectedBody->getWorldPosition();
                camera.setPosition(target + followOffset);
                camera.lookAt(target);
            }
            leftWasDown = leftDown;

            renderer.beginScene(framebufferWidth, framebufferHeight);
            renderer.clear();
            renderer.setViewport(framebufferWidth, framebufferHeight);
            renderer.drawSkybox(camera, skybox, aspectRatio);
            renderer.drawSolarSystem(solarSystem, camera, aspectRatio);
            renderer.endScene(postProcessSettings);

            if (showInfoOverlay) {
                drawSceneLabels(solarSystem, selectedBody, camera, aspectRatio,
                                framebufferWidth, framebufferHeight);
                drawBodyInfoPanel(selectedBody, camera, framebufferWidth, &showInfoOverlay);
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            window.swapBuffers();
            window.pollEvents();
            window.updateFramebufferSize();  // sync dims after resize / fullscreen
        }

        writeLog(std::string("main loop exited: shouldClose=") +
                 (window.shouldClose() ? "true" : "false") +
                 " wantsQuit=" + (wantsQuit ? "true" : "false"));
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        writeLog("imgui shutdown complete");

    } catch (const std::exception& e) {
        showError(e.what());
        return -1;
    }

    return 0;
}
