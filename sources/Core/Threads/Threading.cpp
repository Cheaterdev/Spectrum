#include "pch.h"
#include "threading.h"


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
