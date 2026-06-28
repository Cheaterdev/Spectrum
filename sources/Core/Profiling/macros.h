#pragma once
// Profiling macros for Core and all dependent projects.
// Included by Core/Defines.h via forced include.
// Define PROFILING (e.g. via Sharpmake per-config) to enable instrumentation.
// Without it, PROFILE / PROFILE_GPU expand to nothing — true zero overhead.

#ifndef PROFILE
#  ifdef PROFILING
#    define PROFILE(x) \
         auto SPECTRUM_UNIQUE_NAME = Profiler::get().start(x);
#    define PROFILE_GPU(x) \
         auto SPECTRUM_UNIQUE_NAME = HAL::Eventer::thread_current \
             ? HAL::Eventer::thread_current->start(x) \
             : Timer();
#  else
#    define PROFILE(x)     ((void)0)
#    define PROFILE_GPU(x) ((void)0)
#  endif
#endif
