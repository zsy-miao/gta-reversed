#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <ctime>

#include "mem.h"
#include "pad.h"
#include "script.h"
#include "skip_intro.h"
#include "menu_monitor.h"
#include "log.h"

// ============================================================================
// Constants
// ============================================================================
constexpr DWORD POLL_INTERVAL_MS = 100;
constexpr int   MAX_COMMANDS     = 256;

// ============================================================================
// Globals
// ============================================================================
static volatile bool g_running     = true;
static volatile bool g_scriptStop  = false;
static volatile bool g_scriptRunning = false;
static HANDLE        g_thread      = nullptr;
static char          g_asiDir[MAX_PATH] = {};

// ============================================================================
// Log file helpers
// ============================================================================
static FILE* OpenLogFile() {
    char logsDir[MAX_PATH];
    snprintf(logsDir, MAX_PATH, "%slogs", g_asiDir);
    CreateDirectoryA(logsDir, nullptr);

    time_t now = time(nullptr);
    struct tm lt;
    localtime_s(&lt, &now);

    char filePath[MAX_PATH];
    snprintf(filePath, MAX_PATH, "%s\\%04d-%02d-%02d_%02d-%02d-%02d.log",
             logsDir,
             lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
             lt.tm_hour, lt.tm_min, lt.tm_sec);

    return fopen(filePath, "w");
}


// ============================================================================
// Detect F8 key press (edge-triggered)
// ============================================================================
static bool DetectF8Press() {
    static bool wasDown = false;
    bool isDown = (GetAsyncKeyState(VK_F8) & 0x8000) != 0;
    bool pressed = isDown && !wasDown;
    wasDown = isDown;
    return pressed;
}

// ============================================================================
// Wait for game to be fully loaded (player ped exists + game time ticking)
// ============================================================================
static bool WaitForGameLoaded(FILE* log) {
    LogWrite(log, "Waiting for game to load...");
    while (g_running) {
        uintptr_t pedPtr = 0;
        uint32_t  gameTime = 0;
        bool hasPed  = SafeRead(ADDR_PLAYER_PED_PTR, pedPtr) && pedPtr != 0;
        bool hasTime = SafeRead(ADDR_GAME_TIME_MS, gameTime) && gameTime != 0;
        if (hasPed && hasTime) {
            LogWrite(log, "Game loaded (ped=0x%08X, time=%u)", pedPtr, gameTime);
            return true;
        }
        MenuMonitor_Update(log);
        Sleep(POLL_INTERVAL_MS);
    }
    return false;
}

// ============================================================================
// Run script from automation.txt
// ============================================================================
static void RunScript(FILE* log) {
    char scriptPath[MAX_PATH];
    snprintf(scriptPath, MAX_PATH, "%sautomation.txt", g_asiDir);

    LogWrite(log, "Loading script: %s", scriptPath);

    static ScriptCommand cmds[MAX_COMMANDS];
    int count = LoadScript(scriptPath, cmds, MAX_COMMANDS, log);
    if (count == 0) {
        LogWrite(log, "No commands loaded — check script file");
        return;
    }

    g_scriptStop = false;
    g_scriptRunning = true;
    LogWrite(log, "Executing %d commands...", count);

    ExecuteScript(cmds, count, &g_scriptStop, log);

    g_scriptRunning = false;
}

// ============================================================================
// Automation Thread — main loop
// ============================================================================
static DWORD WINAPI AutomationThread(LPVOID param) {
    // Determine ASI directory
    char asiPath[MAX_PATH];
    GetModuleFileNameA(static_cast<HMODULE>(param), asiPath, MAX_PATH);
    char* lastSlash = strrchr(asiPath, '\\');
    if (!lastSlash) lastSlash = strrchr(asiPath, '/');
    if (lastSlash) {
        size_t len = lastSlash - asiPath + 1;
        memcpy(g_asiDir, asiPath, len);
        g_asiDir[len] = '\0';
    }

    FILE* log = OpenLogFile();
    if (!log) return 0;

    LogWrite(log, "Automation plugin loaded");
    LogWrite(log, "Press F8 to run automation.txt (or stop a running script)");

    SkipIntro_Run(log, &g_running);
    MenuMonitor_Init();

    if (!WaitForGameLoaded(log)) {
        LogWrite(log, "Plugin shutting down (game not loaded)");
        fclose(log);
        return 0;
    }

    LogWrite(log, "Game started");

    // Main loop: F8 hotkey detection
    while (g_running) {
        if (DetectF8Press()) {
            if (g_scriptRunning) {
                LogWrite(log, "F8 pressed — stopping script");
                g_scriptStop = true;
                while (g_scriptRunning && g_running)
                    Sleep(1);
                LogWrite(log, "Script stopped");
            } else {
                LogWrite(log, "F8 pressed — starting script");
                RunScript(log);
            }
        }
        MenuMonitor_Update(log);
        Sleep(1);
    }

    LogWrite(log, "Plugin shutting down");
    fclose(log);
    return 0;
}

// ============================================================================
// DllMain
// ============================================================================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        SkipIntro_Patch();
        g_running = true;
        g_thread = CreateThread(nullptr, 0, AutomationThread, hModule, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        g_running = false;
        g_scriptStop = true;
        if (g_thread) {
            WaitForSingleObject(g_thread, 5000);
            CloseHandle(g_thread);
            g_thread = nullptr;
        }
        break;
    }
    return TRUE;
}
