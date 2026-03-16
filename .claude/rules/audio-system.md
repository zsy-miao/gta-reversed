---
paths:
  - "source/game_sa/Audio/**"
---

# Audio System

116 files in `source/game_sa/Audio/`. All classes use the `AE` (Audio Engine) prefix.

## Directory Structure

```
Audio/
  config/     - audio configuration data
  data/       - audio data structures
  entities/   - audio entity classes (per-object audio)
  hardware/   - low-level audio output (DirectSound, OpenAL)
  loaders/    - audio file loading (SFX banks, streams)
  managers/   - high-level audio managers
  Enums/      - audio enumeration types
  OpenAL/     - OpenAL backend implementation
```

## Layered Architecture

1. **Managers** (top): CAEAudioEnvironment, CAESoundManager, CAEMusicManager, CAECutsceneTrackManager
2. **Entities** (mid): CAEAudioEntity base -> CAEPedAudioEntity, CAEVehicleAudioEntity, CAEWeaponAudioEntity, etc.
3. **Loaders** (mid): CAEBankLoader, CAEStreamingDecoder, CAEStreamThread
4. **Hardware** (low): CAEAudioHardware, CAEStaticChannel, CAEStreamingChannel
5. **Config/Data**: Audio configuration tables, sound definitions

## CAEAudioEntity

Base class for objects that emit sound. Subclasses:
- `CAEPedAudioEntity` - footsteps, voice, impacts
- `CAEVehicleAudioEntity` - engine, horn, crash, radio
- `CAEWeaponAudioEntity` - gunshots, impacts
- `CAEDoorAudioEntity` - door sounds
- `CAECollisionAudioEntity` - collision sounds
- `CAEExplosionAudioEntity` - explosions
- `CAEFireAudioEntity` - fire crackling
- `CAEWaterCannonAudioEntity` - water cannon

## Audio Event Flow

1. Game code calls entity audio method (e.g., `PlayFootstepSound()`)
2. Entity creates sound request with parameters
3. `CAESoundManager` manages active sounds, handles priority/distance culling
4. `CAEAudioHardware` outputs via DirectSound or OpenAL

## Naming Convention

All audio classes: `CAE*` prefix (C + AE = Audio Engine).
All audio enums: `eAudio*` or in `Audio/Enums/` directory.

## Navigation Guide

1. `entities/` - understand per-object audio pattern
2. `managers/` - high-level control flow
3. `hardware/` - output abstraction layer
4. `Enums/` - audio event type catalogs
