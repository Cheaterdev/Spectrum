#pragma once
// Profiling macros for Core and all dependent projects.
// Included by Core/Defines.h via forced include.
// NOTE: Both PROFILING and non-PROFILING branches expand identically today —
// the Profiler/Eventer calls become no-ops in Retail builds because those
// objects are compiled out.  The #ifdef is kept for potential future divergence.

#ifndef PROFILE
#  define PROFILE(x) \
       auto SPECTRUM_UNIQUE_NAME = Profiler::get().start(x);
#  define PROFILE_GPU(x) \
       auto SPECTRUM_UNIQUE_NAME = HAL::Eventer::thread_current \
           ? HAL::Eventer::thread_current->start(x) \
           : Timer(nullptr, nullptr);
#endif
