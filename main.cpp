#include "src/core/Window.h"
#include "src/core/Camera.h"
#include "src/core/Input.h"
#include "src/core/Time.h"
#include "src/render/Renderer.h"
#include "src/render/Skybox.h"
#include "src/scene/SolarSystem.h"
#include "src/utils/Constants.h"
#include "src/utils/Paths.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

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
#include <sstream>
#include <ctime>

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
        Renderer::PostProcessSettings postProcessSettings;

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

            int framebufferWidth = window.getWidth();
            int framebufferHeight = window.getHeight();
            float aspectRatio = window.getAspectRatio();
            renderer.beginScene(framebufferWidth, framebufferHeight);
            renderer.clear();
            renderer.setViewport(framebufferWidth, framebufferHeight);
            renderer.drawSkybox(camera, skybox, aspectRatio);
            renderer.drawSolarSystem(solarSystem, camera, aspectRatio);
            renderer.endScene(postProcessSettings);

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
