#pragma once
// =============================================================================
// Modules/Defines.h  —  BASE LAYER
//
// SPECTRUM_DEFINES_INCLUDED is the sentinel that the bridge copies in each
// original macro header use to detect whether the forced include is active.
// Do not remove it.
#define SPECTRUM_DEFINES_INCLUDED
//
// Only globally-required third-party configuration lives here.
// Library-specific feature flags belong in the wrapper header for that library
// (e.g. MAGIC_ENUM_RANGE_* in magic_enum/magic_enum.h).
// No parent — this is the bottom of the chain.
// =============================================================================

// ---- Windows SDK target -------------------------------------------------
// Must be set before <windows.h>, <sdkddkver.h>, or any DirectX header is
// pulled in.  Multiple module wrappers include those headers independently
// (windows/windows.h, d3d12/d3d12_includes.h, etc.) so this must live here
// rather than in any single wrapper.
#ifndef NTDDI_VERSION
#  define NTDDI_VERSION NTDDI_WIN10_RS1
#endif

// ---- Windows SDK GUID operator== suppression ----------------------------
// guiddef.h declares operator==(REFGUID, REFGUID) under this guard.  When
// <windows.h> is imported as a C++20 header unit (e.g. via `import windows;`)
// it already exports that operator with C++ linkage.  If guiddef.h is later
// included again in a TU's GMF (e.g. via <agents.h> -> <concrt.h>), the
// compiler sees a second declaration and raises C2732 (linkage contradiction).
// Defining this token globally ensures the operator is only declared once,
// via the header-unit path, regardless of include order.
#ifndef _SYS_GUID_OPERATOR_EQ_
#  define _SYS_GUID_OPERATOR_EQ_
#endif

// ---- D3D12 / COM helper macros ------------------------------------------
// Brings in IID_PPV_ARGS, DXGI_ERROR_NOT_FOUND, _countof and similar SDK
// macros that named modules cannot export.  Must come after NTDDI_VERSION
// and _SYS_GUID_OPERATOR_EQ_ are set above.
#ifdef _WIN32
#include "d3d12/d3d12_macros.h"
#else
// ---- MSVC keyword/intrinsic compatibility (Clang / Android) ---------------
// Macro-only on purpose: this is forced-included ahead of every `module X;`
// line, and Clang rejects declarations there.
#  define __forceinline  inline __attribute__((always_inline))
#  define __debugbreak() __builtin_debugtrap()
// SEH isn't available off Windows. Core only uses __try/__except to name threads
// (the MSVC debugger-exception trick), which android/shims/windows.ixx decodes directly.
// Win32 integer typedefs that MSVC builds get for free through the Windows headers
// that d3d12_macros.h drags into every TU. Macros, not typedefs: this header must
// not contain declarations (see above).
// Fixed-width names map to <cstdint> types: on LP64 Android uint64_t is `unsigned long`,
// so `unsigned long long` would not match declarations written with uint64.
// Windows' LONG/ULONG are 32-bit, hence int32_t / uint32_t here. Single-token typedef names (not `unsigned int`) so
// functional casts such as UINT(x) stay valid.
#  define UINT64         uint64_t
#  define UINT32         uint32_t
#  define UINT16         uint16_t
#  define UINT8          uint8_t
#  define UINT           uint32_t
#  define INT            int32_t
#  define FLOAT          float
#  define LONG           int32_t
#  define BYTE           uint8_t
#  define ULONG          uint32_t
#  define SIZE_T         size_t
// Windows virtual-key codes: the GUI's key events carry them (see android/shims for the
// touch/keyboard bridge that will produce them).
#  define VK_BACK        0x08
#  define VK_TAB         0x09
#  define VK_RETURN      0x0D
#  define VK_SHIFT       0x10
#  define VK_CONTROL     0x11
#  define VK_MENU        0x12
#  define VK_ESCAPE      0x1B
#  define VK_SPACE       0x20
#  define VK_END         0x23
#  define VK_HOME        0x24
#  define VK_LEFT        0x25
#  define VK_UP          0x26
#  define VK_RIGHT       0x27
#  define VK_DOWN        0x28
#  define VK_DELETE      0x2E
// Debug hotkeys poll the keyboard; there is no hardware keyboard to poll.
#  define GetAsyncKeyState(vk)  ((short)0)
#  define GetKeyState(vk)       ((short)0)
#  define MAX_PATH       4096
// MSVC bit-scan intrinsics (return 1 and set *Index when Mask is non-zero).
#  define _BitScanForward(Index, Mask) __extension__({ unsigned long long __m = (Mask); \
       __m ? (*(Index) = __builtin_ctzll(__m), (unsigned char)1) : (unsigned char)0; })
#  define _BitScanReverse(Index, Mask) __extension__({ unsigned long long __m = (Mask); \
       __m ? (*(Index) = 63 - __builtin_clzll(__m), (unsigned char)1) : (unsigned char)0; })
#  define __try          try
#  define __except(x)    catch (...)
#endif
