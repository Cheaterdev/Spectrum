#pragma once
import Profiling;

#define MERGE_(a,b)  a##b
#define LABEL_(a) MERGE_(__timer__, a)


#define UNIQUE_NAME LABEL_(__LINE__)


#ifdef PROFILING
#define PROFILE(x) auto UNIQUE_NAME = Profiler::get().start(x);
#else
#define PROFILE(x) ;
#endif

#ifdef PROFILING
#define PROFILE_GPU(x) auto UNIQUE_NAME = Eventer::thread_current?Eventer::thread_current->start(x):Timer(nullptr, nullptr);
#else
#define PROFILE_GPU(x) ;
#endif