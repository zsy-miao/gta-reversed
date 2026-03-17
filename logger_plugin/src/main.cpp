#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <ctime>

// ============================================================================
// GTA SA 1.0 US Memory Addresses (read-only, no hooks)
// ============================================================================
constexpr uintptr_t ADDR_PLAYER_PED_PTR   = 0xB7CD98; // CWorld::Players[0].m_pPed
constexpr uintptr_t ADDR_GAME_TIME_MS     = 0xB7CB84; // CTimer::m_snTimeInMilliseconds

constexpr uintptr_t OFFSET_MATRIX_PTR     = 0x14;     // CPlaceable::m_matrix (CMatrixLink*)
constexpr uintptr_t OFFSET_SIMPLE_POS     = 0x04;     // CSimpleTransform::m_vPosn (fallback)
constexpr uintptr_t OFFSET_MATRIX_POS     = 0x30;     // CMatrix::m_pos

// Timing
constexpr DWORD POLL_INTERVAL_MS = 100;
constexpr DWORD LOG_INTERVAL_MS  = 2000;

// ============================================================================
// Globals
// ============================================================================
static volatile bool g_running = true;
static HANDLE        g_thread  = nullptr;

// ============================================================================
// SafeRead — SEH-protected memory read
// ============================================================================
template <typename T>
static bool SafeRead(uintptr_t address, T& out) {
    __try {
        out = *reinterpret_cast<const T*>(address);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// ============================================================================
// ReadPlayerPosition — read player world coordinates
// ============================================================================
struct Vec3 { float x, y, z; };

static bool ReadPlayerPosition(Vec3& pos) {
    // Step 1: Read ped pointer from CWorld::Players[0]
    uintptr_t pedPtr = 0;
    if (!SafeRead(ADDR_PLAYER_PED_PTR, pedPtr) || pedPtr == 0)
        return false;

    // Step 2: Read matrix pointer from ped
    uintptr_t matrixPtr = 0;
    if (!SafeRead(pedPtr + OFFSET_MATRIX_PTR, matrixPtr) || matrixPtr == 0) {
        // Fallback: read from CSimpleTransform (ped + 0x04)
        return SafeRead(pedPtr + OFFSET_SIMPLE_POS, pos);
    }

    // Step 3: Read position from matrix
    return SafeRead(matrixPtr + OFFSET_MATRIX_POS, pos);
}

// ============================================================================
// Log file helpers
// ============================================================================
static FILE* OpenLogFile(const char* asiPath) {
    // Determine directory of the ASI file
    char dir[MAX_PATH];
    strncpy(dir, asiPath, MAX_PATH - 1);
    dir[MAX_PATH - 1] = '\0';

    // Strip filename to get directory
    char* lastSlash = strrchr(dir, '\\');
    if (!lastSlash) lastSlash = strrchr(dir, '/');
    if (lastSlash) *(lastSlash + 1) = '\0';

    // Create logs subdirectory
    char logsDir[MAX_PATH];
    snprintf(logsDir, MAX_PATH, "%slogs", dir);
    CreateDirectoryA(logsDir, nullptr);

    // Generate timestamped filename
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

static void LogWrite(FILE* f, const char* fmt, ...) {
    if (!f) return;

    // Timestamp prefix with milliseconds
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(f, "[%02d:%02d:%02d.%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    // Message
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);

    fprintf(f, "\n");
    fflush(f);
}

// ============================================================================
// Logger Thread
// ============================================================================
static DWORD WINAPI LoggerThread(LPVOID param) {
    // Get ASI module path for log file location
    char asiPath[MAX_PATH];
    GetModuleFileNameA(static_cast<HMODULE>(param), asiPath, MAX_PATH);

    // Phase 1: Wait for game to load
    while (g_running) {
        uintptr_t pedPtr = 0;
        uint32_t  gameTime = 0;

        bool hasPed  = SafeRead(ADDR_PLAYER_PED_PTR, pedPtr) && pedPtr != 0;
        bool hasTime = SafeRead(ADDR_GAME_TIME_MS, gameTime) && gameTime != 0;

        if (hasPed && hasTime)
            break;

        Sleep(POLL_INTERVAL_MS);
    }

    if (!g_running)
        return 0;

    // Phase 2: Open log file and record start
    FILE* logFile = OpenLogFile(asiPath);
    if (!logFile)
        return 0;

    LogWrite(logFile, "Game Started");

    // Phase 3: Periodic coordinate logging
    while (g_running) {
        Vec3 pos;
        if (ReadPlayerPosition(pos)) {
            LogWrite(logFile, "Player pos: x=%.2f y=%.2f z=%.2f", pos.x, pos.y, pos.z);
        } else {
            LogWrite(logFile, "Player pos: unavailable");
        }

        // Sleep in small increments so we can exit promptly
        for (DWORD elapsed = 0; elapsed < LOG_INTERVAL_MS && g_running; elapsed += POLL_INTERVAL_MS) {
            Sleep(POLL_INTERVAL_MS);
        }
    }

    LogWrite(logFile, "Logger shutting down");
    fclose(logFile);
    return 0;
}

// ============================================================================
// DllMain
// ============================================================================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        g_running = true;
        g_thread = CreateThread(nullptr, 0, LoggerThread, hModule, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        g_running = false;
        if (g_thread) {
            WaitForSingleObject(g_thread, 5000);
            CloseHandle(g_thread);
            g_thread = nullptr;
        }
        break;
    }
    return TRUE;
}
