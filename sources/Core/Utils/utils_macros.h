
#define STRINGIZE(x) #x

#define SPECTRUM_MERGE_(a,b)  a##b
#define SPECTRUM_LABEL_(a) SPECTRUM_MERGE_(__timer__, a)
#define SPECTRUM_UNIQUE_NAME SPECTRUM_LABEL_(__LINE__)

#define CACHE_ALIGN(x) __declspec(align(x))

#define GEN_DEF_COMP(x) \
	bool operator==(const x& r) const = default;\
	auto operator<=>(const x& r) const = default;


#ifdef DEV
#define THREAD_CHECKER mutable std::atomic<std::thread::id> __checker_;
#define ASSERT_SINGLETHREAD Checker __g__(__checker_);
#else
#define THREAD_CHECKER 
#define ASSERT_SINGLETHREAD 
#endif
#ifdef LEAK_TEST_ENABLE
#define LEAK_TEST(x) LeakDetectorInstance ___leak_tester = LeakDetectorInstance(#x);
#else
#define LEAK_TEST(x) ;
#endif

#ifdef DEV
#define THREAD_SCOPE(x) volatile ThreadScope SPECTRUM_UNIQUE_NAME(ThreadType::x);
#define CHECK_THREAD(x) {ThreadScope::check_type(ThreadType::x);};
#else
#define THREAD_SCOPE(x) ;
#define CHECK_THREAD(x) ;
#endif

#define BUG_ALERT Log::get()<<"ALERT HERE"<<Log::endl

#define DISABLE_OPTIMIZATION __pragma(optimize( "", off ))