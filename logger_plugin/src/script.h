#pragma once
#include <cstdint>
#include <cstdio>

// ============================================================================
// Script command types
// ============================================================================
enum class CmdType : uint8_t {
    Nop,
    Wait,       // wait <ms>
    Teleport,   // teleport <x> <y> <z>
    Pad,        // pad <field> <value> <duration_ms>
    PadMulti,   // padmulti <duration_ms> <field1> <val1> [field2 val2 ...]
    Log,        // log <message>
};

struct PadEntry {
    int    offset; // byte offset within CControllerState
    int16_t value;
};

struct ScriptCommand {
    CmdType type = CmdType::Nop;

    // Teleport
    float tx = 0, ty = 0, tz = 0;

    // Wait / Pad duration
    uint32_t durationMs = 0;

    // Pad (single)
    PadEntry padEntry = {};

    // PadMulti
    PadEntry padEntries[8] = {};
    int      padCount = 0;

    // Log / raw text (also used for display)
    char text[256] = {};
};

// Parse script file into command array. Returns number of commands loaded.
int  LoadScript(const char* path, ScriptCommand* out, int maxCmds, FILE* log);

// Execute commands sequentially (blocking, call from background thread).
// Sets *stop = true causes early termination.
void ExecuteScript(ScriptCommand* cmds, int count, volatile bool* stop, FILE* log);
