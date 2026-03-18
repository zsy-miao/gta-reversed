#include "skip_intro.h"
#include "mem.h"
#include <windows.h>

// ============================================================================
// Saved original bytes for the three patched functions
// ============================================================================
static uint8_t s_origVideoPlay = 0;
static uint8_t s_origFadeOut   = 0;
static uint8_t s_origFadeIn    = 0;
static bool    s_patched       = false;

// ============================================================================
// SkipIntro_Patch — NOP three functions by writing RET at entry
// ============================================================================
void SkipIntro_Patch() {
    bool ok = true;
    ok &= PatchByte(ADDR_VIDEO_PLAYER_PLAY, 0xC3, &s_origVideoPlay);
    ok &= PatchByte(ADDR_FADE_OUT,          0xC3, &s_origFadeOut);
    ok &= PatchByte(ADDR_FADE_IN,           0xC3, &s_origFadeIn);
    s_patched = ok;
}

// ============================================================================
// Restore all three functions to their original bytes
// ============================================================================
static void RestorePatches() {
    if (!s_patched) return;
    UnpatchByte(ADDR_VIDEO_PLAYER_PLAY, s_origVideoPlay);
    UnpatchByte(ADDR_FADE_OUT,          s_origFadeOut);
    UnpatchByte(ADDR_FADE_IN,           s_origFadeIn);
    s_patched = false;
}

// ============================================================================
// SkipIntro_Run — poll gGameState and advance past intro screens
// ============================================================================
void SkipIntro_Run(FILE* log, volatile bool* running) {
    if (!s_patched) return;

    if (log) fprintf(log, "[SkipIntro] Waiting to advance past intro states...\n");

    while (*running) {
        int32_t state = -1;
        if (!SafeRead(ADDR_GAME_STATE, state)) {
            Sleep(10);
            continue;
        }

        if (state >= GS_FRONTEND_LOADING) {
            RestorePatches();
            if (log) {
                fprintf(log, "[SkipIntro] gGameState=%d >= %d, patches restored\n",
                        state, GS_FRONTEND_LOADING);
                fflush(log);
            }
            return;
        }

        if (state == GS_PLAYING_LOGO) {
            SafeWrite<int32_t>(ADDR_GAME_STATE, GS_TITLE);
            if (log) { fprintf(log, "[SkipIntro] %d -> %d (skip logo video)\n", GS_PLAYING_LOGO, GS_TITLE); fflush(log); }
        } else if (state == GS_PLAYING_INTRO) {
            SafeWrite<int32_t>(ADDR_GAME_STATE, GS_FRONTEND_LOADING);
            if (log) { fprintf(log, "[SkipIntro] %d -> %d (skip intro video)\n", GS_PLAYING_INTRO, GS_FRONTEND_LOADING); fflush(log); }
        }

        Sleep(10);
    }

    // Plugin shutting down before intro finished — restore anyway
    RestorePatches();
}
