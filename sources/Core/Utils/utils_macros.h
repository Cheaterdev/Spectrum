
#define STRINGIZE(x) #x

#define SPECTRUM_MERGE_(a,b)  a##b
#define SPECTRUM_LABEL_(a) SPECTRUM_MERGE_(__timer__, a)
#define SPECTRUM_UNIQUE_NAME SPECTRUM_LABEL_(__LINE__)

#define CACHE_ALIGN(x) __declspec(align(x))

#define GEN_DEF_COMP(x) \
	bool operator==(const x& r) const = default;\
	auto operator<=>(const x& r) const = default;

#define GENERATE_OPS __GENERATE_OPS__



#define THREAD_CHECKER std::atomic<std::thread::id> __checker_;
#define ASSERT_SINGLETHREAD Checker __g__(__checker_);

#ifdef LEAK_TEST_ENABLE
#define LEAK_TEST(x) LeakDetectorInstance ___leak_tester = LeakDetectorInstance(#x);
#else
#define LEAK_TEST(x) ;
#endif



#ifdef PROFILING
#define PROFILE(x) auto SPECTRUM_UNIQUE_NAME = Profiler::get().start(x);
#else
#define PROFILE(x) ;
#endif

#ifdef PROFILING
#define PROFILE_GPU(x) auto SPECTRUM_UNIQUE_NAME = Render::Eventer::thread_current?Render::Eventer::thread_current->start(x):Timer(nullptr, nullptr);
#else
#define PROFILE_GPU(x) ;
#endif

#ifdef DEV
#define THREAD_SCOPE(x) volatile ThreadScope SPECTRUM_UNIQUE_NAME(ThreadType::x);
#define CHECK_THREAD(x) {ThreadScope::check_type(ThreadType::x);};
#else
#define THREAD_SCOPE(x) ;
#define CHECK_THREAD(x) ;
#endif



#define DISABLE_OPTIMIZATION __pragma(optimize( "", off ))