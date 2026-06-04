export module Core:StateContext;

import stl.core;
import stl.threading;
import windows;
import :IdGenerator;
import :Threading;

export{
	template<class T, unsigned int part_count>
	class NonCopyableFastIndexer
	{

		mutable std::array<T, part_count> states;

		mutable  SpinLock states_lock;
		mutable std::map<int, T> state_map;

	public:
		T& operator[](unsigned int id)
		{
			if (id < part_count)
				return states[id];


			std::lock_guard<SpinLock> guard(states_lock);

			auto it = state_map.find(id);

			if (it == state_map.end())
			{
				auto& state = state_map[id];

				if (init_func)
					init_func(state);
				return state;
			}

			return it->second;
		}

		~NonCopyableFastIndexer()
		{

		}
		using init_func_type = std::function<void(T&)>;
		init_func_type init_func;
		void set_init_func(init_func_type f)
		{
			init_func = f;

			if (init_func)
			{
				for (auto& s : states)
				{
					init_func(s);
				}
				// Also reset existing overflow-map entries (ids >= part_count).
				// Without this, per-command-list CPU states with ids >= 128 retain
				// stale used=true flags and dangling first_usage/last_usage pointers
				// from the previous frame (usage_points.clear() frees them in
				// on_execute).  Accessing those dangling pointers in compile_transitions
				// is UB and prevents the correct PRESENT→COLOR_ATTACHMENT barrier from
				// being emitted, causing the swapchain image to render black.
				std::lock_guard<SpinLock> guard(states_lock);
				for (auto& [k, v] : state_map)
					init_func(v);
			}
		}
	};


	class StateContext;
	class ContextGenerator
	{
		IdGenerator<> generator;
		std::atomic_uint64_t global_id;
	public:

		void generate(StateContext* context);

		void free(StateContext* context);
	};


	class StateContext
	{

		template<class T>
		friend class ObjectState;
		friend class ContextGenerator;

		int local_id = -1;
		std::uint64_t global_id;

		ContextGenerator* owner;
	public:
		std::uint64_t get_global_id() const
		{
			return global_id;
		}
		StateContext() = default;
		void release();
	};



	template<class T>
	class ObjectState
	{

		class State : public T
		{
			friend class ObjectState<T>;
			std::uint64_t used_id;
		public:

			using T::reset;
			State() = default;
			State(State&&) noexcept = default;
		private:
			State(const State&) = delete;
			State& operator=(const State&) = delete;
		};

	protected:
		mutable NonCopyableFastIndexer<State, 128> states;

	public:

		T& get_state(StateContext* context) const
		{
			State& state = states[context->local_id];

			if (state.used_id != context->global_id)
			{
				state.used_id = context->global_id;
				state.reset();
			}

			return state;
		}
	};

}
