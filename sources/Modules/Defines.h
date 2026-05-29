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
#include "d3d12/d3d12_macros.h"
