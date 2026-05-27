#pragma once
// Token utilities, compiler helpers and thread-safety annotations for Core
// and all dependent projects.  Included by Core/Defines.h via forced include.

#ifndef STRINGIZE
#  define STRINGIZE(x) #x
#endif

#ifndef SPECTRUM_MERGE_
#  define SPECTRUM_MERGE_(a, b)  a##b
#  define SPECTRUM_LABEL_(a)     SPECTRUM_MERGE_(__timer__, a)
// Generates a name that is unique within the translation unit (per line).
#  define SPECTRUM_UNIQUE_NAME   SPECTRUM_LABEL_(__LINE__)
#endif

#ifndef CACHE_ALIGN
// MSVC-specific: align a type/variable to x bytes.
#  define CACHE_ALIGN(x)  __declspec(align(x))
#endif

#ifndef UNUSED
// Suppress "unreferenced variable" warnings without a runtime cost.
#  define UNUSED(x)  (void)(sizeof((x), 0))
#endif

#ifndef DISABLE_OPTIMIZATION
// Emit a compiler pragma to disable optimisation from this point forward.
#  define DISABLE_OPTIMIZATION  __pragma(optimize("", off))
#endif

#ifndef unorm
// HLSL semantic keyword — stripped in C++ translation units.
#  define unorm
#endif

#ifndef GEN_DEF_COMP
// Generate a defaulted equality + three-way comparison operator pair for type x.
#  define GEN_DEF_COMP(x)             \
       bool operator==(const x& r) const = default; \
       auto operator<=>(const x& r) const = default;
#endif

// ---- Thread-safety annotations (active only in DEV builds) ----------------
#ifndef THREAD_CHECKER
#  ifdef DEV
#    define THREAD_CHECKER        mutable std::atomic<std::thread::id> __checker_;
#    define ASSERT_SINGLETHREAD   Checker __g__(__checker_);
#    define THREAD_SCOPE(x)       volatile ThreadScope SPECTRUM_UNIQUE_NAME(ThreadType::x)
#    define CHECK_THREAD(x)       { ThreadScope::check_type(ThreadType::x); }
#  else
#    define THREAD_CHECKER
#    define ASSERT_SINGLETHREAD
#    define THREAD_SCOPE(x)  (void)0
#    define CHECK_THREAD(x)  (void)0
#  endif
#endif

#ifndef BUG_ALERT
// Quick debug alert written to the Core log stream.
#  define BUG_ALERT  Log::get() << "ALERT HERE" << Log::endl
#endif
