#include "script.h"
#include "mem.h"
#include "pad.h"
#include <cstring>
#include <cstdlib>
#include <timeapi.h>

#pragma comment(lib, "winmm.lib")

// ============================================================================
// Log helper (same format as main.cpp LogWrite)
// ============================================================================
static void LogWrite(FILE* f, const char* fmt, ...) {
    if (!f) return;
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(f, "[%02d:%02d:%02d.%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");
    fflush(f);
}

// ============================================================================
// Trim leading/trailing whitespace in-place, return pointer to trimmed start
// ============================================================================
static char* Trim(char* s) {
    while (*s == ' ' || *s == '\t') s++;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n'))
        s[--len] = '\0';
    return s;
}

// ============================================================================
// LoadScript — parse automation.txt into ScriptCommand array
// ============================================================================
int LoadScript(const char* path, ScriptCommand* out, int maxCmds, FILE* log) {
    FILE* f = fopen(path, "r");
    if (!f) {
        LogWrite(log, "ERROR: Cannot open script file: %s", path);
        return 0;
    }

    char line[512];
    int count = 0;

    while (fgets(line, sizeof(line), f) && count < maxCmds) {
        char* s = Trim(line);

        // Skip empty lines and comments
        if (s[0] == '\0' || s[0] == '#')
            continue;

        ScriptCommand& cmd = out[count];
        cmd = {}; // zero-init
        strncpy(cmd.text, s, sizeof(cmd.text) - 1);

        // Parse command type
        char token[64] = {};
        const char* rest = s;
        if (sscanf(rest, "%63s", token) != 1)
            continue;
        rest += strlen(token);
        while (*rest == ' ' || *rest == '\t') rest++;

        if (_stricmp(token, "wait") == 0) {
            cmd.type = CmdType::Wait;
            if (sscanf(rest, "%u", &cmd.durationMs) != 1) {
                LogWrite(log, "PARSE ERROR: wait needs <ms>: %s", s);
                continue;
            }
        }
        else if (_stricmp(token, "teleport") == 0) {
            cmd.type = CmdType::Teleport;
            if (sscanf(rest, "%f %f %f", &cmd.tx, &cmd.ty, &cmd.tz) != 3) {
                LogWrite(log, "PARSE ERROR: teleport needs <x> <y> <z>: %s", s);
                continue;
            }
        }
        else if (_stricmp(token, "pad") == 0) {
            cmd.type = CmdType::Pad;
            char fieldName[64] = {};
            int value = 0;
            uint32_t ms = 0;
            if (sscanf(rest, "%63s %d %u", fieldName, &value, &ms) != 3) {
                LogWrite(log, "PARSE ERROR: pad needs <field> <value> <duration_ms>: %s", s);
                continue;
            }
            int off = PadFieldOffset(fieldName);
            if (off < 0) {
                LogWrite(log, "PARSE ERROR: unknown pad field '%s': %s", fieldName, s);
                continue;
            }
            cmd.padEntry.offset = off;
            cmd.padEntry.value = static_cast<int16_t>(value);
            cmd.durationMs = ms;
        }
        else if (_stricmp(token, "padmulti") == 0) {
            cmd.type = CmdType::PadMulti;
            uint32_t ms = 0;
            if (sscanf(rest, "%u", &ms) != 1) {
                LogWrite(log, "PARSE ERROR: padmulti needs <duration_ms> ...: %s", s);
                continue;
            }
            cmd.durationMs = ms;
            // Skip past the duration token
            while (*rest && *rest != ' ' && *rest != '\t') rest++;
            while (*rest == ' ' || *rest == '\t') rest++;

            // Parse field-value pairs
            cmd.padCount = 0;
            char fn[64];
            int val;
            while (cmd.padCount < 8 && sscanf(rest, "%63s %d", fn, &val) == 2) {
                int off = PadFieldOffset(fn);
                if (off < 0) {
                    LogWrite(log, "PARSE ERROR: unknown pad field '%s': %s", fn, s);
                    break;
                }
                cmd.padEntries[cmd.padCount].offset = off;
                cmd.padEntries[cmd.padCount].value = static_cast<int16_t>(val);
                cmd.padCount++;
                // Advance past field name
                while (*rest && *rest != ' ' && *rest != '\t') rest++;
                while (*rest == ' ' || *rest == '\t') rest++;
                // Advance past value
                while (*rest && *rest != ' ' && *rest != '\t') rest++;
                while (*rest == ' ' || *rest == '\t') rest++;
            }
            if (cmd.padCount == 0) {
                LogWrite(log, "PARSE ERROR: padmulti needs at least one field-value pair: %s", s);
                continue;
            }
        }
        else if (_stricmp(token, "log") == 0) {
            cmd.type = CmdType::Log;
            // rest already points to the message text (stored in cmd.text for display)
            strncpy(cmd.text, rest, sizeof(cmd.text) - 1);
        }
        else {
            LogWrite(log, "PARSE ERROR: unknown command '%s': %s", token, s);
            continue;
        }

        count++;
    }

    fclose(f);
    LogWrite(log, "Loaded %d commands from %s", count, path);
    return count;
}

