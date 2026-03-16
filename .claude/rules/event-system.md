---
paths:
  - "source/game_sa/Events/**"
---

# Event System

177 files in `source/game_sa/Events/`. ~94 event types that drive ped AI responses.

## CEvent Base (Event.h, 0xC)

Members: `m_nTimeActive` (int32), `m_bValid` (bool).

Pure virtual interface:
- `GetEventType()` - returns eEventType enum
- `GetEventPriority()` - higher = more important
- `GetLifeTime()` - ticks before expiry
- `Clone()` - deep copy

Virtual with defaults:
- `AffectsPed(ped)` / `AffectsPedGroup(group)` - filtering (default: true)
- `IsCriminalEvent()` / `ReportCriminalEvent(ped)` - crime system integration
- `HasEditableResponse()` - CEventEditableResponse returns true
- `GetSourceEntity()` - entity that caused the event
- `TakesPriorityOver(other)` - priority comparison
- `GetLocalSoundLevel()` - for audio-driven events
- `DoInformVehicleOccupants(ped)` - propagate to vehicle passengers
- `IsValid(ped)` - validity check (timeout or explicit)
- `CanBeInterruptedBySameEvent()` - duplicate event handling

## LLVM-Style Casting

Events support `classof()` with special handling for `CEventEditableResponse`:
```cpp
// Each event: static constexpr auto Type = EVENT_*;
// CEventEditableResponse detected via HasEditableResponse()
notsa::isa<CEventDamage>(event)
notsa::dyn_cast<CEventEditableResponse>(event)
```

## CEventEditableResponse

Subclass of CEvent that allows the task system to modify the response. Used for events where the ped's reaction can be customized (damage, threats, etc.).

## Event Dispatch Flow

1. Event created (e.g., `CEventDamage`, `CEventGunShot`)
2. `CEventHandler` receives and prioritizes events
3. Events filtered by `AffectsPed()` / `IsValid()`
4. Winning event triggers task creation via `CEventHandler`
5. Task assigned to CTaskManager's EVENT_RESPONSE_TEMP or EVENT_RESPONSE_NONTEMP slot

## Common Event Categories

- **Damage**: CEventDamage, CEventGunShot, CEventGotKnockedOverByCar
- **Threat**: CEventSeenPanickedPed, CEventAcquaintancePedHate, CEventDanger
- **Social**: CEventChat, CEventLeaderEnteredCarAsDriver, CEventGroupEvent
- **Vehicle**: CEventCarUpsideDown, CEventPassObject, CEventVehicleDamage
- **Script**: CEventScriptCommand, CEventCreatePartnerTask

## Navigation Guide

1. `Event.h` - base class interface
2. `EventEditableResponse.h` - editable response pattern
3. `EventHandler.h` / `EventHandlerHistory.h` - dispatch infrastructure
4. Individual events in `Events/` directory (flat structure, no subdirs)
