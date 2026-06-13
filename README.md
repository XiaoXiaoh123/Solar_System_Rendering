# Solar System Rendering

基于 C++17 与 OpenGL 4.6 的本地实时宇宙场景渲染项目。不依赖商业游戏引擎，使用 GLFW、GLAD、GLM、stb_image 与 Dear ImGui 从零搭建渲染、资源、交互和教学 UI。

![OpenGL](https://img.shields.io/badge/OpenGL-4.6-blue)
![C++](https://img.shields.io/badge/C++-17-00599C)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)

## 当前功能

- **场景系统**：默认进入 `Solar System`，可在控制面板顶部切换到 `Black Hole`
- **启动加载画面**：资源初始化期间显示进度和状态，避免启动时黑屏无反馈
- **太阳系渲染**：太阳、八大行星、月球、轨道线、天空盒、行星纹理和程序化回退
- **椭圆轨道**：支持半长轴、偏心率、倾角、升交点经度、近日点参数
- **尺度模式**：`Artistic / Real / Logarithmic` 三种比例切换
- **日期系统**：模拟时间映射为年月日，并在 UI 中显示
- **大气散射**：地球、火星、金星拥有可调的大气视觉参数
- **HDR / Bloom**：太阳、吸积盘、光子环等高亮区域支持后处理增强
- **黑洞场景**：事件视界、光子球、吸积盘、Schwarzschild 风格光线步进透镜、近似 Kerr 自旋非对称
- **教学模式**：黑洞标签、说明面板、参数 preset、debug preset、镜头 preset
- **资源管理**：统一加载 shader、texture、mesh，支持 shader 热重载
- **性能基础**：共享球体网格、LOD 球体、uniform location 缓存

## 快速开始

### 直接运行

将 `build/SolarSystem.exe` 与 `build/assets/` 保持在同一目录下，双击运行。

如果启动阶段短暂停留在加载页，这是正常的资源加载过程。运行日志位于：

```text
build/SolarSystem.log
build/SolarSystem_error.log
```

### 从源码构建

要求：

- Windows 10/11
- 支持 OpenGL 4.6 的 GPU
- MinGW-w64 / GCC 11+
- `mingw32-make`、`g++` 已加入 `PATH`

```bash
mingw32-make clean
mingw32-make all
mingw32-make run
```

也可以使用 CMake：

```bash
cmake -S . -B build/cmake-check -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build/cmake-check
```

构建后会自动复制 `assets/` 到输出目录。

## 操作说明

| 操作 | 功能 |
|------|------|
| W A S D | 前后左右移动 |
| Q / E | 下降 / 上升 |
| 鼠标移动 | 旋转相机 |
| 鼠标滚轮 | 缩放视角 |
| Shift | 加速移动 |
| ESC | 打开 / 关闭控制面板 |
| 空格 | 暂停 / 恢复模拟时间 |
| . / , | 增加 / 降低时间倍率 |
| F5 | 热重载 shader |

控制面板打开时显示鼠标，摄像机输入会让给 ImGui。关闭面板后恢复自由飞行控制。

## 控制面板

### 通用

- `Scene Select`：切换 `Solar System` / `Black Hole`
- `Play / Pause`：暂停或恢复模拟时间
- `Time Scale`：调整时间倍率
- `Post FX`：控制 bloom、曝光、阈值、模糊次数

### Solar System

- `Scale Mode`：艺术比例、真实比例、对数比例
- `Debug View`：Lit、Raw Texture、UV、Normals
- `Atmospheric Scattering`：开启或关闭大气
- 大气参数：强度、边缘辉光、日落带、背光散射、昼夜边界
- 行星列表：每个天体可调自转速度，并可 `Select and Follow`
- `Info`：显示天体标签、距离、周期、轨道参数等讲解信息

### Black Hole

- `Teaching Panel` / `Labels`：开启黑洞教学说明和场景标签
- `Parameter Preset`：快速切换黑洞视觉风格
- `Debug Preset`：查看温度、Doppler、alpha、远近侧、透镜场、光子环、阴影遮罩
- `Camera Preset`：Teaching、Edge、Top、Photon Ring、Wide Lens
- 可调参数：事件视界、光子球、自旋、吸积盘内外半径、厚度、湍流、Doppler、透镜强度、阴影柔度等

## 项目结构

```text
Solar_System_Rendering/
├── main.cpp
├── Makefile
├── CMakeLists.txt
├── README.md
├── 黑洞渲染方案.md
├── assets/
│   ├── config/
│   │   ├── solar_system.ini
│   │   └── black_hole.ini
│   ├── shaders/
│   │   ├── planet.vert / planet.frag
│   │   ├── atmosphere.vert / atmosphere.frag
│   │   ├── sun.vert / sun.frag
│   │   ├── bloom_*.vert / bloom_*.frag
│   │   ├── hdr_composite.vert / hdr_composite.frag
│   │   ├── black_hole.vert / black_hole.frag
│   │   ├── accretion_disk.vert / accretion_disk.frag
│   │   └── gravitational_lensing.vert / gravitational_lensing.frag
│   └── textures/
│       └── skybox/starmap_2020_4k.png
├── src/
│   ├── core/
│   ├── render/
│   │   ├── Renderer.*
│   │   ├── ResourceManager.*
│   │   ├── Shader.*
│   │   ├── Mesh.*
│   │   └── Skybox.*
│   ├── scene/
│   │   ├── SceneCatalog.*
│   │   ├── SolarSystem.*
│   │   ├── SolarSystemConfig.*
│   │   ├── BlackHoleScene.*
│   │   ├── CelestialBody.*
│   │   ├── Planet.*
│   │   ├── Star.*
│   │   └── Orbit.*
│   └── utils/
└── thirdparty/
```

## 关键实现

- `SceneCatalog` 负责场景目录，主循环根据 `activeScene` 分发更新与绘制
- `SolarSystemConfig` 从 `assets/config/solar_system.ini` 加载天体配置
- `BlackHoleScene` 独立管理黑洞参数、吸积盘 mesh、教学 UI 和镜头 preset
- `Renderer` 负责 HDR framebuffer、bloom、屏幕空间透镜和最终合成
- `ResourceManager` 统一管理 shader、texture、mesh，并支持 shader 热重载
- `Shader` 缓存 uniform location，减少每帧查询开销
- 黑洞场景采用懒加载，默认太阳系启动时不编译黑洞 shader

## 已知取舍

- 黑洞引力透镜目前是低成本 Schwarzschild 风格步进近似，不是完整广义相对论光线追踪
- 吸积盘厚度、遮挡和 Doppler 是视觉近似，目标是实时可讲解
- 启动加载仍是同步资源加载，只是加入了可见反馈；后续可升级异步加载
- Intel 集显对复杂 GLSL 编译较敏感，吸积盘 shader 已改为更保守的兼容写法

## 后续计划

- 异步资源加载与真正的加载任务进度
- 黑洞 ray marching / geodesic 近似积分
- 更真实的星场透镜背景和吸积盘自遮挡
- 喷流、热冕、尘埃/等离子体体积效果
- 教学 preset 保存/导入导出
- 截图、录制、讲解脚本模式
- 多场景扩展：双星、星云、星系、黑洞吞噬恒星等

## 许可

仅用于学习与个人项目。纹理素材请遵循各自来源的授权条款。