// ============================================================================
// ReadPlayerPosition — read player world coordinates
// ============================================================================
static bool ReadPlayerPosition(Vec3& pos) {
    uintptr_t pedPtr = 0;
    if (!SafeRead(ADDR_PLAYER_PED_PTR, pedPtr) || pedPtr == 0)
        return false;

    uintptr_t matrixPtr = 0;
    if (SafeRead(pedPtr + OFF_MATRIX_PTR, matrixPtr) && matrixPtr != 0)
        return SafeRead(matrixPtr + OFF_MATRIX_POS, pos);

    return SafeRead(pedPtr + OFF_SIMPLE_POS, pos);
}

static void LogPlayerPos(FILE* log, const char* label, int idx, int total) {
    Vec3 pos;
    if (ReadPlayerPosition(pos))
        LogWrite(log, "[%d/%d] %s pos: x=%.2f y=%.2f z=%.2f", idx, total, label, pos.x, pos.y, pos.z);
    else
        LogWrite(log, "[%d/%d] %s pos: unavailable", idx, total, label);
}

// ============================================================================
// Command executors
// ============================================================================

static void ExecWait(uint32_t ms, volatile bool* stop, FILE* log, int idx, int total) {
    LogWrite(log, "[%d/%d] EXEC: wait %u", idx, total, ms);
    DWORD start = GetTickCount();
    while (GetTickCount() - start < ms && !*stop) {
        Sleep(1);
    }
    DWORD elapsed = GetTickCount() - start;
    LogWrite(log, "[%d/%d] DONE: wait %u (elapsed %ums)", idx, total, ms, elapsed);
}

static void ExecTeleport(float x, float y, float z, volatile bool* stop, FILE* log, int idx, int total) {
    LogWrite(log, "[%d/%d] EXEC: teleport %.2f %.2f %.2f", idx, total, x, y, z);
    LogPlayerPos(log, "BEFORE", idx, total);

    uintptr_t ped = 0;
    if (!SafeRead(ADDR_PLAYER_PED_PTR, ped) || !ped) {
        LogWrite(log, "[%d/%d] ERROR: teleport failed — no player ped", idx, total);
        return;
    }

    Vec3 pos = {x, y, z};

    // Write to matrix if available
    uintptr_t mat = 0;
    if (SafeRead(ped + OFF_MATRIX_PTR, mat) && mat) {
        SafeWrite(mat + OFF_MATRIX_POS, pos);
    }
    // Also write to simple transform position (backup)
    SafeWrite(ped + OFF_SIMPLE_POS, pos);

    LogWrite(log, "[%d/%d] DONE: teleport OK (ped=0x%08X, mat=0x%08X)", idx, total, ped, mat);
    LogPlayerPos(log, "AFTER", idx, total);
}

