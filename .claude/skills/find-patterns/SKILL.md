---
name: find-patterns
description: "Finds and categorizes recurring code patterns across the codebase: searches for examples, analyzes variations, and produces a summary."
context: fork
user_invocable: true
arguments:
  - name: pattern
    description: "The code pattern to find (e.g., 'classof', 'VALIDATE_SIZE', 'IsFixBugs', 'StaticRef', 'RH_ScopedInstall')"
    required: true
---

# Find Patterns: {{ pattern }}

You are a code pattern analysis agent for the gta-reversed project (GTA San Andreas reverse engineering).

## Your Task

Find and categorize all uses of the **{{ pattern }}** pattern across the codebase.

## Steps

1. **Search broadly**: Grep for "{{ pattern }}" across the entire codebase. Note total match count.

2. **Sample diverse examples**: Read 5-10 representative examples from different subsystems to understand variations.

3. **Categorize variations**: Group the pattern uses by:
   - Usage style (e.g., different macro forms, different parameter patterns)
   - Subsystem (Entity, Tasks, Scripts, etc.)
   - Complexity (simple vs. complex usage)

4. **Identify conventions**: Determine:
   - Is there a canonical/preferred form?
   - Are there deprecated or discouraged variations?
   - What are the common mistakes or anti-patterns?

5. **Statistics**: Count occurrences by category.

## Output Format

```
# Pattern: {{ pattern }}

## Summary
<What this pattern does and why it exists>

## Statistics
- Total occurrences: N
- Files containing pattern: M

## Variations
### Variation 1: <name>
<Description, example code, count>

### Variation 2: <name>
<Description, example code, count>

## Conventions
<The correct/preferred way to use this pattern>

## Examples by Subsystem
<Table: Subsystem | Count | Example File>
```

## Rules
- ONLY use read-only tools: Read, Glob, Grep
- Do NOT modify any files
- Limit output to the most informative examples, not every occurrence
