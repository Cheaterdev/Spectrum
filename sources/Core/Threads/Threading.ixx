export module Core:Threading;

import <stl/threading.h>;
import <stl/core.h>;
import <stl/memory.h>;

import <stl/core.h>;
import <windows/windows.h>;

import windows;

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
export
{


void SetThreadName(uint32_t dwThreadID, const char* threadName);
void SetThreadName(std::string threadName);
void SetThreadName(std::thread* thread, const char* threadName);


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


class SpinLock
{
public:
	void lock()
	{
		while (lck.test_and_set(std::memory_order_acquire))
		{
		}
	}

	void unlock()
	{
		lck.clear(std::memory_order_release);
	}
	SpinLock() = default;
	SpinLock(const SpinLock&) {}
	void operator=(const SpinLock&) {}
private:
	std::atomic_flag lck = ATOMIC_FLAG_INIT;
};


}





void SetThreadName(uint32_t dwThreadID, const char* threadName)
{
	// DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}

	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}
void SetThreadName(std::string threadName)
{
	SetThreadName(GetCurrentThreadId(), threadName.c_str());
}

void SetThreadName(std::thread* thread, const char* threadName)
{
	DWORD threadId = ::GetThreadId(static_cast<HANDLE>(thread->native_handle()));
	SetThreadName(threadId, threadName);
}

thread_local ThreadType ThreadScope::thread_type = ThreadType::NONE;