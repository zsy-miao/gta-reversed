# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A project to completely reverse engineer GTA San Andreas. Builds as a DLL (.asi) that injects into the game via ASI loader, replacing original functions with reversed implementations. Functions can be toggled between original and reversed code at runtime. C++23, x86/Win32 only.

## Build Commands

```bash
# First-time setup (installs Conan deps + generates CMake project)
python setup.py                          # Debug, unity build (default)
python setup.py --no-unity-build         # Debug, no unity build
python setup.py --buildconf Release      # Release build

# Build from command line
cmake --build build

# Open in Visual Studio
# Open build/GTASA.sln (or build/GTASA.slnx)
```

**Requirements**:
- [Visual Studio 2026](https://visualstudio.microsoft.com/downloads/) (default; VS 2022 also works but requires changing `compiler.version` to `194` in `conanprofile.txt`)
- [Python](https://www.python.org/downloads/) >= 3.x
- [Conan](https://docs.conan.io/2/installation.html) >= 2.x (`pip install conan`; first install also run `conan profile detect`)
- [CMake](https://cmake.org/download/) >= 4.20 (Conan auto-downloads cmake/4.2.0 via `conanprofile.txt` `[replace_tool_requires]`)

**Output**: `bin/<config>/gta_reversed.asi` — DLL injected into GTA SA via ASI loader.

**Note on cmake path**: `cmake` may not be in the system PATH. Conan auto-downloads it to `~/.conan2/p/cmake*/p/bin/cmake.exe`. To find and use it:
```bash
# Find the Conan-installed cmake
find ~/.conan2 -name "cmake.exe" -type f 2>/dev/null
# Build using the full path (example, actual hash varies)
/c/Users/admin/.conan2/p/cmake20ac4e0137fee/p/bin/cmake.exe --build build
```

**Game EXE**: Must use GTA SA **1.0 US exe**. Both the compact (5.0 MiB) and full-size (13.7 MiB) variants work, as long as memory addresses match. See [Steam guide](https://steamcommunity.com/sharedfiles/filedetails/?id=3036381821) for downgrader details.

**Installation**: Run `contrib/install.py` with administrator privileges — it unpacks `contrib/plugins.zip` (ASI loader + mouse fix) into the game directory and creates symlinks from `<game>/scripts/` to `bin/<config>/`, so rebuilds are automatically picked up. Alternatively, manually install:
```bash
# 1. Unpack plugins
python -c "import zipfile; zipfile.ZipFile('contrib/plugins.zip').extractall('<game_dir>')"
# 2. Create scripts dir and symlinks (requires admin)
mkdir -p <game_dir>/scripts
mklink "<game_dir>\scripts\gta_reversed.asi" "<repo>\bin\Debug\gta_reversed.asi"
mklink "<game_dir>\scripts\gta_reversed.pdb" "<repo>\bin\Debug\gta_reversed.pdb"
# 3. Set environment variables
# GTA_SA_DIR = <game_dir>
# GTA_SA_EXE = <game_dir>\gta_sa.exe
```

There is no test suite. Testing is done by injecting the DLL into the game and verifying behavior in-game. Use the debug menu (F7) for quick access.

## Architecture

### Hook System — The Core Pattern

Every reversed function is registered via the hook system. Each `.cpp` file has a static `InjectHooks()` method:

```cpp
void CEntity::InjectHooks() {
    RH_ScopedVirtualClass(CEntity, 0x863928, 22);  // class, GTA vtable addr, num vfuncs
    RH_ScopedCategory("Entity");                     // follows directory layout

    RH_ScopedInstall(SomeMethod, 0x532AE0);          // member function at GTA address
    RH_ScopedVMTInstall(Render, 0x534310);            // virtual function hook
    RH_ScopedGlobalInstall(SomeGlobal, 0xABCDEF);    // free function
    RH_ScopedOverloadedInstall(Add, "rect", 0x5347D0, void(CEntity::*)(const CRect&));
}
```

All `InjectHooks()` calls are centralized in `source/InjectHooksMain.cpp`. The category in `RH_ScopedCategory` should match the directory path (e.g., files in `Entity/` use `"Entity"`, files in root `game_sa/` use `RH_ScopedCategoryGlobal()`).

### Source Layout

- **`source/game_sa/`** — Main reversed game code (~1200+ files), organized by subsystem
- **`source/extensions/`** — C++ utilities: `Casting.hpp` (LLVM-style `isa<>/cast<>/dyn_cast<>`), `random.hpp`, `WEnum.hpp`, `EntityRef.hpp`, etc.
- **`source/reversiblehooks/`** — Hook management infrastructure (runtime toggle between GTA/reversed code)
- **`source/reversiblebugfixes/`** — Runtime-toggleable bug fixes
- **`source/toolsmenu/`** — ImGui-based debug menu (F7 in-game)
  - `DebugModules/` — Each module extends `DebugModule` base class in `notsa::debugmodules` namespace
- **`source/app/`** — Platform layer, D3D9 rendering, window management
- **`source/HookSystem.h`** — Low-level x86 JMP hooking primitives
- **`source/Base.h`** — Core types (`int8`-`int64`, `uint8`-`uint64`), `VALIDATE_SIZE`, `StaticRef<T>`
- **`source/StdInc.h`** — Precompiled header included everywhere

### Entity Hierarchy

```
CPlaceable → CEntity → CPhysical ─┬→ CPed (→ CPlayerPed, CCivilianPed, CCopPed, CEmergencyPed)
                                   ├→ CVehicle (→ CAutomobile, CBike, CBoat, CHeli, CPlane, CTrain...)
                                   ├→ CObject (→ CCutsceneObject, CProjectile)
                                   └→ CBuilding
                       CDummy (→ CDummyObject, CDummyPed)
```

### Key Game Systems

- **Tasks** (`game_sa/Tasks/`): `CTask` → `CTaskSimple` / `CTaskComplex`. 70+ concrete types in `TaskTypes/`. `CTaskManager` manages 5 primary + 6 secondary task slots per ped.
- **Events** (`game_sa/Events/`): `CEvent` base with 130+ types. `CEventHandler` dispatches events to peds.
- **Scripts** (`game_sa/Scripts/`): `CRunningScript` executes SCM bytecode. Commands in `Commands/` subdirectory with template-based parser (`CommandParser/`).
- **Pools** (`game_sa/Pools/`): Template `CPool<T,S>` for memory management. Access via `GetPedPool()`, `GetVehiclePool()`, etc.
- **Audio** (`game_sa/Audio/`): Subsystem with Config/Entities/Hardware/Loaders/Managers subdirectories.

### Type Casting

Uses LLVM-style casting from `extensions/Casting.hpp` (in `notsa::` namespace):
```cpp
if (notsa::isa<CAutomobile>(vehicle)) { ... }
auto* car = notsa::cast<CAutomobile>(vehicle);         // asserts on failure
auto* car = notsa::dyn_cast<CAutomobile>(vehicle);     // returns optional
```

Entity/Task types support this via `classof()` static method pattern.

## Code Conventions

### Naming
- **Classes**: `CClassName` (game classes), `cLowercase` (data structures)
- **Enums**: `eEnumName` with `e` prefix
- **Members**: `m_` prefix for non-static, `ms_` for static class members. No prefix in structs.
- **Globals**: `g_` for cross-file, `s_` for file-local
- No Hungarian notation

### Style
- 4-space indent, LF line endings, no column limit
- `auto` when type is obvious; `f` suffix on float literals (`1.0f`)
- Range-based for loops preferred; use `rng::` for `std::ranges`, `rngv::` for `std::views`
- `std::span` or `rngv::take` over manual index loops with count variables
- `constexpr` over macros; `static inline` over `extern` in headers
- Fixed-width types: `uint8`, `int32`, `uint32` (not `DWORD` outside Win32 code)
- `std::array` over C-style arrays
- `VALIDATE_SIZE(StructName, expectedSize)` to verify struct layouts match GTA memory

### Static Memory Access
```cpp
static inline auto& ms_FooCount = StaticRef<uint32, 0xDEADBEEF>();
```

### Bug Fixes
Bug fixes must be gated so they can be toggled:
```cpp
if (notsa::IsFixBugs()) {
    // fixed behavior
}
```
Or for compile-time: `#ifdef FIX_BUGS`. Reversible bug fixes are defined in `source/reversiblebugfixes/Bugs.hpp`.

### Script Commands
Script command handlers are registered per-category (`notsa::script::commands::*::RegisterHandlers()`), using a template-based command parser in `Scripts/CommandParser/`.

## Subsystem Quick Index

| Subsystem | Path | Files | Key Entry Points |
|-----------|------|-------|-----------------|
| Entity | `game_sa/Entity/` | 61 | CEntity.h, CPhysical.h, CPed.h, CVehicle.h |
| Tasks | `game_sa/Tasks/` | 644 | Task.h, TaskComplex.h, TaskSimple.h, TaskManager.h |
| Events | `game_sa/Events/` | 177 | Event.h, EventEditableResponse.h, EventHandler.h |
| Scripts | `game_sa/Scripts/` | 43 | CRunningScript.h, CTheScripts.h, Commands/ |
| Audio | `game_sa/Audio/` | 116 | entities/, managers/, hardware/, loaders/ |
| Animation | `game_sa/Animation/` | 24 | AnimManager.h, AnimBlendHierarchy.h |
| Collision | `game_sa/Collision/` | 38 | Collision.h, ColModel.h, ColStore.h |
| Core | `game_sa/Core/` | 41 | Vector.h, Matrix.h, Pool.h, Link.h |
| Pools | `game_sa/Pools/` | 19 | GetPedPool(), GetVehiclePool(), etc. |
| Fx | `game_sa/Fx/` | 120 | Particle effects system |
| Root game_sa | `game_sa/*.h/cpp` | 531 | CCamera, CWorld, CStreaming, CWeather |
| Extensions | `extensions/` | 19 | Casting.hpp, EntityRef.hpp, WEnum.hpp |
| Hooks | `reversiblehooks/` | 10 | ReversibleHooks.h, HookCategory.h |
| Bug Fixes | `reversiblebugfixes/` | 2 | Bugs.hpp, ReversibleBugFix.hpp |
| Tools Menu | `toolsmenu/` | ~30 | Debug menu (F7), DebugModules/ |
| App | `app/` | ~10 | Platform layer, D3D9, window |
| HookSystem | `HookSystem.h` | 1 | Low-level x86 JMP patching |
| InjectHooks | `InjectHooksMain.cpp` | 1 | Central hook registration |

All paths relative to `source/`. Use `docs/hooks.csv` (7965+ entries) to look up specific function addresses — grep it, don't load it whole.

@docs/CodingGuidelines.MD
