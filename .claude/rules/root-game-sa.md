---
paths:
  - "source/game_sa/*.h"
  - "source/game_sa/*.cpp"
---

# Root game_sa Files

~531 files directly in `source/game_sa/` (not in subdirectories). Contains core singletons, managers, and fundamental game systems.

## Key Singletons

| Class | Description |
|-------|-------------|
| CCamera | Camera system, player view control |
| CWorld | World management, entity sectors, line-of-sight |
| CStreaming | Asset streaming (models, textures, animations) |
| CWeather | Weather state machine, fog, rain, sun |
| CClock | Game time management |
| CPopulation | Ped/vehicle spawning and population control |
| CCarCtrl | Vehicle AI traffic control |
| CPathFind | Pathfinding nodes and routes |
| CMenuManager | Menu system, settings |
| CGarages | Garage system |
| CStats | Player statistics |
| CHud | HUD rendering |
| CRadar | Minimap and radar blips |

## Important Manager Classes

- `CModelInfo` - model information database
- `CStreaming` - streaming manager for all assets
- `CFileLoader` - IPL/IDE file parsing
- `CTimeCycle` - time-of-day lighting
- `CWaterLevel` - water plane management
- `CFireManager` - fire instances
- `CExplosion` - explosion effects
- `CProjectileInfo` - projectile tracking
- `CPickups` - pickup items
- `CRestart` - respawn management
- `CMessages` - on-screen text messages

## Vehicle-Related (root level)

- `CCarAI` - vehicle AI decision making
- `CCarCtrl` - traffic spawning/control
- `CCarEnterExit` - enter/exit vehicle logic
- `CCarGenerator` - placed vehicle generators
- `CDamageManager` - vehicle damage state
- `CAutoPilot` - vehicle autopilot AI

## Ped-Related (root level)

- `CPedIntelligence` - ped decision making (wraps CTaskManager + CEventHandler)
- `CPedGroups` / `CPedGroupMembership` - group AI
- `CPedStats` - ped stat definitions
- `CAccident` / `CAccidentManager` - injury tracking
- `CCover` / `CCoverPoint` - cover system

## Common Patterns

- Most "C*" classes use `static inline auto& member = StaticRef<Type, 0xAddr>()` for GTA memory
- Singletons accessed via static methods or global references
- `RH_ScopedCategoryGlobal()` used in InjectHooks() for these files

## Navigation Guide

- For gameplay systems, start with the manager class header
- Use `docs/hooks.csv` to find function addresses: `grep "ClassName" docs/hooks.csv`
- Check `InjectHooksMain.cpp` for reversal status of specific classes
