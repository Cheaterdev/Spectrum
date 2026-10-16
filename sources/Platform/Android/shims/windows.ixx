// Android stand-in for Modules/windows/windows.ixx: only the Win32 surface the
// ported layers actually use, backed by bionic / logcat. Grow it per layer.
module;

#include <android/log.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <sys/prctl.h>
#include <unistd.h>

export module windows;

export {

using BOOL = int;
using WORD = uint16_t;
using DWORD = uint32_t;
using HRESULT = long;
using ULONG_PTR = uintptr_t;
using LPCSTR = const char*;
using LPSTR = char*;
using LPCWSTR = const wchar_t*;

// A window handle is an ANativeWindow* on Android; opaque to everything but the swapchain.
using HWND = void*;
using HINSTANCE = void*;

// A struct rather than void*: Core does static_cast<HANDLE>(thread->native_handle()),
// and pthread_t is an integer on bionic, which cannot be cast to a pointer type.
struct HANDLE
{
    uintptr_t value = 0;
    constexpr HANDLE() = default;
    constexpr explicit HANDLE(uintptr_t v) : value(v) {}
    constexpr explicit operator bool() const { return value != 0; }
    friend constexpr bool operator==(HANDLE, HANDLE) = default;
};

inline constexpr HANDLE INVALID_HANDLE_VALUE{~uintptr_t(0)};

inline constexpr WORD FOREGROUND_BLUE      = 0x0001;
inline constexpr WORD FOREGROUND_GREEN     = 0x0002;
inline constexpr WORD FOREGROUND_RED       = 0x0004;
inline constexpr WORD FOREGROUND_INTENSITY = 0x0008;
inline constexpr WORD BACKGROUND_BLUE      = 0x0010;
inline constexpr WORD BACKGROUND_GREEN     = 0x0020;
inline constexpr WORD BACKGROUND_RED       = 0x0040;
inline constexpr WORD BACKGROUND_INTENSITY = 0x0080;

inline constexpr DWORD STD_INPUT_HANDLE  = DWORD(-10);
inline constexpr DWORD STD_OUTPUT_HANDLE = DWORD(-11);
inline constexpr DWORD STD_ERROR_HANDLE  = DWORD(-12);

// MSVC CRT; lives here because the files using it all `import windows`.
inline int strerror_s(char* buffer, size_t size, int errnum)
{
    if (!buffer || size == 0)
        return 22; // EINVAL
    strncpy(buffer, strerror(errnum), size - 1);
    buffer[size - 1] = '\0';
    return 0;
}

inline DWORD GetCurrentThreadId() { return static_cast<DWORD>(gettid()); }

inline DWORD GetThreadId(HANDLE thread)
{
    return static_cast<DWORD>(pthread_gettid_np(static_cast<pthread_t>(thread.value)));
}

// The console is logcat here; there is no console window to allocate or recolor.
inline HANDLE GetStdHandle(DWORD) { return HANDLE{1}; }
inline BOOL AllocConsole() { return 1; }
inline BOOL FreeConsole() { return 1; }
inline BOOL SetConsoleTitleW(LPCWSTR) { return 1; }
inline BOOL SetConsoleTextAttribute(HANDLE, WORD) { return 1; }

inline BOOL WriteConsoleA(HANDLE, const void* buffer, DWORD chars, DWORD*, void*)
{
    __android_log_print(ANDROID_LOG_INFO, "Spectrum", "%.*s", static_cast<int>(chars), static_cast<const char*>(buffer));
    return 1;
}

inline void OutputDebugStringA(LPCSTR text) { __android_log_write(ANDROID_LOG_DEBUG, "Spectrum", text); }

namespace Windows
{
    inline constexpr unsigned int MB_OK    = 0x00000000u;
    inline constexpr unsigned int MB_YESNO = 0x00000004u;
    inline constexpr int IDOK  = 1;
    inline constexpr int IDYES = 6;
    inline constexpr int IDNO  = 7;
}

// No dialogs here: log the message and take the affirmative answer.
inline int MessageBoxA(HWND, LPCSTR text, LPCSTR caption, unsigned int type)
{
    __android_log_print(ANDROID_LOG_WARN, "Spectrum", "[%s] %s", caption ? caption : "", text ? text : "");
    return (type & Windows::MB_YESNO) ? Windows::IDYES : Windows::IDOK;
}

// Mouse cursors: the system draws the pointer on Android (touch); these only keep the GUI's
// cursor table compiling. Distinct strings stand in for the MAKEINTRESOURCE ids.
using LPCTSTR = const char*;
using HCURSOR = void*;
inline constexpr LPCTSTR IDC_ARROW = "IDC_ARROW", IDC_IBEAM = "IDC_IBEAM", IDC_SIZENS = "IDC_SIZENS",
                         IDC_SIZEWE = "IDC_SIZEWE", IDC_SIZENWSE = "IDC_SIZENWSE", IDC_SIZENESW = "IDC_SIZENESW",
                         IDC_SIZEALL = "IDC_SIZEALL", IDC_NO = "IDC_NO", IDC_WAIT = "IDC_WAIT", IDC_HAND = "IDC_HAND";
inline HCURSOR LoadCursor(HINSTANCE, LPCTSTR) { return nullptr; }
inline HCURSOR SetCursor(HCURSOR) { return nullptr; }

// TextEngine looks for system fonts under <windows dir>\Fonts; Android keeps them in /system/fonts.
inline unsigned int GetWindowsDirectoryA(char* buffer, unsigned int size)
{
    const char* dir = "/system";
    unsigned int n = static_cast<unsigned int>(strlen(dir));
    if (buffer && size > n)
        memcpy(buffer, dir, n + 1);
    return n;
}

inline constexpr unsigned int CP_UTF8 = 65001;

// UTF-16 (Windows) <-> UTF-8: wchar_t is 32-bit on Android, so this is UTF-32 <-> UTF-8.
inline int WideCharToMultiByte(unsigned int, DWORD, const wchar_t* src, int src_len, char* dst, int dst_len,
                               const char*, BOOL*)
{
    if (src_len < 0)
        src_len = static_cast<int>(wcslen(src)) + 1;
    int written = 0;
    auto put = [&](unsigned char c) { if (dst && written < dst_len) dst[written] = static_cast<char>(c); ++written; };
    for (int i = 0; i < src_len; ++i)
    {
        unsigned int c = static_cast<unsigned int>(src[i]);
        if (c < 0x80) put(static_cast<unsigned char>(c));
        else if (c < 0x800) { put(0xC0 | (c >> 6)); put(0x80 | (c & 0x3F)); }
        else if (c < 0x10000) { put(0xE0 | (c >> 12)); put(0x80 | ((c >> 6) & 0x3F)); put(0x80 | (c & 0x3F)); }
        else { put(0xF0 | (c >> 18)); put(0x80 | ((c >> 12) & 0x3F)); put(0x80 | ((c >> 6) & 0x3F)); put(0x80 | (c & 0x3F)); }
    }
    return written;
}

inline int MultiByteToWideChar(unsigned int, DWORD, const char* src, int src_len, wchar_t* dst, int dst_len)
{
    if (src_len < 0)
        src_len = static_cast<int>(strlen(src)) + 1;
    int written = 0;
    auto put = [&](unsigned int c) { if (dst && written < dst_len) dst[written] = static_cast<wchar_t>(c); ++written; };
    for (int i = 0; i < src_len;)
    {
        unsigned int c = static_cast<unsigned char>(src[i]);
        int extra = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;
        if (extra) c &= (0x3F >> extra);
        ++i;
        for (; extra > 0 && i < src_len; --extra, ++i)
            c = (c << 6) | (static_cast<unsigned char>(src[i]) & 0x3F);
        put(c);
    }
    return written;
}

// Core names threads with the MSVC trick: RaiseException(0x406D1388) inside __try/__except
// (mapped to try/catch in Modules/Defines.h). Decode the THREADNAME_INFO payload and
// apply it; bionic can only rename the calling thread (prctl).
inline void RaiseException(DWORD code, DWORD, DWORD arg_count, const ULONG_PTR* args)
{
    if (code != 0x406D1388 || arg_count < 3)
        return;
    const char* name = reinterpret_cast<const char*>(args[1]);
    DWORD thread_id = static_cast<DWORD>(args[2] & 0xFFFFFFFFu);
    if (thread_id == DWORD(-1) || thread_id == GetCurrentThreadId())
        prctl(PR_SET_NAME, name);
}

}
