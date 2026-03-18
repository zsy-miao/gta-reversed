#pragma once
#include <cstdint>
#include <cstdio>

// ============================================================================
// GTA SA 1.0 US — Intro-related addresses
// ============================================================================
constexpr uintptr_t ADDR_GAME_STATE            = 0xC8D4C0; // gGameState (int32)
constexpr uintptr_t ADDR_VIDEO_PLAYER_PLAY     = 0x747660; // VideoPlayer::Play
constexpr uintptr_t ADDR_FADE_OUT              = 0x590860; // CLoadingScreen::DoPCTitleFadeOut
constexpr uintptr_t ADDR_FADE_IN               = 0x590990; // CLoadingScreen::DoPCTitleFadeIn

// Game states
constexpr int32_t GS_INITIAL           = 0;
constexpr int32_t GS_LOGO              = 1;
constexpr int32_t GS_PLAYING_LOGO      = 2;
constexpr int32_t GS_TITLE             = 3;
constexpr int32_t GS_PLAYING_INTRO     = 4;
constexpr int32_t GS_FRONTEND_LOADING  = 5;

// Call from DllMain DLL_PROCESS_ATTACH (before CreateThread)
void SkipIntro_Patch();

// Call from automation thread (before WaitForGameLoaded).
// Polls gGameState, advances stuck states, restores patches when done.
void SkipIntro_Run(FILE* log, volatile bool* running);
