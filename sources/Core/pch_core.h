#pragma once


//#include <fstream>


//#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
/*
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/serialization/array.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/export.hpp>

*/





#define THREAD_CHECKER std::atomic<std::thread::id> __checker_;
#define ASSERT_SINGLETHREAD Checker __g__(__checker_);

#ifdef LEAK_TEST_ENABLE
#define LEAK_TEST(x) LeakDetectorInstance ___leak_tester = LeakDetectorInstance(#x);
#else
#define LEAK_TEST(x) ;
#endif
