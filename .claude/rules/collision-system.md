---
paths:
  - "source/game_sa/Collision/**"
---

# Collision System

38 files in `source/game_sa/Collision/`. Collision detection and response.

## Collision Model Architecture

```
CColModel - composite collision model
  CCollisionData - actual collision geometry
    CColSphere[]   - bounding spheres
    CColBox[]      - axis-aligned boxes
    CColTriangle[] - triangle mesh
    CColLine[]     - line segments (for wheels, etc.)
    CColDisk[]     - disk shapes

CColPoint - collision contact point (position, normal, surface type)
CColStore - streaming manager for collision data
```

## Primitive Types

| Class | Description |
|-------|-------------|
| `CColSphere` | center + radius, with surface type |
| `CColBox` | min/max corners (AABB) |
| `CColTriangle` | 3 vertex indices + surface |
| `CColTrianglePlane` | triangle with precomputed plane |
| `CColLine` | start/end points |
| `CColDisk` | circle in 3D space |
| `CSphere` / `CBox` | simpler math primitives |
| `CBoundingBox` | axis-aligned bounding box |

## CCollision (Static)

Main entry point for collision tests. Static methods:
- `TestSphereTriangle()`, `TestLineSphere()`, `TestLineTriangle()` - intersection tests
- `ProcessSphereTriangle()`, `ProcessLineTriangle()` - full collision processing with contact info
- `ProcessColModels()` - test two CColModel against each other
- `CalculateTrianglePlanes()` - precompute triangle planes

## CColModel

The composite model attached to each entity. Contains:
- Bounding box and bounding sphere (quick rejection)
- CCollisionData with detailed geometry
- Shared across instances of the same model

## CColStore

Streaming manager for collision data. Works with CStreaming to load/unload collision files (`.col`) per area. Indexed by area ID.

## CColPoint

Result of collision detection:
- `m_vecPoint` - contact position
- `m_vecNormal` - contact normal
- `m_nSurfaceTypeA/B` - surface types (eSurfaceType enum)
- `m_fDepth` - penetration depth

## CColSurface

Surface type data. Links to material properties (friction, sound, particle effects).

## Navigation Guide

1. `Collision.h` - static collision test/process functions
2. `ColModel.h` - composite collision model
3. `CollisionData.h` - geometry data container
4. Primitives: `ColSphere.h`, `ColBox.h`, `ColTriangle.h`
5. `ColStore.h` - streaming integration
6. `ColPoint.h` - collision results
