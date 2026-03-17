# gta-reversed 编译、运行与操作指南

> 基于项目源码（README.md、setup.py、CMakeLists.txt、conanfile.py、conanprofile.txt、contrib/install.py）核实编写。

## 一、环境要求

| 工具 | 版本要求 | 说明 |
|------|----------|------|
| Visual Studio | **2026**（推荐） | 默认 `conanprofile.txt` 中 `compiler.version=195` 对应 VS 2026 MSVC 工具集 |
| Visual Studio | 2022（可选） | 需手动将 `conanprofile.txt` 中 `compiler.version` 改为 `194` |
| Python | >= 3.x | 安装时勾选 "Add to PATH" |
| Conan | >= 2.x | 通过 `pip install conan` 安装 |
| CMake | >= 4.20 | Conan 会通过 `[replace_tool_requires]` 自动下载 cmake/4.2.0，无需单独安装特定版本 |

> **注意**：项目为 **x86 (Win32)** 构建，`conanprofile.txt` 中 `arch=x86`，`CMakeLists.txt` 中 `CMAKE_GENERATOR_PLATFORM` 设为 `Win32`。

## 二、首次编译

### 步骤 1：安装 Conan

```bash
pip install conan
conan profile detect    # 仅首次安装时需要
```

### 步骤 2：运行 setup 脚本

```bash
# 在项目根目录执行
python setup.py                          # Debug + Unity Build（默认，编译更快）
python setup.py --no-unity-build         # Debug + 非 Unity Build
python setup.py --buildconf Release      # Release 构建
python setup.py --buildconf RelWithDebInfo  # Release + 调试信息
```

`setup.py` 会依次执行：
1. `conan install` — 安装依赖（ogg、vorbis、nlohmann_json、spdlog、tracy、imgui、SDL3、libjpeg-turbo）
2. `cmake --preset default-unity` (或 `default`) — 生成 VS 解决方案

### 步骤 3：编译

**方法 A — 命令行：**
```bash
cmake --build build
```

**方法 B — Visual Studio：**
打开 `build/GTASA.sln`（或 `build/GTASA.slnx`），选择 Debug/Release 配置后编译。

### 编译产物

```
bin/Debug/gta_reversed.asi      # Debug 版本
bin/Debug/gta_reversed.pdb      # 调试符号
bin/Release/gta_reversed.asi    # Release 版本
```

## 三、安装到游戏

### 游戏 EXE 要求（重要！）

必须使用 GTA San Andreas **Compact exe**：
- 文件大小**恰好** `5,189,632 bytes`（4.94 MiB）
- 这**不是**普通的 1.0 US exe，使用错误的 exe 会导致随机崩溃
- 你必须拥有正版游戏资源文件

### 方法 A：自动安装（推荐）

```bash
# 需要管理员权限（创建符号链接需要）
python contrib/install.py
```

脚本会：
1. 弹出文件选择对话框，让你选择 `gta_sa.exe`
2. 解压 `contrib/plugins.zip` 到游戏目录（包含 ASI Loader + Mouse Fix dinput8.dll）
3. 在 `<游戏目录>/scripts/` 下创建指向 `bin/<config>/` 的**符号链接**
4. 设置环境变量 `GTA_SA_EXE` 和 `GTA_SA_DIR`

> 使用符号链接的好处：每次重新编译后，游戏目录自动获取最新的 `.asi` 文件，无需手动复制。

### 方法 B：手动安装

