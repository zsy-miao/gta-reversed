---
name: explore-subsystem
description: Deep exploration of a gta-reversed subsystem - lists files, reads key headers, identifies class hierarchies, checks InjectHooks registration, and reports reverse engineering progress.
context: fork
user_invocable: true
arguments:
  - name: subsystem
    description: "Subsystem name or path to explore (e.g., Tasks, Entity, Audio, Scripts, Animation, Collision, Events, Core, Pools)"
    required: true
---

# Explore Subsystem: {{ subsystem }}

You are a code exploration agent for the gta-reversed project (GTA San Andreas reverse engineering).

## Your Task

Deeply explore the **{{ subsystem }}** subsystem and produce a comprehensive report.

## Steps

1. **Locate the subsystem**: Search `source/game_sa/` for directories matching "{{ subsystem }}". Also check `source/extensions/`, `source/reversiblehooks/`, `source/reversiblebugfixes/`.

2. **List all files**: Use Glob to find all `.h` and `.cpp` files. Report total count.

3. **Read key headers**: Read the main base class header(s) first. Identify:
   - Class hierarchy (inheritance tree)
   - Key virtual methods and their signatures
   - Important enums and constants
   - Member variables that define state

4. **Check InjectHooks registrations**: Grep `source/InjectHooksMain.cpp` for this subsystem's section. Count how many functions are hooked.

5. **Identify patterns**: Look for:
   - `classof()` static methods (LLVM-style casting support)
   - `VALIDATE_SIZE` calls (struct layout verification)
   - `StaticRef<>` usage (static memory access)
   - `notsa::IsFixBugs()` usage (bug fixes)
   - Helper functions marked `// NOTSA`

6. **Report reverse engineering progress**: For each major class, check if the `.cpp` file has substantial implementation or is mostly stubs.

## Output Format

```
# {{ subsystem }} Subsystem Report

## File Statistics
- Total files: N (.h: X, .cpp: Y)
- Directory structure: ...

## Class Hierarchy
<ASCII tree diagram>

## Key Interfaces
<For each major class: virtual methods, important members>

## Patterns Used
<List patterns found with examples>

## Reverse Engineering Progress
<Table: Class | Functions Hooked | Status>

## Navigation Guide
<Key files to read for understanding this subsystem>
```

## Rules
- ONLY use read-only tools: Read, Glob, Grep, Bash (for git log/wc only)
- Do NOT modify any files
- Be thorough but concise - focus on architectural understanding
