---
paths:
  - "source/game_sa/Animation/**"
---

# Animation System

24 files in `source/game_sa/Animation/`. Manages skeletal animation blending for peds and objects.

## Class Hierarchy

```
CAnimManager (static) - global animation manager
  CAnimBlock - IFP animation file container
    CAnimBlendHierarchy - single animation clip (walk, run, etc.)
      CAnimBlendSequence - per-bone keyframe data
        AnimSequenceFrames - keyframe types (KR, KRT, compressed variants)

CAnimBlendAssociation - runtime animation instance (playing on a ped)
  CAnimBlendStaticAssociation - shared static data
  CAnimBlendNode - per-bone blend state

CAnimBlendAssocGroup - group of related animations (e.g., all walk variants)
CAnimBlendClumpData - animation state attached to an RW clump
CAnimBlendFrameData - per-bone frame data (links to RW frame)
```

## Key Concepts

1. **IFP Files**: Animation packs loaded by CAnimManager. Each contains multiple hierarchies.
2. **Hierarchies**: A single animation (e.g., "walk_player"). Contains sequences per bone.
3. **Associations**: Runtime instances - an animation playing on a ped with blend weight/speed.
4. **Groups** (AssocGroup): Categorize animations by ped type/style (normal, fat, muscular, etc.).
5. **Blending**: Multiple associations active simultaneously, blended by weight per bone.

## CAnimManager (Static)

- `Initialise()` / `Shutdown()` - lifecycle
- `GetAnimation(name/id)` - look up hierarchy
- `GetAnimBlock(name)` - find IFP block
- `AddAnimation()` / `BlendAnimation()` - create associations on a clump
- `RemoveAnimBlock()` / `RequestModel()` - streaming integration

## Animation Descriptors

- `AnimAssocDescriptions.h` - animation group descriptions
- `AnimAssocAnimations.h` - individual animation definitions
- `AnimationStyleDescriptor.h` - per-style animation set

## Partial Body Animation

Animations can affect only specific bones (upper body, lower body, single limb) via bone masks in CAnimBlendNode. Allows walking + shooting simultaneously.

## Navigation Guide

1. `AnimManager.h` - start here for the global API
2. `AnimBlendHierarchy.h` + `AnimBlendSequence.h` - data model
3. `AnimBlendAssociation.h` - runtime playback
4. `AnimBlendAssocGroup.h` - animation categorization
