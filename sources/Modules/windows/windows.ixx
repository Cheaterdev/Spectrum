module;
export import "windows.h";
export module windows;

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
