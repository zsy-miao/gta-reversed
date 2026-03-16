---
paths:
  - "source/game_sa/Entity/**"
---

# Entity System

61 files in `source/game_sa/Entity/`. The core game object hierarchy.

## Class Hierarchy

```
CPlaceable (0x18) - position + matrix
  CEntity (0x38) - RwObject, flags, model, type/status
    CPhysical (0x138) - mass, velocity, forces, collision
      CPed - pedestrians (CPlayerPed, CCivilianPed, CCopPed, CEmergencyPed)
      CVehicle - vehicles (CAutomobile, CBike, CBoat, CHeli, CPlane, CTrain, CTrailer, CQuadBike, CMonsterTruck, CBmx)
      CObject - objects (CCutsceneObject, CHandObject, CProjectile)
    CBuilding (0x38) - static structures, no physics
    CDummy (0x38) - non-physical markers (CDummyObject, CDummyPed)
```

## Entity Type System

`CEntity` uses a 3-bit `m_nType` field (eEntityType): NOTHING=0, BUILDING=1, VEHICLE=2, PED=3, OBJECT=4, DUMMY=5.
5-bit `m_nStatus` (eEntityStatus): PLAYER=0, SIMPLE=2, PHYSICS=3, ABANDONED=4, WRECKED=5, etc.
32-bit flag union for visibility, collision, streaming, rendering flags.

## Key Virtual Methods (CEntity)

- `Add()`/`Remove()` - world sector management
- `SetModelIndex()`/`CreateRwObject()`/`DeleteRwObject()` - model setup
- `ProcessControl()`/`ProcessCollision()`/`ProcessShift()` - per-frame update
- `PreRender()`/`Render()`/`SetupLighting()` - rendering pipeline
- `Teleport()`, `FlagToDestroyWhenNextProcessed()`

## CPhysical Additions

- Movement: `m_vecMoveSpeed`, `m_vecTurnSpeed`, `ApplyMoveForce()`, `ApplyTurnForce()`
- Collision: `m_apCollidedEntities[6]`, `ProcessEntityCollision()`, `ApplyCollision()`
- Attachment: `AttachEntityToEntity()`, `m_pAttachedTo`, `PositionAttachedEntity()`
- Flags: bulletproof, fireproof, collisionproof, invulnerable, explosionproof

## Type Casting

Two patterns coexist:
1. **Enum-based**: `entity->GetType() == ENTITY_TYPE_VEHICLE` or `entity->GetIsTypeVehicle()`
2. **Reinterpret helpers**: `entity->AsVehicle()`, `entity->AsPed()`, `entity->AsObject()`
3. **LLVM-style** (via `extensions/Casting.hpp`): `notsa::isa<CVehicle>(entity)`, `notsa::dyn_cast<CPed>(entity)`

## Reference System

Entities use `RegisterReference()`/`CleanUpOldReference()` to prevent dangling pointers. Template helpers: `CEntity::ChangeEntityReference()`, `CEntity::SetEntityReference()`.

## Navigation Guide

1. Start with `Entity.h` for base flags and virtual interface
2. Read `Physical.h` for physics layer
3. Subclass headers: `Ped/Ped.h`, `Vehicle/Vehicle.h`, `Object/Object.h`
4. Check `Pools/` for `GetPedPool()`, `GetVehiclePool()` accessors
