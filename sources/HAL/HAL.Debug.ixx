export module HAL:Debug;


export namespace HAL
{

	namespace Debug
	{
		constexpr bool RunForPix = false;

#ifdef DEV
		constexpr bool GfxDebug = !RunForPix&&true;
		constexpr bool ValidationErrors = !RunForPix&&true;
		constexpr bool CheckErrors =  !RunForPix&&true;
		constexpr bool DebugViews =  !RunForPix&&false;
#else
		constexpr bool GfxDebug = false;
		constexpr bool ValidationErrors = false;
		constexpr bool CheckErrors = false;
		constexpr bool DebugViews = false;
#endif
	}
}