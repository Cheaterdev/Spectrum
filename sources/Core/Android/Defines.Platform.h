#pragma once
// Android half of Core/Defines.h (the Windows one is ../Windows/Defines.Platform.h; the build's include
// path decides which is found). CORE_ASSERT_FAIL is what ASSERT expands to on failure.
//
// Macros only: Clang rejects declarations in a forced include ahead of `module X;`
// ("module declaration must occur at the start of the translation unit"), so no <cstdio> and no
// assert_fail() here. The expression and location are not logged; the trap's tombstone names the call site.
#define CORE_ASSERT_FAIL(expr, file, line)  __builtin_debugtrap()
