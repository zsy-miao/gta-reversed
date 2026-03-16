---
paths:
  - "source/game_sa/Tasks/**"
---

# Task System

644 files in `source/game_sa/Tasks/`. The ped AI behavior system with ~300 task types.

## Class Hierarchy

```
CTask (0x8) - base, has m_Parent pointer
  CTaskSimple (0x8) - leaf tasks, implements ProcessPed()
    CTaskSimpleRunAnim, CTaskSimpleStandStill, CTaskSimpleCarDrive, ...
  CTaskComplex (0xC) - composite tasks, has m_pSubTask
    CTaskComplexWander, CTaskComplexEnterCar, CTaskComplexDie, ...
```

## CTask Base (Task.h)

Pure virtual interface:
- `Clone()` - deep copy
- `GetSubTask()` - CTaskSimple returns nullptr, CTaskComplex returns m_pSubTask
- `IsSimple()` - type discriminator
- `GetTaskType()` - returns eTaskType enum
- `MakeAbortable(ped, priority, event)` - request early termination
- `StopTimer(event)` - pause internal timer

LLVM-style casting: each task has `static constexpr auto Type = TASK_*;` and `classof()` support.
Concept: `template<typename T> concept Task = std::is_base_of_v<CTask, T>;`

## CTaskSimple (TaskSimple.h)

Key method: `virtual bool ProcessPed(CPed* ped)` - returns true when task is finished.
Optional: `virtual bool SetPedPosition(CPed* ped)` - adjust ped position.

## CTaskComplex (TaskComplex.h)

Three lifecycle methods (all pure virtual):
1. `CreateFirstSubTask(ped)` - called once at start
2. `ControlSubTask(ped)` - called every tick; return new task to switch, same to continue, nullptr to finish
3. `CreateNextSubTask(ped)` - called when current subtask finishes

`SetSubTask()` manages subtask lifecycle (deletes old, sets parent).

## CTaskManager (TaskManager.h, 0x30)

Each CPed owns one CTaskManager with:
- **5 primary slots** (ePrimaryTasks): PHYSICAL_RESPONSE, EVENT_RESPONSE_TEMP, EVENT_RESPONSE_NONTEMP, PRIMARY, DEFAULT
- **6 secondary slots** (eSecondaryTask): ATTACK, DUCK, SAY, FACIAL_COMPLEX, PARTIAL_ANIM, IK

Key methods:
- `SetTask(task, slot)` / `SetTaskSecondary(task, slot)`
- `GetActiveTask()` - first non-null primary task (highest priority)
- `GetSimplestActiveTask()` - deepest leaf in active task tree
- `Find<T>(activeOnly)` - template search by task type
- `Has<T>()` / `HasAnyOf<Ts...>()` - type checks
- `ManageTasks()` - per-frame task tree management
- `FlushImmediately()` / `Flush()` - abort/delete all tasks

## Directory Structure

- `Tasks/` - base classes (Task.h, TaskSimple.h, TaskComplex.h, TaskManager.h)
- `Tasks/TaskTypes/` - 300 concrete task implementations
- `Tasks/TaskTypes/Interior/` - interior-specific tasks
- `Tasks/TaskTypes/SeekEntity/` - entity seeking tasks with PosCalculators
- `Tasks/Allocators/` - task allocation, including PedGroup allocators

## Abort Priority

`eAbortPriority`: LEISURE=0, URGENT=1, IMMEDIATE=2. Higher priority = harder to refuse.
