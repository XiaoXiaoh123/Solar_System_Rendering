# Solar System Rendering

基于 C++ 与 OpenGL 4.6 的本地太阳系实时渲染项目。不依赖商业游戏引擎，使用 GLFW、GLAD、GLM 等图形库从零搭建渲染管线。

![OpenGL](https://img.shields.io/badge/OpenGL-4.6-blue)
![C++](https://img.shields.io/badge/C++-17-00599C)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)

## 功能

- 太阳 + 八颗行星的三维可视化，**真实公转/自转周期**驱动
- Blinn-Phong 光照模型（太阳为点光源），环境光可调
- 轨道线渲染，WASD 自由飞行摄像机，窗口自适应（支持最大化/全屏）
- **Dear ImGui 控制面板**：时间倍速、播放/暂停、环境光、退出程序
- 行星支持纹理贴图（无纹理时使用程序化着色）

## 快速开始

### 直接运行

将 `build/SolarSystem.exe` 与 `build/assets/` 放在**同一目录**下，双击运行。无外部依赖。

### 从源码构建

**要求：** MinGW-w64 (GCC 11+)，CMake 3.20+

```bash
# MinGW 构建
mingw32-make all

# 运行
mingw32-make run
```

构建产物为 `build/SolarSystem.exe`，着色器自动复制到 `build/assets/shaders/`。

## 操作说明

| 按键 | 功能 |
|------|------|
| W A S D | 前后左右移动 |
| Q / E | 上升 / 下降 |
| 鼠标移动 | 旋转视角 |
| 滚轮 | 缩放 |
| Shift | 加速移动 (5x) |
| **ESC** | 打开 / **关闭控制面板**（打开时暂停天体，显示鼠标） |
| 空格 | 暂停 / 恢复时间 |
| . / , | 加速 / 减速时间 |

### 控制面板

位于左上角，ESC 唤出。包含：

| 控件 | 说明 |
|------|------|
| **Play / Pause** | 暂停 / 恢复天体运动 |
| **Time Scale 滑块** | 0 ~ 10x，拖动即时生效 |
| **预设按钮** | 0 / 0.1x / 0.5x / 1x / 5x / 10x |
| **Ambient 滑块** | 环境光强度 0 ~ 0.5 |
| **Quit Program** | 退出程序 |

面板打开时摄像机锁定，天体自动暂停；关闭后面板恢复运动。

## 时间系统

- **1 真实秒 = 30 模拟地球日**（1x 默认基准，地球约 12 秒完成一圈公转）
- 时间倍速范围 0 ~ 10x，可通过滑块、预设按钮或键盘 `,` `.` 调整

## 项目结构

```
Solar_System_Rendering/
├── main.cpp
├── Makefile
├── CMakeLists.txt
├── README.md
├── 技术方案.md
├── assets/
│   └── shaders/
│       ├── orbit.vert / orbit.frag
│       ├── planet.vert / planet.frag
│       ├── skybox.vert / skybox.frag
│       └── sun.vert / sun.frag
├── src/
│   ├── core/
│   │   ├── Camera.h / .cpp
│   │   ├── Input.h / .cpp
│   │   ├── Time.h / .cpp
│   │   └── Window.h / .cpp
│   ├── lighting/
│   │   └── PointLight.h
│   ├── render/
│   │   ├── Mesh.h / .cpp
│   │   ├── Renderer.h / .cpp
│   │   ├── Shader.h / .cpp
│   │   ├── Skybox.h / .cpp
│   │   ├── SphereMesh.h / .cpp
│   │   └── Texture.h / .cpp
│   ├── scene/
│   │   ├── CelestialBody.h / .cpp
│   │   ├── Orbit.h / .cpp
│   │   ├── Planet.h / .cpp
│   │   ├── SolarSystem.h / .cpp
│   │   └── Star.h / .cpp
│   └── utils/
│       └── Constants.h
└── thirdparty/
    ├── glad/
    │   ├── include/
    │   │   ├── glad/gl.h
    │   │   ├── glad/glad.h
    │   │   └── KHR/khrplatform.h
    │   └── src/glad.c
    ├── glfw_src/          (GLFW 3.4 源码)
    ├── glm/               (GLM 1.0.1, header-only)
    ├── imgui/             (Dear ImGui 1.91 + backends)
    └── stb/stb_image.h
```

## 关键实现细节

- **模型矩阵**：`R_orbit * T_orbit * R_tilt * R_self` 顺序（GLM 右乘），确保公转坐标正确
- **输入架构**：Input 在 ImGui 之前初始化，避免 GLFW 回调冲突；面板打开时 `io.WantCapture` 机制隔离摄像机控制
- **窗口缩放**：`glfwSetFramebufferSizeCallback` 实时更新 `glViewport`，渲染填满窗口

## 系统要求

- Windows 10/11
- 支持 OpenGL 4.6 的 GPU（2014 年后的 NVIDIA / AMD / Intel 集显均可）
- 运行时无需额外安装

## 扩展计划

- [ ] 大气散射（Rayleigh / Mie）
- [ ] 阴影映射
- [ ] LOD + 程序化星球生成
- [ ] 多线程纹理加载
- [ ] 行星纹理贴图

## 许可

仅用于学习与个人项目。行星纹理素材推荐 [Solar System Scope](https://www.solarsystemscope.com/textures/)。
