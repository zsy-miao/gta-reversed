#pragma once
#include <cstdint>
#include <cstring>

// ============================================================================
// CControllerState mirror (0x30 bytes) — matches GTA SA 1.0 US layout
// ============================================================================
#pragma pack(push, 1)
struct ControllerState {
    int16_t LeftStickX;       // +0x00
    int16_t LeftStickY;       // +0x02  (-128=forward, +128=backward)
    int16_t RightStickX;      // +0x04
    int16_t RightStickY;      // +0x06
    int16_t LeftShoulder1;    // +0x08  (L1)
    int16_t LeftShoulder2;    // +0x0A  (L2)
    int16_t RightShoulder1;   // +0x0C  (R1 / aim)
    int16_t RightShoulder2;   // +0x0E  (R2)
    int16_t DPadUp;           // +0x10
    int16_t DPadDown;         // +0x12
    int16_t DPadLeft;         // +0x14
    int16_t DPadRight;        // +0x16
    int16_t Start;            // +0x18
    int16_t Select;           // +0x1A
    int16_t ButtonSquare;     // +0x1C  (jump / reverse)
    int16_t ButtonTriangle;   // +0x1E  (enter/exit vehicle)
    int16_t ButtonCross;      // +0x20  (sprint / accelerate)
    int16_t ButtonCircle;     // +0x22  (fire)
    int16_t ShockButtonL;     // +0x24
    int16_t ShockButtonR;     // +0x26
    int16_t ChatIndicated;    // +0x28
    int16_t PedWalk;          // +0x2A
    int16_t VehicleMouseLook; // +0x2C
    int16_t RadioTrackSkip;   // +0x2E
};
static_assert(sizeof(ControllerState) == 0x30);
#pragma pack(pop)

// ============================================================================
// PadFieldOffset — map field name string to byte offset within ControllerState
// Returns -1 if name is not recognized.
// ============================================================================
inline int PadFieldOffset(const char* name) {
    struct Entry { const char* name; int offset; };
    static const Entry table[] = {
        {"left_stick_x",       0x00},
        {"left_stick_y",       0x02},
        {"right_stick_x",      0x04},
        {"right_stick_y",      0x06},
        {"left_shoulder1",     0x08},
        {"left_shoulder2",     0x0A},
        {"right_shoulder1",    0x0C},
        {"right_shoulder2",    0x0E},
        {"dpad_up",            0x10},
        {"dpad_down",          0x12},
        {"dpad_left",          0x14},
        {"dpad_right",         0x16},
        {"start",              0x18},
        {"select",             0x1A},
        {"button_square",      0x1C},
        {"button_triangle",    0x1E},
        {"button_cross",       0x20},
        {"button_circle",      0x22},
        {"shock_button_l",     0x24},
        {"shock_button_r",     0x26},
        {"chat_indicated",     0x28},
        {"ped_walk",           0x2A},
        {"vehicle_mouse_look", 0x2C},
        {"radio_track_skip",   0x2E},
    };

    for (const auto& e : table) {
        if (_stricmp(name, e.name) == 0)
            return e.offset;
    }
    return -1;
}
