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
        // --- Init window & OpenGL context ---
        Window window(Constants::DEFAULT_WIDTH, Constants::DEFAULT_HEIGHT,
                      "Solar System - OpenGL 4.6");

        // --- Init GLAD (must be before ANY OpenGL call) ---
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD (OpenGL loader)");
        }

        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

        // --- Core systems ---
        Input input(window.getHandle());
        Camera camera(glm::vec3(0.0f, 350.0f, 800.0f), -90.0f, -25.0f);
        Time  time;
        Renderer renderer;

        // --- Scene (shader loading uses OpenGL, must be after GLAD init) ---
        SolarSystem solarSystem;

        // --- Skybox ---
        Skybox skybox;

        // --- UI state ---
        bool  showOrbits = true;
        float timeScale  = Constants::DEFAULT_TIME_SCALE;
        time.setTimeScale(timeScale);

        // --- Main loop ---
        while (!window.shouldClose()) {
            time.tick();
            input.update();

            // --- Process input ---
            if (input.isKeyDown(GLFW_KEY_ESCAPE)) {
                glfwSetWindowShouldClose(window.getHandle(), GLFW_TRUE);
            }

            // Camera movement
            float camSpeed = Constants::CAM_SPEED;
            if (input.isKeyDown(GLFW_KEY_LEFT_SHIFT)) {
                camSpeed = Constants::CAM_SPEED_FAST;
            }
            if (input.isKeyDown(GLFW_KEY_W)) camera.moveForward( camSpeed * time.getRawDeltaTime());
            if (input.isKeyDown(GLFW_KEY_S)) camera.moveForward(-camSpeed * time.getRawDeltaTime());
            if (input.isKeyDown(GLFW_KEY_A)) camera.moveRight(  -camSpeed * time.getRawDeltaTime());
            if (input.isKeyDown(GLFW_KEY_D)) camera.moveRight(   camSpeed * time.getRawDeltaTime());
            if (input.isKeyDown(GLFW_KEY_Q)) camera.moveUp(     -camSpeed * time.getRawDeltaTime());
            if (input.isKeyDown(GLFW_KEY_E)) camera.moveUp(      camSpeed * time.getRawDeltaTime());

            // Mouse look
            glm::vec2 mouseDelta = input.getMouseDelta();
            camera.rotate(mouseDelta.x * Constants::CAM_SENSITIVITY,
                         -mouseDelta.y * Constants::CAM_SENSITIVITY);

            // Scroll zoom
            float scroll = input.getScrollDelta();
            if (scroll != 0.0f) {
                camera.zoom(scroll * 2.0f);
            }

            // Time control
            if (input.isKeyDown(GLFW_KEY_PERIOD)) {
                timeScale *= 1.0f + time.getRawDeltaTime() * 2.0f;
                time.setTimeScale(timeScale);
            }
            if (input.isKeyDown(GLFW_KEY_COMMA)) {
                timeScale /= 1.0f + time.getRawDeltaTime() * 2.0f;
                time.setTimeScale(timeScale);
            }
            if (input.isKeyDown(GLFW_KEY_SPACE)) time.setTimeScale(0.0f);
            if (input.isKeyDown(GLFW_KEY_R))     time.setTimeScale(1.0f);

            // Toggle orbits
            static bool oKeyWasDown = false;
            bool oKeyDown = input.isKeyDown(GLFW_KEY_O);
            if (oKeyDown && !oKeyWasDown) showOrbits = !showOrbits;
            oKeyWasDown = oKeyDown;

            // --- Update ---
            solarSystem.update(time);

            // --- Render ---
            float aspectRatio = window.getAspectRatio();
            renderer.clear();
            renderer.setViewport(window.getWidth(), window.getHeight());
            renderer.drawSolarSystem(solarSystem, camera, aspectRatio);

            // --- FPS counter ---
            static float fpsTimer = 0.0f;
            static int   fpsCount = 0;
            fpsTimer += time.getRawDeltaTime();
            fpsCount++;
            if (fpsTimer >= 1.0f) {
                std::string title = "Solar System | FPS: " + std::to_string(fpsCount)
                    + " | Time: " + std::to_string(timeScale).substr(0, 5) + "x"
                    + " | Orbits: " + (showOrbits ? "ON" : "OFF");
                glfwSetWindowTitle(window.getHandle(), title.c_str());
                fpsTimer = 0.0f;
                fpsCount = 0;
            }

            // --- Swap buffers and poll events ---
            window.swapBuffers();
            window.pollEvents();
        }

    } catch (const std::exception& e) {
        showError(e.what());
        return -1;
    }

    return 0;
}
