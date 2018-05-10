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