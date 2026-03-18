#include "menu_monitor.h"
#include "mem.h"
#include "log.h"
#include "skip_intro.h" // ADDR_GAME_STATE

// FrontEndMenuManager (0xBA6748) field addresses
constexpr uintptr_t ADDR_MENU_CURRENT_SCREEN = 0xBA68A5; // +0x15D, eMenuScreen (int8)
constexpr uintptr_t ADDR_MENU_SELECTED_SLOT  = 0xBA68A7; // +0x15F, uint8 (0-7)

// eMenuScreen values
constexpr int8_t SCREEN_NEW_GAME_ASK  = 6;
constexpr int8_t SCREEN_SELECT_GAME   = 7;
constexpr int8_t SCREEN_LOAD_GAME     = 9;
constexpr int8_t SCREEN_LOAD_GAME_ASK = 11;

// eGameState values
constexpr int32_t GS_FRONTEND_IDLE = 7;

static int32_t s_prevGameState = -1;
static int8_t  s_prevScreen    = -1;

void MenuMonitor_Init() {
    s_prevGameState = -1;
    s_prevScreen = -1;
}

void MenuMonitor_Update(FILE* log) {
    // 1. Detect main menu reached (gGameState transitions to 7)
    int32_t gameState = 0;
    if (SafeRead(ADDR_GAME_STATE, gameState)) {
        if (gameState == GS_FRONTEND_IDLE && s_prevGameState != GS_FRONTEND_IDLE) {
            LogWrite(log, "[MenuMonitor] Main menu reached");
        }
        s_prevGameState = gameState;
    }

    // 2 & 3. Detect menu screen changes
    int8_t screen = -1;
    if (SafeRead(ADDR_MENU_CURRENT_SCREEN, screen)) {
        if (screen != s_prevScreen) {
            if (screen == SCREEN_NEW_GAME_ASK || screen == SCREEN_SELECT_GAME) {
                LogWrite(log, "[MenuMonitor] User selected: New Game");
            }
            else if (screen == SCREEN_LOAD_GAME) {
                LogWrite(log, "[MenuMonitor] User selected: Load Game");
            }
            else if (screen == SCREEN_LOAD_GAME_ASK && s_prevScreen == SCREEN_LOAD_GAME) {
                uint8_t slot = 0;
                if (SafeRead(ADDR_MENU_SELECTED_SLOT, slot)) {
                    LogWrite(log, "[MenuMonitor] Save slot selected: %d", (int)(slot + 1));
                }
            }
            s_prevScreen = screen;
        }
    }
}
