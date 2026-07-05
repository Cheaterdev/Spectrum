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
#include <cstdio>   // assert_fail diagnostics (fopen/fprintf)

// ============================================================================
// Assert  (no standalone header — originates in Core/Core_defs.h alongside
// non-macro content, so the macro and its helper live here directly)
// ============================================================================

#ifndef DEV
#  define ASSERT(expr)  ((void)0)
#else
#  define ASSERT(expr) \
       ((!!(expr)) ? (void)0 : ::Core::assert_fail(#expr, __FILE__, __LINE__))
#endif

// Core::assert_fail — called by ASSERT on failure (DEV builds only).
// The #ifndef guard prevents a redefinition when Core_defs.h is also
// compiled into the same TU (the antlr generated header still imports it).
#ifndef CORE_ASSERT_FAIL_DEFINED
#define CORE_ASSERT_FAIL_DEFINED
namespace Core {
    [[noreturn]] inline void assert_fail(const char* expr, const char* file, int line) noexcept {
        // Record where the assert fired before breaking — WER only reports the
        // __debugbreak address inside this function, not the failing call site.
        if (FILE* f = std::fopen("assert.txt", "a")) {
            std::fprintf(f, "ASSERT FAILED: (%s)  at %s:%d\n", expr, file, line);
            std::fclose(f);
        }
        std::fprintf(stderr, "ASSERT FAILED: (%s)  at %s:%d\n", expr, file, line);
        std::fflush(stderr);
        __debugbreak();
    }
}
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