1. 下载 [Silent's ASI Loader](https://gtaforums.com/topic/523982-relopensrc-silents-asi-loader/)，将其 DLL 放入游戏根目录
2. 解压 `contrib/plugins.zip` 中的 `dinput8.dll`（Mouse Fix）到游戏根目录
3. 在游戏根目录创建 `scripts` 文件夹（如果不存在）
4. 将 `bin/<config>/gta_reversed.asi` 复制到 `<游戏目录>/scripts/`

### 目录结构示例

```
GTA San Andreas/
├── gta_sa.exe              # Compact exe (5,189,632 bytes)
├── vorbisFile.dll          # ASI Loader (或 dinput8.dll 方式)
├── dinput8.dll             # Mouse Fix
├── scripts/
│   ├── gta_reversed.asi    # 本项目编译产物（或其符号链接）
│   └── gta_reversed.pdb    # 调试符号（可选，调试用）
└── ... (游戏原始文件)
```

## 四、运行与调试

### 启动游戏

直接运行 `gta_sa.exe`，ASI Loader 会自动加载 `scripts/` 目录下的所有 `.asi` 文件。

### 附加调试器

1. 启动游戏
2. 在 Visual Studio 中 **Debug → Attach to Process → gta_sa.exe**
3. 推荐安装 [ReAttach](https://marketplace.visualstudio.com/items?itemName=ErlandR.ReAttach) 插件简化此流程

### FastLoader 快速启动

项目内置了快速加载功能，可通过 INI 配置（`[FastLoader]` 段）控制：

| 设置 | 默认值 | 说明 |
|------|--------|------|
| `SaveGameToLoad` | `-1` | 自动加载的存档编号（-1 = 不自动加载） |
| `SkipSaveGameLoadKey` | `VK_CONTROL` | 按住此键跳过自动加载 |
| `NoEAX` | `true` | 跳过 EAX 启动画面 |
| `NoNVidia` | `true` | 跳过 NVidia 启动画面 |
| `NoLogo` | `true` | 跳过 Logo 视频 |
| `NoTitleOrIntro` | `true` | 跳过片头视频 |
| `NoCopyright` | `true` | 跳过版权画面 |
| `NoFading` | `true` | 跳过加载淡入淡出 |
| `NoLoadScreen` | `true` | 跳过预加载画面 |
| `NoLoadBar` | `false` | 跳过加载进度条 |
| `NoLoadingTune` | `true` | 跳过加载音乐 |
| `RenderAtAllTimes` | `true` | 最小化时继续渲染 |

## 五、调试菜单（F7）

按 **F7**（或 **Ctrl+M**）打开/关闭 ImGui 调试菜单。菜单状态自动保存到 `DebugModules.json`。

### 菜单模块一览

#### Tools（工具）
| 模块 | 功能 |
|------|------|
| **Teleporter** | 传送到坐标/地图标记/已保存位置，支持数字键 1-9 快捷传送 |
| **Spawner** | 搜索并生成任意 Ped 或车辆模型 |
| **Mission Starter** | 按名称/ID 启动 135 个任务中的任意一个 |
| **Cheats** | GUI 作弊面板（无敌、武器、金钱、通缉等级、天气等） |

#### Settings（设置）
| 模块 | 功能 |
|------|------|
| **Hooks** | 运行时切换任意函数的原版/逆向实现，树状视图 + 命名空间过滤 |
| **Post / Special FX** | 开关/调参所有后处理效果（夜视、红外、热浪、CCTV 等） |
| **Bugs** | 开关可逆 Bug 修复（空指针、越界、帧率依赖等） |

#### Visualization（可视化）
| 模块 | 功能 |
|------|------|
| **Collision** | 3D 渲染碰撞网格（盒体、球体、三角形、包围盒） |
| **Ped Info** | 查看所有 Ped 的任务层级树，3D 可视化任务路径 |
| **Cover Points** | 可视化掩体点，双击传送 |
| **2D Effects** | 可视化灯光、粒子、吸引点、入口等，按类型着色 |
| **TimeCyc / Cull Zones / COcclusion / Audio Zones** | 各类区域可视化 |

#### Stats（统计）
| 模块 | 功能 |
|------|------|
| **Pools** | 26 个内存池使用率表格 |
| **Streaming** | 加载通道状态（IDLE/STARTED/READING/ERR） |
| **ImGui Metrics** | ImGui 内置指标/Demo 窗口 |

#### Extra（其他）
| 模块 | 功能 |
|------|------|
| **Sound Manager** | 实时播放音效列表，可调音量/暂停 |
| **Vehicle Info** | 悬挂压缩、弹簧长度等数据 |
| **Checkpoints** | 放置/测试各类检查点标记 |
| **TimeCycle Editor** | 完整的时间循环编辑器（冻结时间、天气、颜色） |
| **Load Monitor** | 帧内加载性能监控图表 |
| **Scripts / Clouds / Weapon / Particle / Text / Proc Objects** | 各子系统调试 |
| **多个 Audio 子模块** | 警用扫描器、车辆音频、音效硬件、环境/过场/电台音轨 |

### 快捷键

| 按键 | 上下文 | 功能 |
|------|--------|------|
| **F7** / **Ctrl+M** | 全局 | 开关调试菜单 |
| **1-9** | 菜单关闭时 | 传送到 Teleporter 保存的位置槽 |
| **0** | 菜单关闭时 | 传送回上一个位置 |
| **J** | 菜单关闭时 | 喷气背包 |
| **T** | 菜单关闭时 | 生成 Buffalo + 提升通缉 |
| **Ctrl（按住）** | 加载画面 | 跳过自动加载存档 |

## 六、日志与性能分析

### 日志系统（spdlog）

项目使用 spdlog，通过以下宏输出日志：

```cpp
NOTSA_LOG_DEBUG(...)   // 调试信息
NOTSA_LOG_INFO(...)    // 一般信息
NOTSA_LOG_WARN(...)    // 警告
NOTSA_LOG_ERR(...)     // 错误
NOTSA_LOG_CRIT(...)    // 严重错误
NOTSA_LOG_TRACE(...)   // 追踪级别
```

日志级别默认为 `SPDLOG_LEVEL_TRACE`（全量输出）。

### 性能分析（Tracy）

渲染管线中集成了 Tracy profiling 标记（`ZoneScoped`），可连接 [Tracy Profiler](https://github.com/wolfpld/tracy) 客户端查看帧级性能数据。

## 七、Conan 依赖清单

来源：`conanfile.py`

| 依赖 | 版本 | 用途 |
|------|------|------|
| ogg | 1.3.5 | 音频编解码基础库 |
| vorbis | 1.3.7 | OGG Vorbis 音频解码 |
| nlohmann_json | 3.11.3 | JSON 序列化（调试模块状态等） |
| spdlog | 1.15.0 | 日志框架（使用 std::format） |
| tracy | cci.20220130 | 性能分析 |
| imgui | 1.91.5-docking | 调试菜单 UI（Docking 分支） |
| SDL3 | 3.2.6 | 输入处理（默认；可选 DInput） |
| libjpeg-turbo | 3.1.0 | JPEG 图像处理 |

## 八、注意事项

- **不要使用其他第三方插件**——项目明确表示不对未测试的插件提供支持
- 编译为 **x86 (Win32)**，不支持 x64
- C++ 标准为 **C++23**（`conanprofile.txt` 中 `compiler.cppstd=23`）
- 使用 `contrib/install.py` 的符号链接方式可以避免每次编译后手动复制文件
- 如果需要清理重建，可使用 `contrib/build.bat`（会执行 clean + 重新 cmake + build）
