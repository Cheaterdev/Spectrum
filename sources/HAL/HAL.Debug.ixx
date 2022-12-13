export module HAL:Debug;


export namespace HAL
{

	namespace Debug
	{
		constexpr bool RunForPix = false;

#ifdef DEV
		constexpr bool ValidationErrors = !RunForPix&&true;
		constexpr bool CheckErrors =  !RunForPix&&true;
		constexpr bool DebugViews =  !RunForPix&&false;
#else
		constexpr bool ValidationErrors =  !RunForPix&&false;
		constexpr bool CheckErrors =  !RunForPix&&false;
		constexpr bool DebugViews =  !RunForPix&&false;
#endif
	}
}