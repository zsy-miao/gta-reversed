#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>

// ============================================================================
// GTA SA 1.0 US Memory Addresses
// ============================================================================
constexpr uintptr_t ADDR_PLAYER_PED_PTR   = 0xB7CD98; // CWorld::Players[0].m_pPed
constexpr uintptr_t ADDR_GAME_TIME_MS     = 0xB7CB84; // CTimer::m_snTimeInMilliseconds
constexpr uintptr_t ADDR_PAD0_NEW_STATE   = 0xB73458; // CPad::Pads[0].NewState (CControllerState, 0x30 bytes)
constexpr uintptr_t ADDR_UPDATE_PADS      = 0x541DD0; // CPad::UpdatePads() — called each frame

// CPlaceable / CMatrix offsets
constexpr uintptr_t OFF_MATRIX_PTR  = 0x14; // CPlaceable::m_matrix (CMatrixLink*)
constexpr uintptr_t OFF_MATRIX_POS  = 0x30; // CMatrix::m_pos (Vec3)
constexpr uintptr_t OFF_SIMPLE_POS  = 0x04; // CSimpleTransform::m_vPosn (Vec3)

// ============================================================================
// Vec3
// ============================================================================
struct Vec3 { float x, y, z; };

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
// SafeWrite — SEH-protected memory write
// ============================================================================
template <typename T>
static bool SafeWrite(uintptr_t address, const T& value) {
    __try {
        *reinterpret_cast<T*>(address) = value;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// ============================================================================
// PatchByte / UnpatchByte — write a single byte to code section (.text)
// Handles VirtualProtect for execute-only pages.
// ============================================================================
static bool PatchByte(uintptr_t address, uint8_t newByte, uint8_t* outOldByte) {
    DWORD oldProtect;
    if (!VirtualProtect(reinterpret_cast<void*>(address), 1, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;
    if (outOldByte)
        *outOldByte = *reinterpret_cast<uint8_t*>(address);
    *reinterpret_cast<uint8_t*>(address) = newByte;
    VirtualProtect(reinterpret_cast<void*>(address), 1, oldProtect, &oldProtect);
    return true;
}

static bool UnpatchByte(uintptr_t address, uint8_t origByte) {
    return PatchByte(address, origByte, nullptr);
}
