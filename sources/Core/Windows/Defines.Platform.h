#pragma once
// Windows half of Core/Defines.h (the Android one is ../Android/Defines.Platform.h; the project's include
// path decides which is found). CORE_ASSERT_FAIL is what ASSERT expands to on failure.
#include <cstdio>   // assert_fail diagnostics (fopen/fprintf)

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

#define CORE_ASSERT_FAIL(expr, file, line)  ::Core::assert_fail(expr, file, line)
