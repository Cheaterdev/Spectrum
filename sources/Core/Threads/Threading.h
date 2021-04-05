#pragma once

#ifdef _WIN32
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)


void SetThreadName(uint32_t dwThreadID, const char* threadName);
void SetThreadName(std::string threadName);
void SetThreadName(std::thread* thread, const char* threadName);
#else
void SetThreadName(std::thread* thread, const char* threadName)
{
    auto handle = thread->native_handle();
    pthread_setname_np(handle, threadName);
}


#include <sys/prctl.h>
void SetThreadName(std::string threadName)
{
    prctl(PR_SET_NAME, threadName.c_str(), 0, 0, 0);
}

#endif



struct thread_tester
{
	std::atomic<std::thread::id>& id;
	thread_tester(std::atomic<std::thread::id>& id) :id(id)
	{
		auto prev = id.exchange(std::this_thread::get_id());

		assert(prev == std::thread::id());
	}

	~thread_tester()
	{
		auto prev = id.exchange(std::thread::id());
		assert(prev == std::this_thread::get_id());
	}
};

namespace Thread
{


	struct Lockable
	{
		using guard = std::lock_guard<std::mutex>;
		using mutex = std::mutex;
	};

	struct  Free
	{
		using guard = thread_tester;
		using mutex = std::atomic<std::thread::id>;
	};
}


enum class ThreadType: int
{
	NONE,
	GRAPHICS,
	GUI
};


struct ThreadScope
{
	thread_local static ThreadType thread_type;

	ThreadType prev_type;
	ThreadScope(ThreadType type)
	{
		prev_type = thread_type;
		thread_type = type;
		
	}

virtual ~ThreadScope()
	{
	thread_type = prev_type;
	}

	static void check_type(ThreadType type)
	{
		assert(thread_type == type);
	}
};

#ifdef DEV
	#define THREAD_SCOPE(x) volatile ThreadScope UNIQUE_NAME(ThreadType::x);
	#define CHECK_THREAD(x) {ThreadScope::check_type(ThreadType::x);};
#else
	#define THREAD_SCOPE(x) ;
	#define CHECK_THREAD(x) ;
#endif

