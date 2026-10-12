export module HAL:Debug;

import Core;
import :Types;

export namespace HAL
{

	namespace Debug
	{
		constexpr bool RunForPix = false;

		// Operation batching. Flip false to fall back to one usage point per op
		// (near the pre-batching behavior) — batched vs unbatched output must be
		// identical; any difference is a hazard-detection bug.
		inline bool EnableOpBatching = true;


#ifdef DEV
		constexpr bool GfxDebug = !RunForPix&&false;
		constexpr bool ValidationErrors = !RunForPix&&true;
		constexpr bool CheckErrors =  !RunForPix&&true;
		constexpr bool DebugViews =  !RunForPix&&false;
#else
		constexpr bool GfxDebug = false;
		constexpr bool ValidationErrors = false;
		constexpr bool CheckErrors = false;
		constexpr bool DebugViews = false;
#endif

		// Graphics-API validation errors (D3D12 debug-layer ERROR/CORRUPTION)
		// reported so far in this process. The test runner compares the count
		// before and after each test, so a GPU test that triggers one fails.
		class ValidationLog
		{
			// Function-local statics, not `static inline` members: clang (Android NDK)
			// emits a dynamic-init .init_array entry for a module-interface inline
			// variable that points at a comdat discarded in favour of the importer's
			// copy, failing the link with "relocation refers to a discarded section".
			struct State
			{
				std::atomic<uint64> count = 0;
				std::mutex          last_mutex;
				std::string         last;
			};

			static State& state()
			{
				static State s;
				return s;
			}

		public:
			static void report(std::string_view message)
			{
				auto& s = state();
				{
					std::lock_guard lock(s.last_mutex);
					s.last = message;
				}
				++s.count;
			}

			static uint64 error_count() { return state().count.load(); }

			static std::string last_error()
			{
				auto& s = state();
				std::lock_guard lock(s.last_mutex);
				return s.last;
			}
		};

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
