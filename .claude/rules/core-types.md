---
paths:
  - "source/game_sa/Core/**"
---

# Core Types

41 files in `source/game_sa/Core/`. Fundamental math types, containers, and memory management.

## Math Types

| Class | File | Description |
|-------|------|-------------|
| `CVector` | Vector.h | 3D vector (x, y, z float), interchangeable with `RwV3d` |
| `CVector2D` | Vector2D.h | 2D vector (x, y float) |
| `CMatrix` | Matrix.h | 4x3 transformation matrix, wraps `RwMatrix` |
| `CMatrixLink` | MatrixLink.h | Pool-allocated matrix (linked list) |
| `CQuaternion` | Quaternion.h | Rotation quaternion |
| `CRect` | Rect.h | 2D rectangle (left, top, right, bottom) |

`CVector` is the primary 3D type. Use it over `RwV3d`. `CMatrix` wraps RenderWare's matrix.

## Memory Pool

`CPool<T, S>` (Pool.h) - fixed-size memory pool with handle-based access:
- `New()` / `Delete()` - allocate/free
- `GetAt(index)` - get by pool index
- `GetRef(handle)` - get by handle (includes generation check)
- `GetIndex(ptr)` - get index from pointer
- Iterable: range-based for loop support
- `GetActiveCount()` / `GetSize()` - capacity info

Pool accessors in `Pools/`: `GetPedPool()`, `GetVehiclePool()`, `GetObjectPool()`, `GetBuildingPool()`.

## Container Types

| Class | Description |
|-------|-------------|
| `CLink<T>` / `CLinkList<T>` | Doubly-linked list with sentinel nodes |
| `CPtrList` | Base pointer list |
| `CPtrListSingleLink` | Singly-linked pointer list |
| `CPtrListDoubleLink` | Doubly-linked pointer list |
| `CPtrNode*` | Node types for pointer lists |
| `CEntryInfoList` / `CEntryInfoNode` | Entity sector entry tracking |
| `List_c` / `ListItem_c` | Alternative linked list (lowercase naming) |
| `SArray<T>` | Simple array wrapper |

## Spatial Data Structures

| Class | Description |
|-------|-------------|
| `CQuadTreeNode` | Quadtree for 2D spatial partitioning |
| `COctTree` / `COctTreeBase` | Octree for 3D spatial queries |
| `CStore<T, N>` | Fixed-capacity store (static array + count) |

## Other

- `CMatrixLinkList` - pool of CMatrixLink nodes
- `CKeyGen` - hash key generation (string -> uint32)

## Navigation Guide

1. `Vector.h` / `Matrix.h` - essential math types
2. `Pool.h` - memory pool pattern (used everywhere)
3. `Link.h` / `PtrList.h` - linked list containers
4. `Quaternion.h` / `Rect.h` - additional math
