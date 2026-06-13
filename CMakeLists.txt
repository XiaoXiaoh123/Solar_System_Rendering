cmake_minimum_required(VERSION 3.20)
project(SolarSystem VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Options ---
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# --- Source files ---
set(CORE_SOURCES
    src/core/Window.cpp
    src/core/Camera.cpp
    src/core/Input.cpp
    src/core/Time.cpp
)

set(RENDER_SOURCES
    src/render/Shader.cpp
    src/render/Mesh.cpp
    src/render/Texture.cpp
    src/render/SphereMesh.cpp
    src/render/ResourceManager.cpp
    src/render/Skybox.cpp
    src/render/Renderer.cpp
)

set(SCENE_SOURCES
    src/scene/CelestialBody.cpp
    src/scene/Planet.cpp
    src/scene/Star.cpp
    src/scene/Orbit.cpp
    src/scene/SceneCatalog.cpp
    src/scene/SolarSystemConfig.cpp
    src/scene/SolarSystem.cpp
    src/scene/BlackHoleScene.cpp
)

set(UTILS_SOURCES
    src/utils/Paths.cpp
)

set(IMGUI_SOURCES
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/imgui_impl_glfw.cpp
    thirdparty/imgui/imgui_impl_opengl3.cpp
)

set(ALL_SOURCES
    main.cpp
    ${CORE_SOURCES}
    ${RENDER_SOURCES}
    ${SCENE_SOURCES}
    ${UTILS_SOURCES}
    ${IMGUI_SOURCES}
)

# --- Third-party: GLAD (OpenGL loader) ---
add_library(glad STATIC
    thirdparty/glad/src/glad.c
)
target_include_directories(glad PUBLIC thirdparty/glad/include)
target_link_libraries(glad PUBLIC opengl32)

# --- Third-party: GLFW ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/glfw_src)
target_link_libraries(glfw PRIVATE opengl32 gdi32)

# --- Main executable ---
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/glm
    ${CMAKE_SOURCE_DIR}/thirdparty/stb
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui
    ${CMAKE_SOURCE_DIR}/thirdparty/glfw_src/include
    ${CMAKE_SOURCE_DIR}/thirdparty/glad/include
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -O1)
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    opengl32
    gdi32
)

# --- Copy assets to output directory ---
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying assets to build directory..."
)

# --- Platform-specific ---
if(WIN32)
    # Use static MSVC runtime on Windows
    if(MSVC)
        set_property(TARGET ${PROJECT_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

# --- Print configuration ---
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
