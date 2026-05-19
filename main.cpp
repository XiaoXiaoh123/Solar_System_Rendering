#include "src/core/Window.h"
#include "src/core/Camera.h"
#include "src/core/Input.h"
#include "src/core/Time.h"
#include "src/render/Renderer.h"
#include "src/render/Skybox.h"
#include "src/scene/SolarSystem.h"
#include "src/utils/Constants.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl3.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

static void showError(const std::string& msg) {
    std::cerr << "Fatal error: " << msg << std::endl;
#ifdef _WIN32
    MessageBoxA(nullptr, msg.c_str(), "Solar System - Fatal Error",
                MB_OK | MB_ICONERROR);
#endif
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    try {
        Window window(Constants::DEFAULT_WIDTH, Constants::DEFAULT_HEIGHT,
                      "Solar System");

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GPU:    " << glGetString(GL_RENDERER) << std::endl;

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
        Renderer     renderer;
        SolarSystem  solarSystem;
        Skybox       skybox;
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
        bool  wantsQuit        = false;

        float ambientStrength  = Constants::AMBIENT_STRENGTH;

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

                    float earthAngle = 0.0f;
                    for (auto& p : solarSystem.getPlanets()) {
                        if (p->getName() == "Earth") {
                            earthAngle = p->getOrbitAngle();
                            break;
                        }
                    }
                    const char* paused = (timeScale == 0.0f) ? "[PAUSED]" : "";
                    ImGui::Text("FPS: %.0f  dt: %.4f  scale: %.3f %s",
                                io.Framerate, time.getRawDeltaTime(),
                                timeScale, paused);
                    ImGui::Text("Earth orbit angle: %.2f rad", earthAngle);
                }
                ImGui::End();
            }

            // --- Settings panel ---
            if (showSettings) {
                ImGui::SetNextWindowPos(ImVec2(10, 60), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_Once);

                if (ImGui::Begin("Control Panel", &showSettings,
                                 ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_AlwaysAutoResize)) {

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

                    // ---- Lighting ----
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.4f, 1.0f), "Lighting");
                    ImGui::Separator();
                    ImGui::SliderFloat("Ambient", &ambientStrength, 0.0f, 2.0f, "%.3f");

                    bool showAtmosphere = solarSystem.getShowAtmosphere();
                    if (ImGui::Checkbox("Atmospheric Scattering", &showAtmosphere)) {
                        solarSystem.setShowAtmosphere(showAtmosphere);
                    }

                    ImGui::Spacing();

                    // ---- Planets ----
                    ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "Planets");
                    ImGui::Separator();

                    auto planetSlider = [](const char* name, CelestialBody* body) {
                        float mult = body->getRotationSpeedMultiplier();
                        float base = body->getBaseRotationSpeed();
                        ImGui::PushID(name);
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("%s", name);
                        ImGui::SameLine(100);
                        ImGui::PushItemWidth(150);
                        if (ImGui::SliderFloat("##rot", &mult, 0.0f, 10.0f, "%.2fx")) {
                            body->setRotationSpeedMultiplier(mult);
                        }
                        ImGui::PopItemWidth();
                        ImGui::SameLine();
                        ImGui::TextDisabled("%.2f rad/s", base * mult);
                        ImGui::PopID();
                    };

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
                window.setCursorDisabled(false);
            } else if (!showSettings && prevShowSettings) {
                timeScale = savedTimeScale;
                time.setTimeScale(savedTimeScale);
                window.setCursorDisabled(true);
            }
            prevShowSettings = showSettings;

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

                if (!io.WantCaptureMouse) {
                    glm::vec2 mouseDelta = input.getMouseDelta();
                    camera.rotate(mouseDelta.x * Constants::CAM_SENSITIVITY,
                                 -mouseDelta.y * Constants::CAM_SENSITIVITY);
                    float scroll = io.MouseWheel;
                    if (scroll != 0.0f) camera.zoom(scroll * 2.0f);
                }
            }

            // --- Update & Render ---
            solarSystem.update(time);
            solarSystem.setAmbientStrength(ambientStrength);

            float aspectRatio = window.getAspectRatio();
            renderer.clear();
            renderer.setViewport(window.getWidth(), window.getHeight());
            renderer.drawSkybox(camera, skybox, aspectRatio);
            renderer.drawSolarSystem(solarSystem, camera, aspectRatio);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            window.swapBuffers();
            window.pollEvents();
            window.updateFramebufferSize();  // sync dims after resize / fullscreen
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

    } catch (const std::exception& e) {
        showError(e.what());
        return -1;
    }

    return 0;
}
