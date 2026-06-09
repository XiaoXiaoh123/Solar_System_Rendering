# Solar System Rendering - Makefile (MinGW-w64 / GCC on Windows)

CXX       := g++
CC        := gcc
CXXFLAGS  := -std=c++17 -O1 -Wall
LDLIBS    := -lopengl32 -lgdi32 -mwindows

SRC_DIR   := src
BUILD_DIR := build/obj
OUT_DIR   := build
TARGET    := $(OUT_DIR)/SolarSystem.exe

# --- Detect shell at parse time (MSYSTEM only set in Git Bash / MSYS2) ---
ifdef MSYSTEM
  HAS_SH := yes
endif

# Convert forward slashes to backslashes for cmd /q /d /c commands
WIN_BUILD_DIR = $(subst /,\,$(BUILD_DIR))
WIN_OUT_DIR   = $(subst /,\,$(OUT_DIR))
WIN_TARGET    = $(subst /,\,$(TARGET))

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
        $(SRC_DIR)/render/ResourceManager.cpp \
        $(SRC_DIR)/render/Skybox.cpp \
        $(SRC_DIR)/render/Renderer.cpp \
        $(SRC_DIR)/scene/CelestialBody.cpp \
        $(SRC_DIR)/scene/Planet.cpp \
        $(SRC_DIR)/scene/Star.cpp \
        $(SRC_DIR)/scene/Orbit.cpp \
        $(SRC_DIR)/scene/SceneCatalog.cpp \
        $(SRC_DIR)/scene/SolarSystemConfig.cpp \
        $(SRC_DIR)/scene/SolarSystem.cpp \
        $(SRC_DIR)/utils/Paths.cpp \
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
	@echo Build complete: $(TARGET)

dirs:
ifeq ($(HAS_SH),)
# ---- Windows cmd ----
	@if not exist $(WIN_BUILD_DIR)\$(SRC_DIR)\core      mkdir $(WIN_BUILD_DIR)\$(SRC_DIR)\core
	@if not exist $(WIN_BUILD_DIR)\$(SRC_DIR)\render    mkdir $(WIN_BUILD_DIR)\$(SRC_DIR)\render
	@if not exist $(WIN_BUILD_DIR)\$(SRC_DIR)\scene     mkdir $(WIN_BUILD_DIR)\$(SRC_DIR)\scene
	@if not exist $(WIN_BUILD_DIR)\$(SRC_DIR)\lighting  mkdir $(WIN_BUILD_DIR)\$(SRC_DIR)\lighting
	@if not exist $(WIN_BUILD_DIR)\$(SRC_DIR)\utils     mkdir $(WIN_BUILD_DIR)\$(SRC_DIR)\utils
	@if not exist $(WIN_OUT_DIR)                        mkdir $(WIN_OUT_DIR)
	@if not exist $(WIN_OUT_DIR)\assets\config          mkdir $(WIN_OUT_DIR)\assets\config
	@if not exist $(WIN_OUT_DIR)\assets\shaders         mkdir $(WIN_OUT_DIR)\assets\shaders
	@if not exist $(WIN_OUT_DIR)\assets\textures        mkdir $(WIN_OUT_DIR)\assets\textures
	@if not exist $(WIN_OUT_DIR)\assets\textures\skybox mkdir $(WIN_OUT_DIR)\assets\textures\skybox
	@if exist assets\config\*.ini copy /y assets\config\*.ini $(WIN_OUT_DIR)\assets\config\ >nul 2>&1
	@if exist assets\shaders\*.vert copy /y assets\shaders\*.vert $(WIN_OUT_DIR)\assets\shaders\ >nul 2>&1
	@if exist assets\shaders\*.frag copy /y assets\shaders\*.frag $(WIN_OUT_DIR)\assets\shaders\ >nul 2>&1
	@if exist assets\textures\*.jpg  copy /y assets\textures\*.jpg  $(WIN_OUT_DIR)\assets\textures\ >nul 2>&1
	@if exist assets\textures\*.png  copy /y assets\textures\*.png  $(WIN_OUT_DIR)\assets\textures\ >nul 2>&1
	@if exist assets\textures\skybox\*.png copy /y assets\textures\skybox\*.png $(WIN_OUT_DIR)\assets\textures\skybox\ >nul 2>&1
else
# ---- Unix (Git Bash / MSYS2) ----
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/core
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/render
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/scene
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/lighting
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)/utils
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(OUT_DIR)/assets/config
	@mkdir -p $(OUT_DIR)/assets/shaders
	@mkdir -p $(OUT_DIR)/assets/textures
	@mkdir -p $(OUT_DIR)/assets/textures/skybox
	@cp assets/config/*.ini $(OUT_DIR)/assets/config/ 2>/dev/null || true
	@cp assets/shaders/*.vert $(OUT_DIR)/assets/shaders/ 2>/dev/null || true
	@cp assets/shaders/*.frag $(OUT_DIR)/assets/shaders/ 2>/dev/null || true
	@cp assets/textures/*.jpg  $(OUT_DIR)/assets/textures/ 2>/dev/null || true
	@cp assets/textures/*.png  $(OUT_DIR)/assets/textures/ 2>/dev/null || true
	@cp assets/textures/skybox/*.png $(OUT_DIR)/assets/textures/skybox/ 2>/dev/null || true
endif

# --- Link ---
$(TARGET): $(OBJS) $(GLAD_OBJ) $(GLFW_OBJS) | dirs
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

# --- Project .cpp to .o ---
$(BUILD_DIR)/%.o: %.cpp | dirs
ifeq ($(HAS_SH),)
	@if not exist $(subst /,\,$(dir $@)) mkdir $(subst /,\,$(dir $@))
else
	@mkdir -p $(dir $@)
endif
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- GLAD ---
$(GLAD_OBJ): $(GLAD_SRC) | dirs
	$(CC) -O2 -Ithirdparty/glad/include -c $< -o $@

# --- GLFW .c to .o ---
$(BUILD_DIR)/glfw_%.o: $(GLFW_DIR)/src/%.c | dirs
	$(CC) $(GLFW_CFLAGS) -c $< -o $@

# --- Clean ---
clean:
ifeq ($(HAS_SH),)
	@if exist $(WIN_BUILD_DIR) rmdir /s /q $(WIN_BUILD_DIR)
	@if exist $(WIN_TARGET)   del /q $(WIN_TARGET)
else
	@rm -rf $(BUILD_DIR) $(TARGET)
endif

# --- Run ---
run: all
ifeq ($(HAS_SH),)
	$(TARGET)
else
	cd $(OUT_DIR) && ./SolarSystem.exe
endif
