export module HAL:Debug;


export namespace HAL
{

	namespace Debug
	{
#ifdef DEV
		static constexpr bool ValidationErrors = true;
		static constexpr bool CheckErrors = true;
		static constexpr bool DebugViews = false;
#else
		static constexpr bool ValidationErrors = false;
		static constexpr bool CheckErrors = false;
		static constexpr bool DebugViews = false;
#endif
	}
}