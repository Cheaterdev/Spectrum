#pragma once

import Debug;
import Singletons;
import Logs;
import EventsProps;

// #define LEAK_TEST_ENABLE
#ifdef LEAK_TEST_ENABLE
	#define LEAK_TEST(x) LeakDetectorInstance ___leak_tester = LeakDetectorInstance(#x);
#else
	#define LEAK_TEST(x) ;
#endif

#define THREAD_CHECKER std::atomic<std::thread::id> __checker_;

#define ASSERT_SINGLETHREAD Checker __g__(__checker_);
