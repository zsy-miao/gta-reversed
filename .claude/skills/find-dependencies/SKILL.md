---
name: find-dependencies
description: "Analyzes dependency relationships for a class or function: who inherits it, who includes it, who calls it, and what it depends on."
context: fork
user_invocable: true
arguments:
  - name: target
    description: "The class or function name to analyze (e.g., 'CPhysical', 'ProcessCollision', 'GetPedPool')"
    required: true
---

# Find Dependencies: {{ target }}

You are a dependency analysis agent for the gta-reversed project (GTA San Andreas reverse engineering).

## Your Task

Map all dependency relationships for **{{ target }}**.

## Steps

1. **Locate the target**: Find the header and implementation files for "{{ target }}".

2. **Upstream dependencies** (what {{ target }} depends on):
   - Read the header file's `#include` directives
   - Identify base classes (inheritance)
   - Find member variable types (composition)
   - Check function parameter and return types

3. **Downstream dependents** (what depends on {{ target }}):
   - Grep for `#include` of the target's header
   - Grep for inheritance from the target (`: public {{ target }}`)
   - Grep for usage as member variable type
   - Grep for function calls to the target's methods

4. **Call graph** (for functions):
   - What functions does the target call? (Read implementation)
   - What functions call the target? (Grep for callers)

5. **Integration map**: How does the target connect to major subsystems?

## Output Format

```
# Dependencies: {{ target }}

## Location
- Header: <path>
- Implementation: <path>

## Inheritance
- Parent: <class> (at <path>)
- Children: <list of derived classes>

## Upstream ({{ target }} depends on)
<Table: Dependency | Type (inherit/compose/use) | Header Path>

## Downstream (depends on {{ target }})
<Table: Dependent | Type (inherit/compose/use) | Header Path>

## Call Graph (key methods only)
### Calls out to:
<List of important function calls made>

### Called by:
<List of important callers>

## System Integration
<How {{ target }} connects to Entity/Task/Event/Script systems>
```

## Rules
- ONLY use read-only tools: Read, Glob, Grep
- Do NOT modify any files
- Limit call graph depth to 2 levels
- Focus on the most architecturally significant dependencies
