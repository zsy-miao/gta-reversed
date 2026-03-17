# gta-reversed 项目的 DLL 注入机制

## 概述

本项目**本身不执行 DLL 注入**。它编译为一个标准的 Windows DLL，文件扩展名改为 `.asi`（通过 CMake 设置 `SUFFIX ".asi"`），依赖外部的 **ASI Loader**（GTA 模组社区常用的第三方工具）将其加载到 GTA San Andreas 进程中。加载后，DLL 的 `DllMain` 入口点被触发，项目自带的 Hook 系统会在运行中的 GTA 进程内存中，用 x86 `JMP` 指令覆盖数千个函数入口，将执行流重定向到逆向还原的 C++ 实现。

整个机制分为三层：

1. **加载层** -- ASI Loader 将 `gta_reversed.asi` 加载到 GTA SA 进程中
2. **注册层** -- `InjectHooksMain()` 调用 700 多个 `InjectHooks()` 方法，逐个注册函数到可逆 Hook 系统
3. **补丁层** -- 对每个注册的函数，在 GTA 原始函数的前 5 个字节写入 x86 `JMP`（操作码 `0xE9`），跳转到逆向实现。此过程在运行时可逆

## 核心类及其职责

| 类 / 文件 | 职责 | 关键方法 |
|---|---|---|
| `dllmain.cpp` | DLL 入口点；ASI Loader 加载 DLL 时由操作系统调用 | `DllMain()` |
| `InjectHooksMain.cpp` | 中央注册中心；调用每个类的 `InjectHooks()` | `InjectHooksMain(HMODULE)` |
| `HookSystem.h` | 底层 x86 JMP 补丁原语（遗留代码，仅用于少数特殊 Hook） | `HookInstall()` |
| `ReversibleHooks.h` | 宏定义和 `Install()` / `InstallVirtual()` 模板 API | `RH_ScopedInstall` 等宏 |
| `ReversibleHooks.cpp` | Hook 注册表、分类树管理、CSV 导出 | `detail::HookInstall()` |
| `ReversibleHook::Simple` | 管理单个非虚函数 Hook；保存原始字节和 JMP 补丁 | `Switch()`, `Check()` |
| `ReversibleHook::Virtual` | 管理虚函数 Hook；同时修补 vtable 条目和直接调用 | `Switch()` |
| `ReversibleHook::Base` | 所有 Hook 类型的抽象基类；跟踪 hooked/locked/reversed 状态 | `State()`, `Switch()` |

## 执行流程

### 第 1 步：ASI Loader 加载 DLL

GTA SA 模组社区使用的 ASI Loader（通常是 `dinput8.dll` 或 `vorbisFile.dll`，放在游戏目录中）是一个代理 DLL，游戏会将其作为合法依赖项加载。初始化时，ASI Loader 扫描游戏目录下所有 `*.asi` 文件，对每个文件调用 `LoadLibrary()`。由于 `.asi` 文件本质上就是重命名的 `.dll` 文件，Windows 会加载它们并调用其 `DllMain`。

### 第 2 步：`DllMain`（`source/dllmain.cpp:83`）

```
DLL_PROCESS_ATTACH:
  1. 检查 RenderWare 尚未启动（*(RwCamera**)0xC1703C 必须为 null）
  2. 设置控制台（AllocConsole, UTF-8）
  3. 解析命令行参数
  4. 从 gta-reversed.ini 加载配置
  5. 调用 InjectHooksMain(hModule)
  6. 应用命令行 Hook 覆盖选项（unhook-all, unhook-some 等）
```

第 90 行的检查（`if (*(RwCamera**)0xC1703C)`）至关重要：DLL 必须在游戏引擎初始化 RenderWare 之前加载，确保 Hook 在游戏调用原始函数之前就已到位。

### 第 3 步：`InjectHooksMain(HMODULE)`（`source/InjectHooksMain.cpp:1451`）

```cpp
void InjectHooksMain(HMODULE hThisDLL) {
    ReversibleHooks::OnInjectionBegin(hThisDLL);  // 保存 DLL 句柄，分配跟踪集合
    InjectHooksMain();                              // 调用 700+ 个 InjectHooks() 方法
    ReversibleHooks::OnInjectionEnd();              // 导出 hooks.csv，清理资源
}
```

内部的 `InjectHooksMain()`（第 532 行）逐个调用每个类的静态 `InjectHooks()` 方法。

### 第 4 步：每个类的 `InjectHooks()` 注册各个函数

典型的 `InjectHooks()` 使用 `RH_ScopedInstall` 等宏，展开后调用 `ReversibleHooks::Install()`。例如：

```cpp
RH_ScopedInstall(SomeMethod, 0x532AE0);
// 展开为：
ReversibleHooks::Install("Category/ClassName", "SomeMethod", 0x532AE0, &ClassName::SomeMethod);
```

### 第 5 步：实际的 x86 补丁（`ReversibleHook::Simple` 构造函数）

对每个函数，创建一个 `Simple` Hook 对象。构造函数执行以下操作：

1. **计算 JMP 偏移量**：`jumpLocation = 目标地址 - 源地址 - 5`（5 = x86 JMP 指令大小）
2. **用 NOP 填充剩余字节**（操作码 `0x90`），如果被覆盖的原始指令大于 5 字节
3. **保存 GTA 地址处的原始字节**（`m_OriginalFunctionContent`）以便后续恢复
4. **处理 Hoodlum 破解版的兼容问题**：某些破解版游戏可执行文件由于 SecuROM 保护移除，在特定地址有 NOP + JMP 的模式。代码检测此模式并跟踪 JMP 找到真实目标地址
5. **写入 JMP 补丁**：通过 `memcpy` 写入 GTA 进程内存（先调用 `VirtualProtect` 使页面可写）

