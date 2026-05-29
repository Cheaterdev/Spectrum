#pragma once
// d3d12_macros.h
//
// Helper macros from the D3D12/Windows SDK that C++ named modules cannot
// export.  Included from Modules/Defines.h (project-wide forced include) so
// every TU — including .ixx module interfaces with no global-module-fragment —
// has them available.
//
// IMPORTANT: no Windows SDK headers are pulled in here.  combaseapi.h /
// windef.h define conflicting macros (ERROR, byte, BOOL, ...) that break
// the rest of the codebase when force-included globally.  Only self-contained
// definitions using compiler intrinsics or pure C++ are allowed.

// ---- SAL annotations ----------------------------------------------------
// <sal.h> is pure macros with zero Windows-type dependencies.
// Provides _In_, _Out_, _COM_Outptr_result_maybenull_, etc.
#include <sal.h>

// ---- COM reference-type macros ------------------------------------------
// In C++ mode guiddef.h defines these as macros, not typedefs, so they are
// not exported by named modules.  IID/GUID themselves are proper typedefs and
// are exported; only the reference aliases need to live here.
#ifndef REFIID
#  define REFIID    const IID &
#endif
#ifndef REFGUID
#  define REFGUID   const GUID &
#endif
#ifndef REFCLSID
#  define REFCLSID  const CLSID &
#endif

// ---- COM HRESULT codes --------------------------------------------------
// Defined as macros via _HRESULT_TYPEDEF_ in winerror.h.
// HRESULT is a typedef exported by the d3d12 module, so it is available
// when these macros are expanded in code.
#ifndef S_OK
#  define S_OK            ((HRESULT)0L)
#endif
#ifndef E_FAIL
#  define E_FAIL          ((HRESULT)0x80004005L)
#endif
#ifndef E_POINTER
#  define E_POINTER       ((HRESULT)0x80004003L)
#endif
#ifndef E_NOINTERFACE
#  define E_NOINTERFACE   ((HRESULT)0x80004002L)
#endif

// ---- Code-page identifiers ----------------------------------------------
#ifndef CP_ACP
#  define CP_ACP    0
#endif
#ifndef CP_UTF8
#  define CP_UTF8   65001
#endif

// ---- Win32 message-box flags --------------------------------------------
#ifndef MB_OK
#  define MB_OK     0x00000000L
#endif

// ---- IID_PPV_ARGS -------------------------------------------------------
// Reimplemented without combaseapi.h.  Uses a private helper to avoid ODR
// conflicts with the SDK version if combaseapi.h is ever included separately.
#ifndef IID_PPV_ARGS
#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)
#endif

// ---- _countof -----------------------------------------------------------
// Defined in <vcruntime.h> but not visible through named-module imports.
#ifndef _countof
#  define _countof(arr)  (sizeof(arr) / sizeof((arr)[0]))
#endif

// ---- IsEqualIID ---------------------------------------------------------
// Defined in objbase.h (not in the d3d12 module export chain).
// IsEqualGUID is exported from d3d12 via guiddef.h and is equivalent.
#ifndef IsEqualIID
#  define IsEqualIID(a, b)  IsEqualGUID(a, b)
#endif