static void ExecPad(const PadEntry& entry, uint32_t ms, volatile bool* stop, FILE* log, int idx, int total) {
    LogWrite(log, "[%d/%d] EXEC: pad offset=0x%02X value=%d duration=%ums", idx, total, entry.offset, entry.value, ms);
    LogPlayerPos(log, "BEFORE", idx, total);
    DWORD start = GetTickCount();
    uint32_t writes = 0;
    while (GetTickCount() - start < ms && !*stop) {
        SafeWrite<int16_t>(ADDR_PAD0_NEW_STATE + entry.offset, entry.value);
        writes++;
        Sleep(1);
    }
    // Reset to zero
    SafeWrite<int16_t>(ADDR_PAD0_NEW_STATE + entry.offset, 0);
    DWORD elapsed = GetTickCount() - start;
    LogPlayerPos(log, "AFTER", idx, total);
    LogWrite(log, "[%d/%d] DONE: pad offset=0x%02X (elapsed %ums, writes ~%u)", idx, total, entry.offset, elapsed, writes);
}

static void ExecPadMulti(const PadEntry* entries, int count, uint32_t ms, volatile bool* stop, FILE* log, int idx, int total) {
    LogWrite(log, "[%d/%d] EXEC: padmulti %d fields, duration=%ums", idx, total, count, ms);
    LogPlayerPos(log, "BEFORE", idx, total);
    DWORD start = GetTickCount();
    uint32_t writes = 0;
    while (GetTickCount() - start < ms && !*stop) {
        for (int i = 0; i < count; i++) {
            SafeWrite<int16_t>(ADDR_PAD0_NEW_STATE + entries[i].offset, entries[i].value);
        }
        writes++;
        Sleep(1);
    }
    // Reset all fields to zero
    for (int i = 0; i < count; i++) {
        SafeWrite<int16_t>(ADDR_PAD0_NEW_STATE + entries[i].offset, 0);
    }
    DWORD elapsed = GetTickCount() - start;
    LogPlayerPos(log, "AFTER", idx, total);
    LogWrite(log, "[%d/%d] DONE: padmulti %d fields (elapsed %ums, writes ~%u)", idx, total, count, elapsed, writes);
}

// ============================================================================
// ExecuteScript — run all commands sequentially
// ============================================================================
void ExecuteScript(ScriptCommand* cmds, int count, volatile bool* stop, FILE* log) {
    // Raise timer resolution to 1ms so Sleep(1) actually sleeps ~1ms instead of ~15.6ms.
    // This is critical: at default resolution, pad writes happen only ~64/sec and lose
    // the race against UpdatePads() which zeroes NewState each frame.
    // At 1ms resolution we get ~1000 writes/sec, overwhelming UpdatePads() reliably.
    timeBeginPeriod(1);

    DWORD totalStart = GetTickCount();

    for (int i = 0; i < count && !*stop; i++) {
        const ScriptCommand& cmd = cmds[i];
        int idx = i + 1;

        switch (cmd.type) {
        case CmdType::Wait:
            ExecWait(cmd.durationMs, stop, log, idx, count);
            break;
        case CmdType::Teleport:
            ExecTeleport(cmd.tx, cmd.ty, cmd.tz, stop, log, idx, count);
            break;
        case CmdType::Pad:
            ExecPad(cmd.padEntry, cmd.durationMs, stop, log, idx, count);
            break;
        case CmdType::PadMulti:
            ExecPadMulti(cmd.padEntries, cmd.padCount, cmd.durationMs, stop, log, idx, count);
            break;
        case CmdType::Log:
            LogWrite(log, "[%d/%d] USER: %s", idx, count, cmd.text);
            break;
        case CmdType::Nop:
            break;
        }
    }

    DWORD totalElapsed = GetTickCount() - totalStart;
    if (*stop) {
        LogWrite(log, "Script STOPPED by user after %ums", totalElapsed);
    } else {
        LogWrite(log, "Script complete (%d/%d commands, total %ums)", count, count, totalElapsed);
    }

    timeEndPeriod(1);
}
