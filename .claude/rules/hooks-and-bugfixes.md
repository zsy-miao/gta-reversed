---
paths:
  - "source/reversiblehooks/**"
  - "source/reversiblebugfixes/**"
  - "source/InjectHooksMain.cpp"
  - "source/HookSystem.h"
---

# Hook System and Bug Fixes

The core infrastructure enabling runtime toggle between original GTA code and reversed implementations.

## Hook Macros (used in InjectHooks())

```cpp
// Class declaration macros
RH_ScopedClass(CClassName);                           // non-virtual class
RH_ScopedVirtualClass(CClassName, 0xVTBL, numVFuncs); // virtual class with vtable
RH_ScopedCategory("DirName");                          // matches source directory
RH_ScopedCategoryGlobal();                             // for root game_sa/ files

// Function installation macros
RH_ScopedInstall(MethodName, 0xAddress);               // member function
RH_ScopedVMTInstall(MethodName, 0xAddress);            // virtual method
RH_ScopedGlobalInstall(FuncName, 0xAddress);           // free function
RH_ScopedOverloadedInstall(Name, "suffix", 0xAddr, Signature); // overloaded function
```

## InjectHooksMain.cpp

Central registry. All `InjectHooks()` calls organized by subsystem sections. To check reversal status of a class, grep this file.

## reversiblehooks/ (10 files)

```
ReversibleHooks.h       - main include, RootHookCategory access
RootHookCategory.h      - root of hook category tree
HookCategory.h          - category node (matches directory structure)
ReversibleHook/
  Base.h                - hook base class (name, address, state)
  Simple.h              - standard function hook (JMP patch)
  Virtual.h             - vtable entry hook
  ScriptCommand.h       - script command hook
```

Runtime toggle: each hook can be individually enabled/disabled via the debug menu (F7).

## reversiblebugfixes/ (2 files)

- `Bugs.hpp` - registry of all toggleable bug fixes, each with name and description
- `ReversibleBugFix.hpp` - bug fix registration infrastructure

### Bug Fix Pattern

```cpp
// Runtime check (preferred)
if (notsa::IsFixBugs()) {
    // corrected behavior
} else {
    // original buggy behavior
}

// Compile-time (when runtime not possible)
#ifdef FIX_BUGS
// corrected behavior
#endif
```

Bug fixes are toggleable at runtime through the debug menu, allowing testing both behaviors.

## HookSystem.h

Low-level x86 JMP hooking primitives. Patches game code at specified addresses to redirect execution to reversed C++ implementations. Handles:
- 5-byte JMP patches
- Vtable replacement
- Hook state tracking (enabled/disabled)

## Navigation Guide

1. `HookSystem.h` - low-level patching mechanism
2. `reversiblehooks/ReversibleHooks.h` - hook management API
3. `InjectHooksMain.cpp` - see what's been reversed
4. `reversiblebugfixes/Bugs.hpp` - bug fix catalog
