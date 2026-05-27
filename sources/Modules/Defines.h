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
