#pragma once
// Macros formerly defined here have been migrated to Core/Defines.h.
// The block below is active only when the Sharpmake-generated project has NOT
// yet been regenerated (i.e. the forced include of Defines.h is not in effect).
// After running generate_project.bat, SPECTRUM_DEFINES_INCLUDED is set by the
// forced include and this entire block is skipped automatically.
#ifndef SPECTRUM_DEFINES_INCLUDED

#include "serialization/serialization_defines.h"
#include "utils/utils_macros.h"
#include "profiling/macros.h"

#ifndef ASSERT
#  ifndef DEV
#    define ASSERT(expr) ((void)0)
#  else
#    define ASSERT(expr) \
         ((!!(expr)) ? (void)0 : ::Core::assert_fail(#expr, __FILE__, __LINE__))
#  endif
#endif

#ifndef STDMETHODCALLTYPE
#  define STDMETHODCALLTYPE __stdcall
#endif
#ifndef S_OK
#  define S_OK 0L
#endif
#ifndef E_FAIL
#  define E_FAIL 0x80004005L
#endif
#ifndef E_INVALIDARG
#  define E_INVALIDARG 0x80070057L
#endif
#ifndef E_NOINTERFACE
#  define E_NOINTERFACE 0x80004002L
#endif
#ifndef E_NOTIMPL
#  define E_NOTIMPL 0x80004001L
#endif
#ifndef FAILED
#  define FAILED(hr) ((long)(hr) < 0)
#endif
#ifndef SUCCEEDED
#  define SUCCEEDED(hr) ((long)(hr) >= 0)
#endif
#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

#endif // !SPECTRUM_DEFINES_INCLUDED

import stl.core;
import stl.memory;

#ifndef CORE_ASSERT_FAIL_DEFINED
#define CORE_ASSERT_FAIL_DEFINED
namespace Core {
    // Called by the ASSERT macro (defined in Core/Defines.h) on failure.
    [[noreturn]] inline void assert_fail(const char* expr, const char* file, int line) noexcept
    {
        (void)expr; (void)file; (void)line;
        __debugbreak();
    }
}
#endif
