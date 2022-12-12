export module HAL:Debug;


export namespace HAL
{

	namespace Debug
	{
#ifdef DEV
		constexpr bool ValidationErrors = false;
		constexpr bool CheckErrors = false;
		constexpr bool DebugViews = false;
#else
		constexpr bool ValidationErrors = false;
		constexpr bool CheckErrors = false;
		constexpr bool DebugViews = false;
#endif
	}
}