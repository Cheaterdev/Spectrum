export module windows;
export import "windows.h";

// Foreground text color bits
#undef FOREGROUND_BLUE
export inline constexpr WORD FOREGROUND_BLUE      = 0x0001;
#undef FOREGROUND_GREEN
export inline constexpr WORD FOREGROUND_GREEN     = 0x0002;
#undef FOREGROUND_RED
export inline constexpr WORD FOREGROUND_RED       = 0x0004;
#undef FOREGROUND_INTENSITY
export inline constexpr WORD FOREGROUND_INTENSITY = 0x0008;

// Background color bits
#undef BACKGROUND_BLUE
export inline constexpr WORD BACKGROUND_BLUE      = 0x0010;
#undef BACKGROUND_GREEN
export inline constexpr WORD BACKGROUND_GREEN     = 0x0020;
#undef BACKGROUND_RED
export inline constexpr WORD BACKGROUND_RED       = 0x0040;
#undef BACKGROUND_INTENSITY
export inline constexpr WORD BACKGROUND_INTENSITY = 0x0080;

// Standard device handle IDs
#undef STD_INPUT_HANDLE
export inline constexpr DWORD STD_INPUT_HANDLE  = DWORD(-10);
#undef STD_OUTPUT_HANDLE
export inline constexpr DWORD STD_OUTPUT_HANDLE = DWORD(-11);
#undef STD_ERROR_HANDLE
export inline constexpr DWORD STD_ERROR_HANDLE  = DWORD(-12);

// Invalid handle sentinel — pointer cast, so const not constexpr
#undef INVALID_HANDLE_VALUE
export inline HANDLE const INVALID_HANDLE_VALUE = HANDLE(LONG_PTR(-1));

// Sync wait constant
#undef INFINITE
export inline constexpr DWORD INFINITE = 0xFFFFFFFFul;

// Event access mask  (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x3)
#undef EVENT_ALL_ACCESS
export inline constexpr DWORD EVENT_ALL_ACCESS = 0x001F0003ul;

#undef MB_OK
#undef MB_YESNO
#undef IDOK
#undef IDYES
#undef IDNO

export namespace Windows
{
	// MessageBox type flags
	inline constexpr UINT MB_OK    = 0x00000000u;
	inline constexpr UINT MB_YESNO = 0x00000004u;

	// Dialog return values
	inline constexpr int IDOK  = 1;
	inline constexpr int IDYES = 6;
	inline constexpr int IDNO  = 7;
}

// CreateEventEx is a macro alias for CreateEventExW/A — expose as a real function
#undef CreateEventEx
export inline HANDLE CreateEventEx(LPSECURITY_ATTRIBUTES attrs, LPCWSTR name, DWORD flags, DWORD access)
{
    return ::CreateEventExW(attrs, name, flags, access);
}
