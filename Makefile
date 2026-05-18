# Solar System Rendering - Makefile (MinGW-w64 / GCC on Windows)

CXX       := g++
CC        := gcc
CXXFLAGS  := -std=c++17 -O2 -Wall
LDLIBS    := -lopengl32 -lgdi32 -mwindows

SRC_DIR   := src
BUILD_DIR := build/obj
OUT_DIR   := build
TARGET    := $(OUT_DIR)/SolarSystem.exe

# --- GLFW ---
GLFW_DIR  := thirdparty/glfw_src
GLFW_SRCS := context init input monitor platform vulkan window \
             egl_context osmesa_context null_init null_monitor null_window null_joystick \
             win32_init win32_joystick win32_module win32_monitor win32_thread win32_time win32_window wgl_context
GLFW_OBJS := $(patsubst %,$(BUILD_DIR)/glfw_%.o,$(GLFW_SRCS))

# --- GLAD ---
GLAD_SRC  := thirdparty/glad/src/glad.c
GLAD_OBJ  := $(BUILD_DIR)/glad.o

# --- Project sources ---
IMGUI_DIR := thirdparty/imgui
IMGUI_SRCS := $(IMGUI_DIR)/imgui.cpp \
              $(IMGUI_DIR)/imgui_draw.cpp \
              $(IMGUI_DIR)/imgui_tables.cpp \
              $(IMGUI_DIR)/imgui_widgets.cpp \
              $(IMGUI_DIR)/imgui_impl_glfw.cpp \
              $(IMGUI_DIR)/imgui_impl_opengl3.cpp

SRCS := main.cpp \
        $(SRC_DIR)/core/Window.cpp \
        $(SRC_DIR)/core/Camera.cpp \
        $(SRC_DIR)/core/Input.cpp \
        $(SRC_DIR)/core/Time.cpp \
        $(SRC_DIR)/render/Shader.cpp \
        $(SRC_DIR)/render/Mesh.cpp \
        $(SRC_DIR)/render/Texture.cpp \
        $(SRC_DIR)/render/SphereMesh.cpp \
        $(SRC_DIR)/render/Skybox.cpp \
        $(SRC_DIR)/render/Renderer.cpp \
        $(SRC_DIR)/scene/CelestialBody.cpp \
        $(SRC_DIR)/scene/Planet.cpp \
        $(SRC_DIR)/scene/Star.cpp \
        $(SRC_DIR)/scene/Orbit.cpp \
        $(SRC_DIR)/scene/SolarSystem.cpp \
        $(IMGUI_SRCS)

OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# --- Include paths ---
INCLUDES := -I. -I$(SRC_DIR) \
            -Ithirdparty/glad/include \
            -Ithirdparty/glm \
            -Ithirdparty/stb \
            -Ithirdparty/imgui \
            -I$(GLFW_DIR)/include

GLFW_CFLAGS := -O2 -Wall -D_GLFW_WIN32 -DUNICODE -D_UNICODE -I$(GLFW_DIR)/include -I$(GLFW_DIR)/src

# --- Rules ---
.PHONY: all clean run dirs

all: dirs $(TARGET)
	@echo "Build complete: $(TARGET)"

dirs:
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/core
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/render
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/scene
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/lighting
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/utils
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(OUT_DIR)/assets/shaders
	@cp -u assets/shaders/*.vert assets/shaders/*.frag $(OUT_DIR)/assets/shaders/ 2>/dev/null || true

# --- Link ---
$(TARGET): $(OBJS) $(GLAD_OBJ) $(GLFW_OBJS) | dirs
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

# --- Project .cpp → .o ---
$(BUILD_DIR)/%.o: %.cpp | dirs
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- GLAD ---
$(GLAD_OBJ): $(GLAD_SRC) | dirs
	$(CC) -O2 -Ithirdparty/glad/include -c $< -o $@

# --- GLFW .c → .o ---
$(BUILD_DIR)/glfw_%.o: $(GLFW_DIR)/src/%.c | dirs
	$(CC) $(GLFW_CFLAGS) -c $< -o $@

# --- Clean ---
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# --- Run ---
run: all
	cd $(OUT_DIR) && ./SolarSystem.exe
