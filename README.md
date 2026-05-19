# Solar System Rendering

基于 C++ 与 OpenGL 4.6 的本地太阳系实时渲染项目。不依赖商业游戏引擎，使用 GLFW、GLAD、GLM 等图形库从零搭建渲染管线。

![OpenGL](https://img.shields.io/badge/OpenGL-4.6-blue)
![C++](https://img.shields.io/badge/C++-17-00599C)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)

## 功能

- 太阳 + 八颗行星 + 月球的三维可视化，**真实公转/自转周期**驱动
- 行星支持纹理贴图（无纹理时使用程序化着色）
- Blinn-Phong 光照模型（太阳为点光源），环境光 0 ~ 2.0 可调
- 轨道线渲染（含月球绕地球轨道），WASD 自由飞行摄像机，窗口自适应
- **Dear ImGui 控制面板**：时间倍速、播放/暂停、环境光、**每行星自转速度**、退出程序

## 快速开始

### 直接运行

将 `build/SolarSystem.exe` 与 `build/assets/` 放在**同一目录**下，双击运行。无外部依赖。

### 从源码构建

**要求：** MinGW-w64 (GCC 11+)，`mingw32-make` 和 `g++` 已加入 PATH。

支持 **PowerShell** / **CMD** / **Git Bash** 终端。

```bash
# 清理旧产物
mingw32-make clean

# 编译
mingw32-make all

# 编译并运行
mingw32-make run
```

构建启动 `build/SolarSystem.exe`，着色器和纹理自动复制到 `build/assets/`。

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
| **Ambient 滑块** | 环境光强度 0 ~ 2.0（默认 0.15） |
| **行星自转滑块** | 每颗行星独立调节自转速度 0 ~ 10x（默认 1.0x = 真实速度） |
| **Quit Program** | 退出程序 |

面板打开时摄像机锁定，天体自动暂停；关闭后面板恢复运动。

## 时间系统

- **1 真实秒 = 30 模拟地球日**（1x 默认基准，地球约 12 秒完成一圈公转）
- 时间倍速范围 0 ~ 10x，可通过滑块、预设按钮或键盘 `,` `.` 调整

## 纹理贴图

### 添加纹理

1. 将行星纹理图片（JPEG/PNG）放入 `assets/textures/` 目录
2. 在 [src/scene/SolarSystem.cpp](src/scene/SolarSystem.cpp) 中，为对应行星的 `CelestialParams` 最后一个字段传入纹理路径：

```cpp
addPlanet({"Earth",    Constants::EARTH_RADIUS,    Constants::EARTH_ORBIT,
           Constants::EARTH_ORBIT_PERIOD,   Constants::EARTH_ROT_PERIOD,
           Constants::EARTH_TILT,
           "assets/textures/earth_daymap.jpg"});  // 纹理路径
```

3. `mingw32-make all` 构建时会自动将 `assets/textures/` 复制到 `build/assets/textures/`

### 纹理来源

推荐 [Solar System Scope](https://www.solarsystemscope.com/textures/) 下载 2K/4K diffuse（color）贴图。

### 回退机制

如果纹理文件缺失或加载失败，行星会自动回退为程序化着色（基于位置的动态色调），程序不会崩溃。

### 支持的格式

| 格式 | 色彩空间 | 说明 |
|------|----------|------|
| JPEG | sRGB | 推荐，文件小 |
| PNG | sRGB (RGB) / Linear (RGBA) | 无损，支持透明度 |
| BMP / TGA | 取决于通道数 | 不推荐 |

## 项目结构

```
Solar_System_Rendering/
├── main.cpp
├── Makefile
├── CMakeLists.txt
├── README.md
├── 技术方案.md
├── assets/
│   ├── shaders/
│   │   ├── orbit.vert / orbit.frag
│   │   ├── planet.vert / planet.frag
│   │   ├── skybox.vert / skybox.frag
│   │   └── sun.vert / sun.frag
│   └── textures/
│       └── earth_daymap.jpg   (行星纹理)
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
- **月球父子关系**：月球通过 `setParent(Earth)` 绑定，`getModelMatrix()` 递归计算世界坐标，轨道线以地球为中心绘制
- **纹理回退**：`CelestialBody::draw()` 中检查 `m_texture.isValid()`，加载失败时自动使用 FragPos 驱动的动态色调
- **太阳着色器**：使用 Fresnel 效果（`pow(fresnel, 5.0)`）呈现从暖橙到亮黄的平滑过渡
- **输入架构**：Input 在 ImGui 之前初始化，避免 GLFW 回调冲突；面板打开时 `io.WantCapture` 机制隔离摄像机控制
- **窗口缩放**：`glfwSetFramebufferSizeCallback` 实时更新 `glViewport`，渲染填满窗口

## 系统要求

- Windows 10/11
- 支持 OpenGL 4.6 的 GPU（2014 年后的 NVIDIA / AMD / Intel 集显均可）
- 运行时无需额外安装

## 扩展计划

- [x] 行星纹理贴图（基础设施完成，地球已应用）
- [x] 月球 + 轨道线 + 父子层级系统
- [x] 控制面板：每行星自转速度独立调节（0 ~ 10x）
- [x] 控制面板：环境光滑块范围 0 ~ 2.0
- [ ] 大气散射（Rayleigh / Mie）
- [ ] 阴影映射
- [ ] LOD + 程序化星球生成
- [ ] 多线程纹理加载

## 许可

仅用于学习与个人项目。行星纹理素材推荐 [Solar System Scope](https://www.solarsystemscope.com/textures/)。