结果：当游戏调用地址 `0x532AE0` 处的原始函数时，CPU 命中 `JMP` 指令，立即重定向到 DLL 中的逆向 C++ 实现。

### 第 6 步：虚函数 Hook（`ReversibleHook::Virtual`）

对于虚函数，系统做两件事：
1. **修补 vtable 条目** -- 同时修改 GTA 的 vtable（在已知内存地址如 `0x863928`）和 DLL 自身的 vtable，使给定 vtable 索引处的函数指针指向逆向实现
2. **同时安装 Simple Hook** -- 在原始函数体上安装，捕获对同一函数的直接（非虚）调用

DLL 自身的 vtable 地址在运行时通过 `GetProcAddress()` 查找 MSVC 修饰符号 `??_7ClassName@@6B@` 获得。

## 运行时切换机制

本项目的核心特色是 Hook 在**运行时可逆**。`Simple::Switch()` 方法在两种状态之间切换：

- **已 Hook**（默认）：GTA 的函数被 JMP 到我们的代码。我们的函数正常运行
- **已取消 Hook**：恢复 GTA 的原始字节。同时在我们的函数上安装反向 JMP 跳回 GTA 原始代码——这意味着即使内部代码直接调用逆向函数，也会弹回到原始游戏代码

这种双向补丁通过存储以下数据实现：
- `m_HookContent` / `m_OriginalFunctionContent` -- GTA 地址处的 JMP 补丁和保存的原始字节
- `m_LibHookContent` / `m_LibOriginalFunctionContent` -- DLL 函数地址处的反向 JMP 补丁和保存的原始字节

ImGui 调试菜单（F7）展示分类树和复选框，开发者可以在游戏运行时实时切换单个函数的开/关。

## 数据结构

```
ReversibleHook::Base（抽象基类）
  ├── m_IsHooked    : bool   -- JMP 当前是否已安装？
  ├── m_IsLocked    : bool   -- 是否阻止 UI 切换？
  ├── m_IsReversed  : bool   -- 此函数是否已被逆向？
  └── m_Name        : string -- 例如 "ProcessControl"

ReversibleHook::Simple : Base
  ├── m_HookContent                : SHookContent  -- 写入 GTA 内存的 0xE9 JMP + NOP
  ├── m_OriginalFunctionContent    : uint8[52]     -- 保存的 GTA 原始字节
  ├── m_iRealHookedAddress         : uint32        -- GTA 函数地址（如 0x532AE0）
  ├── m_iLibFunctionAddress        : uint32        -- 我们 DLL 中的函数地址
  ├── m_LibHookContent             : SHookContent  -- 取消 Hook 时的反向 JMP
  └── m_LibOriginalFunctionContent : uint8[52]     -- 保存的 DLL 原始字节

SHookContent（紧凑结构，52 字节）：
  ├── jumpOpCode    : uint8   -- 始终为 0xE9（x86 相对 JMP）
  ├── jumpLocation  : uint32  -- 到目标的相对偏移量
  └── possibleNops  : uint8[47] -- NOP 填充
```

## 集成接口

- **ASI Loader**（外部）：通过 `LoadLibrary` 将 DLL 加载到 GTA SA 进程
- **ImGui 调试菜单**（`source/toolsmenu/`）：提供运行时 UI 切换 Hook 开/关
- **命令行**：支持 `--unhook-all`、`--unhook-except`、`--unhook-some` 标志控制初始 Hook 状态
- **配置文件**：`gta-reversed.ini` 持久化设置
- **编辑并继续支持**：`Simple::Check()` 检测 MSVC 调试器重新编译函数的情况并更新存储的字节
- **hooks.csv 导出**：注入完成后，所有 7965+ 个 Hook 被导出到 `hooks.csv` 供参考

## 关键源码参考

| 文件 | 行号 | 内容 |
|---|---|---|
| `source/dllmain.cpp` | 83-121 | DLL 入口点，一切的起点 |
| `source/InjectHooksMain.cpp` | 532-583, 1451-1455 | 中央 Hook 注册，调用所有 `InjectHooks()` |
| `source/HookSystem.h` | 1-97 | 遗留底层 x86 JMP 安装（少数特殊情况使用） |
| `source/reversiblehooks/ReversibleHooks.h` | 1-186 | 宏定义（`RH_ScopedInstall` 等）和 `Install()` API |
| `source/reversiblehooks/ReversibleHooks.cpp` | 70-76, 174-197 | `OnInjectionBegin/End`、`detail::HookInstall` |
| `source/reversiblehooks/ReversibleHook/Simple.cpp` | 7-78 | 构造函数：实际的 x86 补丁逻辑（含 Hoodlum 兼容） |
| `source/reversiblehooks/ReversibleHook/Simple.cpp` | 153-167 | `Switch()`：运行时 Hook/取消 Hook 切换 |
| `source/reversiblehooks/ReversibleHook/Virtual.cpp` | 7-40 | 虚函数 Hook：vtable + 直接调用补丁 |
| `source/reversiblehooks/ReversibleHook/Base.h` | 1-78 | 所有 Hook 类型的基类 |
| `source/CMakeLists.txt` | 54, 64 | 构建为 `MODULE` 库，后缀 `.asi` |
