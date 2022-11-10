#ifdef PROFILING
#define PROFILE(x) auto SPECTRUM_UNIQUE_NAME = Profiler::get().start(x);
#else
#define PROFILE(x) ;
#endif

#ifdef PROFILING
#define PROFILE_GPU(x) auto SPECTRUM_UNIQUE_NAME = HAL::Eventer::thread_current?HAL::Eventer::thread_current->start(x):Timer(nullptr, nullptr);
#else
#define PROFILE_GPU(x) ;
#endif