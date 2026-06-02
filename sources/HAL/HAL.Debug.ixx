export module HAL:Debug;

import Core;
import :Types;

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

		// -----------------------------------------------------------------------
		//  Runtime barrier breakpoints.
		//  Set from the debug UI; checked at both recording time and compile time.
		// -----------------------------------------------------------------------

		// Uniquely identifies one barrier transition on a specific resource.
		struct BreakKey
		{
			std::string   resource_name;
			uint          subres = 0;
			ResourceState before;
			ResourceState after;

			bool operator==(const BreakKey&) const = default;
		};

		class BarrierBreakpoints
		{
		public:
			// Toggle a breakpoint on/off.
			// Returns true if the breakpoint is now active, false if it was removed.
			static bool toggle(const BreakKey& key);

			// Returns true if this exact key is registered.
			static bool has(const BreakKey& key);

			// Called on the render thread just before a barrier fires (exact match on before+after).
			static void check_barrier(std::string_view resource, uint subres,
			                          const ResourceState& before, const ResourceState& after);

			// Called at recording time when wanted_state is set.
			// Breaks when any registered key matches resource+subres+after (before is unknown here).
			static void check_usage(std::string_view resource, uint subres,
			                        const ResourceState& wanted);
		};


		//class CommandListDebugger
		//{
		//
		//public:

		//	void pre_command();
		//	void after_command();
		//};
	}
}
