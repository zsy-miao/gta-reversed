---
paths:
  - "source/game_sa/Scripts/**"
---

# Script System

43 files in `source/game_sa/Scripts/`. SCM bytecode execution and command handling.

## Architecture

```
CTheScripts - global script manager (static singleton)
  CRunningScript - individual script executor (linked list)
    CommandParser/ - template-based command parsing
    Commands/ - command handler implementations
```

## CRunningScript

Executes GTA's SCM (Santa Cruz Machine) bytecode. Each running script maintains:
- Program counter (IP)
- Local variables / stack
- Wait state and condition result
- Script name for debugging

Key methods: `ProcessOneCommand()`, `UpdateCompareFlag()`, `CollectParameters()`.

## CTheScripts

Static manager for all scripts. Manages:
- Script pool (running scripts linked list)
- Global variables array
- Mission scripts
- External scripts (CLEO support)

## Command System

### CommandParser/ (Template-Based)

Modern C++ template system that auto-generates command parsing code:
- Strong type aliases map script types to C++ types
- Parameters auto-collected from script bytecode
- Return values auto-written back
- Reduces boilerplate vs. manual `CollectParameters()`/`StoreParameters()`

### Commands/

Command handlers organized in `Commands/Commands.hpp`. Categories registered via `notsa::script::commands::*::RegisterHandlers()`.

CLEO commands in `Commands/CLEO/` subdirectory for custom script extensions.

## Script Command Pattern

```cpp
// Old style (manual parameter collection)
case COMMAND_CREATE_PED: {
    CollectParameters(4);
    auto* ped = ... // create ped from params
    StoreParameters(1); // store handle
    return;
}

// New style (CommandParser template)
// Automatically handles parameter collection/storage
// See CommandParser/ for the template machinery
```

## Key Types

- `eScriptCommands` - command opcode enum
- `tScriptParam` - union type for script parameters (int/float/string/pointer)
- Script entity handles: wrapped integers referencing pool objects

## Navigation Guide

1. `CRunningScript.h` - script executor interface
2. `CTheScripts.h` - global manager
3. `CommandParser/` - understand the template parsing system
4. `Commands/Commands.hpp` - command category registry
