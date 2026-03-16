---
paths:
  - "source/extensions/**"
---

# Extensions

19 files in `source/extensions/`. Modern C++ utilities used across the project.

## Type Casting (Casting.hpp)

LLVM-style casting in `notsa::` namespace:
```cpp
notsa::isa<T>(ptr)              // type check, returns bool
notsa::cast<T>(ptr)             // asserts on failure
notsa::dyn_cast<T>(ptr)         // returns std::optional (nullptr-safe)
notsa::cast_if_present<T>(ptr)  // cast or nullptr
notsa::dyn_cast_if_present<T>(ptr)
```

Requires `classof()` static method on target type. Supported by: CTask, CEvent, entity types.

## Entity References (EntityRef.hpp)

Safe entity reference wrapper that integrates with CEntity's reference system. Prevents dangling pointers when entities are destroyed.

## Wrapped Enum (WEnum.hpp)

Type-safe enum wrapper with string conversion and iteration support. Used for enums that need runtime inspection (debug menu, serialization).

## Fixed-Point Types

| File | Description |
|------|-------------|
| `FixedFloat.hpp` | Fixed-point float with configurable precision |
| `FixedVector.hpp` | CVector with fixed-point components |
| `FixedQuat.hpp` | CQuaternion with fixed-point components |
| `FixedRect.hpp` | CRect with fixed-point components |

Used for compressed data in save files and network packets.

## Utilities

| File | Description |
|------|-------------|
| `utility.hpp` | `notsa::contains()`, range helpers, misc utilities |
| `random.hpp` | Random number generation wrappers |
| `ci_string.hpp` | Case-insensitive string comparison |
| `Singleton.hpp` | CRTP singleton pattern |
| `File.hpp` | File I/O helpers |
| `CommandLine.h` | Command-line argument parsing |
| `Configuration.hpp` | Configuration management |

## Shapes

`Shapes/AngledRect.hpp` - rotated rectangle (not axis-aligned).

## Configs

`Configs/FastLoader.hpp`, `Configs/Miscellaneous.hpp` - feature configuration.

## Navigation Guide

1. `Casting.hpp` - most important, used project-wide
2. `EntityRef.hpp` - safe entity references
3. `utility.hpp` - common helpers
4. `WEnum.hpp` - enum utilities for debug/serialization
