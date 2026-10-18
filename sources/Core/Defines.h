#pragma once
// =============================================================================
// Core/Defines.h  —  CORE LAYER
// Macro entry-point for the Core subproject and everything that depends on it.
// Chains upward to Modules/Defines.h, then pulls in each canonical macro
// header by #include so there is exactly one definition site per macro group.
// =============================================================================
#include "Modules/Defines.h"

// ---- Token utilities, compiler helpers, thread-safety annotations ----------
#include "utils/utils_macros.h"

// ============================================================================
// Assert  (no standalone header — originates in Core/Core_defs.h alongside
// non-macro content, so the macro and its helper live here directly)
// ============================================================================

// The per-platform half (CORE_ASSERT_FAIL, and on Windows the inline assert_fail it calls) is
// Core/<Platform>/Defines.Platform.h; the project's include path picks the folder.
#include "Defines.Platform.h"

#ifndef DEV
#  define ASSERT(expr)  ((void)0)
#else
#  define ASSERT(expr)  ((!!(expr)) ? (void)0 : CORE_ASSERT_FAIL(#expr, __FILE__, __LINE__))
#endif

// ============================================================================
// COM / Win32 HRESULT stubs
// Defined without pulling in <windows.h>.  Guards prevent conflicts when the
// Windows SDK is included later.  (originates in Core/Core_defs.h)
// ============================================================================

#ifndef STDMETHODCALLTYPE
#  define STDMETHODCALLTYPE  __stdcall
#endif
#ifndef S_OK
#  define S_OK  0L
#endif
#ifndef E_FAIL
#  define E_FAIL  0x80004005L
#endif
#ifndef E_INVALIDARG
#  define E_INVALIDARG  0x80070057L
#endif
#ifndef E_NOINTERFACE
#  define E_NOINTERFACE  0x80004002L
#endif
#ifndef E_NOTIMPL
#  define E_NOTIMPL  0x80004001L
#endif
#ifndef FAILED
#  define FAILED(hr)     ((long)(hr) < 0)
#endif
#ifndef SUCCEEDED
#  define SUCCEEDED(hr)  ((long)(hr) >= 0)
#endif
#ifndef TRUE
#  define TRUE  1
#endif
#ifndef FALSE
#  define FALSE  0
#endif

// ---- Profiling macros -------------------------------------------------------
#include "Profiling/macros.h"

// ---- Serialization helpers --------------------------------------------------
#include "Serialization/serialization_defines.h"
