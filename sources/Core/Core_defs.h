#pragma once

#include "serialization/serialization_defines.h"
#include "utils/utils_macros.h"
#include "profiling/macros.h"
#include <Utils/utils_macros.h>

import stl.core;
import stl.memory;

namespace Core {
    [[noreturn]] inline void assert_fail(const char* expr, const char* file, int line) noexcept
    {
       //OutputDebugString("Assertion failed: (%s)\n  %s:%d\n", expr, file, line);
    //    __debugbreak();
   //     for (;;) {}
    }
}

#ifndef DEV
#  define ASSERT(expr) ((void)0)
#else
#  define ASSERT(expr) \
       ((!!(expr)) ? (void)0 : ::Core::assert_fail(#expr, __FILE__, __LINE__))
#endif

#ifndef STDMETHODCALLTYPE
#  define STDMETHODCALLTYPE __stdcall
#endif

// COM HRESULT constants — defined without requiring windows.h macros
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
