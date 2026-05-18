# Solar System Rendering

基于 C++ 与 OpenGL 4.6 的本地太阳系实时渲染项目。不依赖商业游戏引擎，使用 GLFW、GLAD、GLM 等图形库从零搭建渲染管线。

![OpenGL](https://img.shields.io/badge/OpenGL-4.6-blue)
![C++](https://img.shields.io/badge/C++-17-00599C)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)

## 功能

- 太阳 + 八颗行星的三维可视化，公转与自转模拟
- Blinn-Phong 光照模型（太阳为点光源）
- 轨道线渲染，WASD 自由飞行摄像机
- 时间缩放（暂停 / 1x / 最高 1000x），FPS 显示
- 行星支持纹理贴图（无纹理时使用程序化颜色）

## 快速开始

### 直接运行（已编译）

将 `build/SolarSystem.exe` 与 `build/assets/` 放在**同一目录**下，双击 `SolarSystem.exe`。

### 从源码构建

**要求：** MinGW-w64 (GCC 11+) 或 MSVC 2022，CMake 3.20+

```bash
# MinGW 构建
mingw32-make all

# 或 CMake 构建
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

构建产物为 `build/SolarSystem.exe`，着色器自动复制到 `build/assets/shaders/`。

## 操作说明

| 按键 | 功能 |
|------|------|
| W A S D | 前后左右移动 |
| Q / E | 上升 / 下降 |
| 鼠标移动 | 旋转视角 |
| 滚轮 | 缩放视野 |
| Shift 按住 | 加速移动 (5x) |
| ESC | 退出程序 |
| O | 切换轨道线显示 |
| . / , | 加速 / 减速时间 |
| 空格 | 暂停时间 |
| R | 恢复 1x 速度 |

## 项目结构

```
Solar_System_Rendering/
├── main.cpp                     # 入口，主循环
├── Makefile / CMakeLists.txt    # 构建配置
├── assets/shaders/              # GLSL 着色器
│   ├── planet.vert/frag        #   行星（Phong 光照）
│   ├── sun.vert/frag           #   太阳（自发光）
│   ├── skybox.vert/frag        #   天空盒
│   └── orbit.vert/frag         #   轨道线
├── src/
│   ├── core/                    # 窗口、摄像机、输入、计时
│   ├── render/                  # Shader、Mesh、纹理、天空盒、渲染器
│   ├── scene/                   # 天体、行星、恒星、轨道、太阳系
│   ├── lighting/                # 点光源
│   └── utils/                   # 物理常量与缩放参数
└── thirdparty/                  # 第三方库（源码随项目分发）
    ├── glfw_src/                #   GLFW 3.4
    ├── glad/                    #   GLAD (OpenGL 4.6 Core)
    ├── glm/                     #   GLM 1.0.1
    ├── stb/                     #   stb_image
    └── imgui/                   #   Dear ImGui 1.91
```

## 系统要求

- Windows 10/11 64 位
- 支持 OpenGL 4.6 的 GPU（2014 年后的 NVIDIA / AMD / Intel 集显均可）
- 运行时无需额外安装 — GLFW 与 GLAD 静态链接

## 扩展计划

- [ ] 大气散射（Rayleigh / Mie）
- [ ] 阴影映射 (Shadow Mapping)
- [ ] LOD 四叉树球体与程序化星球生成
- [ ] 多线程纹理异步加载
- [ ] Dear ImGui 调试面板

## 许可

仅用于学习与个人项目。行星纹理素材请自行获取（推荐 [Solar System Scope](https://www.solarsystemscope.com/textures/)）。
