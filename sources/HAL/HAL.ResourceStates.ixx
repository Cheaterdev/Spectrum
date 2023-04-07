export module HAL:ResourceStates;

import Core;

import :Types;

using namespace HAL;

export
{

	namespace HAL
	{
		class Transitions;


		constexpr uint ALL_SUBRESOURCES = std::numeric_limits<uint>::max();

		enum class TransitionType :int
		{
			ZERO,
			LAST
		};

		const ResourceState& GetSupportedStates(CommandListType type)
		{
			if (type == CommandListType::COPY) return COPY_STATES;
			if (type == CommandListType::COMPUTE) return COMPUTE_STATES;
			return GRAPHIC_STATES;
		}


		bool IsFullySupport(CommandListType type, const ResourceState& states)
		{
			return (GetSupportedStates(type) & states) == states;
		}


		CommandListType GetBestType(const ResourceState& states, CommandListType preferred_type)
		{
			if (states == ResourceState::COMMON)
				return CommandListType::DIRECT;

			if ((states & COPY_STATES) == states && preferred_type == CommandListType::COPY)
			{
				return CommandListType::COPY;
			}

			if ((states & COMPUTE_STATES) == states && (preferred_type == CommandListType::COMPUTE || preferred_type == CommandListType::COPY))
			{
				return CommandListType::COMPUTE;
			}

			return CommandListType::DIRECT;
		}

		bool can_merge_state(const ResourceState& source, const ResourceState& need)
		{
			assert(source != ResourceState::UNKNOWN);
			assert(need != ResourceState::UNKNOWN);

			if (source == need) return true;

			ResourceState merged = source | need;
			if (check(merged & (~ResourceState::GEN_READ)))
			{
				return false;
			}

			return true;
		}


		ResourceState merge_state(const ResourceState& source, const ResourceState& need)
		{
			assert(need != ResourceState::UNKNOWN);

			if (source == ResourceState::UNKNOWN) return need;
			if (source == need) return source;

			ResourceState merged = source | need;
			if (check(merged & (~ResourceState::GEN_READ)))
			{
				return need;
			}

			return merged;
		}

		enum class BarrierFlags : UINT
		{
			BEGIN = 1,
			END = 2,
			SINGLE = BEGIN | END
		};

		struct BarrierTransition
		{
			Resource* resource;
			ResourceState before;
			ResourceState after;
			UINT subres;
			BarrierFlags flags;
		};

		struct BarrierUAV
		{
			Resource* resource;
			
		};
		struct BarrierAlias
		{
			Resource* before;
			Resource* after;

		};


		using Barrier = std::variant<BarrierTransition, BarrierUAV,BarrierAlias>;

		class Barriers
		{


			std::vector<Barrier> barriers;

			void validate();
			CommandListType type;
		public:

			Barriers(CommandListType type);
			inline operator bool() const
			{
				return !barriers.empty();
			}
			void clear();
			const std::vector<Barrier>& get_barriers() const;
			void uav(Resource* resource);
			void alias(Resource* from, Resource* to);

			void transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags = BarrierFlags::SINGLE);

		};

		struct TransitionPoint;

		struct Transition
		{
			Resource* resource = nullptr;
			ResourceState wanted_state = ResourceState::UNKNOWN;
			UINT subres = -1;

			Transition* prev_transition = nullptr;

			TransitionPoint* point = nullptr;
			TransitionPoint* last_used_point = nullptr;

			BarrierFlags flags = BarrierFlags::SINGLE;
		};

		struct TransitionPoint
		{
			std::list<HAL::Transition> transitions;
			std::list<Resource*> uav_transitions;
			std::list<Resource*> aliasing;

			//HAL::Barriers  compiled_transitions;

			bool start = false;
			TransitionPoint* prev_point = nullptr;
			TransitionPoint* next_point = nullptr;

			TransitionPoint(CommandListType type)// :compiled_transitions(type)
			{
			}
		};
		struct ResourceListStateCPU
		{
			Transition* first_transition = nullptr;
			Transition* last_transition = nullptr;

			ResourceState get_first_state()
			{
				return first_transition->wanted_state;
			}

			ResourceState get_state()
			{
				return last_transition->wanted_state;
			}

			bool used = false;

			void reset()
			{
				used = false;
				first_transition = nullptr;
				last_transition = nullptr;
			}


			Transition* add_transition(Transition* transition)
			{
				auto prev = last_transition;
				last_transition = transition;
				last_transition->prev_transition = prev;

				return transition;
			}

			Transition* set_zero_transition(Transition* transition)
			{
				first_transition->prev_transition = transition;
				first_transition = transition;

				return transition;
			}

		};

		struct SubResourcesGPU;
		struct SubResourcesCPU
		{
			std::vector<ResourceListStateCPU> subres;
			ResourceListStateCPU all_state;

			bool need_discard = false;
			bool used = false;

			bool all_states_same = true;
			void reset()
			{
				used = false;
				all_states_same = true;

				all_state.reset();

				for (auto& s : subres)
				{
					s.reset();
				}
			}

			void make_all_state(Transition* last_transition)
			{
				if (all_states_same) return;
				assert(last_transition);
				all_states_same = true;

				all_state.used = true;
				all_state.last_transition = last_transition;
			}

			void make_unique_state()
			{
				if (!all_states_same) return;
				all_states_same = false;

				for (auto& s : subres)
				{
					if (!s.used)
					{
						s.first_transition = all_state.first_transition;
						s.used = s.first_transition;
					}

					s.last_transition = all_state.last_transition;
				}
			}
			ResourceListStateCPU& get_subres_state(UINT id, bool force = false)
			{
				if ((!force && all_states_same) || id == ALL_SUBRESOURCES)
					return all_state;

				return subres[id];
			}


			const ResourceListStateCPU& get_subres_state(UINT id) const
			{


				if (all_states_same || id == ALL_SUBRESOURCES)
					return all_state;

				return subres[id];
			}

			Transition* get_first_transition(UINT id) const
			{
				if (all_state.first_transition)
					return all_state.first_transition;

				return subres[id].first_transition;
			}

			Transition* get_last_transition(UINT id) const
			{
				if (all_states_same)
					return all_state.last_transition;

				if (id == ALL_SUBRESOURCES) return nullptr;

				return subres[id].last_transition;
			}


			ResourceState get_first_state(UINT id) const
			{
				if (all_state.first_transition)
					return all_state.first_transition->wanted_state;

				return get_first_transition(id)->wanted_state;
			}

			ResourceState get_last_state(UINT id) const
			{
				return get_last_transition(id)->wanted_state;
			}


			void prepare_for(CommandListType type, SubResourcesGPU& state);
		};


		struct ResourceListStateGPU
		{
			ResourceState state = HAL::ResourceState::UNKNOWN;
		};

		struct SubResourcesGPU
		{
			std::vector<ResourceListStateGPU> subres;
			ResourceListStateGPU all_state;

			bool all_states_same = true;


			void make_all_state(ResourceState state)
			{
				//		if (all_states_same) return;
				all_states_same = true;
				all_state.state = state;

#ifdef DEV
				for (auto& s : subres)
					s.state = ResourceState::UNKNOWN;
#endif
			}

			void make_unique_state()
			{
				if (!all_states_same) return;
				all_states_same = false;

				for (auto& s : subres)
				{
					s.state = all_state.state;
				}
#ifdef DEV
				all_state.state = ResourceState::UNKNOWN;
#endif
			}



			void set_cpu_state(const SubResourcesCPU& cpu_state)
			{
				if (cpu_state.all_states_same)
				{
					auto all_state = cpu_state.get_last_state(ALL_SUBRESOURCES);
					make_all_state(all_state);
				}
				else
				{
					make_unique_state();

					for (int i = 0; i < subres.size(); i++)
					{
						auto& gpu = get_subres_state(i);
						auto& cpu = cpu_state.get_subres_state(i);

						if (!cpu.used)
						{
							continue;
						}

						auto last_transition = cpu_state.get_last_transition(i);

						auto last_state = last_transition->wanted_state;
						assert(last_state != ResourceState::UNKNOWN);
						gpu.state = last_state;
					}

				}
			}
			ResourceListStateGPU& get_subres_state(UINT id)
			{
				if (all_states_same || id == ALL_SUBRESOURCES)
					return all_state;

				return subres[id];
			}



			void merge(SubResourcesCPU& other)
			{
				{
					if (!other.all_state.first_transition)
					{
						make_unique_state();
					}
				}


				if (all_states_same)
				{
					all_state.state = merge_state(all_state.state, other.get_first_state(ALL_SUBRESOURCES));
				}
				else
				{
					for (int i = 0; i < subres.size(); i++)
					{
						auto transition = other.get_first_transition(i);

						if (transition)
							subres[i].state = merge_state(subres[i].state, transition->wanted_state);
					}
				}
			}


		};



		class ResourceStateManager : public ObjectState<SubResourcesCPU>
		{
			const Resource* resource;

		protected:
			mutable SubResourcesGPU gpu_state;
			

		public:
			virtual ~ResourceStateManager() = default;

			using ptr = std::unique_ptr<ResourceStateManager>;
			UINT get_subres_count()
			{
				return static_cast<UINT>(gpu_state.subres.size());
			}
			ResourceStateManager(const Resource* resource);
			SubResourcesGPU copy_gpu() const
			{
				return gpu_state;
			}

			void init_subres(int count, ResourceState state) const
			{
				gpu_state.subres.resize(count);
				gpu_state.all_states_same = true;
				gpu_state.all_state.state = state;
				for (auto& e : gpu_state.subres)
					e.state = state;

				states.set_init_func([count](SubResourcesCPU& state)
					{
						state.subres.resize(count);
					});
			}


			SubResourcesCPU& get_cpu_state(Transitions* list) const;

			void stop_using(Transitions* list, UINT subres) const;


			bool is_used(Transitions* list) const;
			void aliasing(int id, uint64_t full_id) const
			{
				//SubResourcesCPU& s = get_state(id);
			//	if (check(s.subres[0].state & ResourceState::RENDER_TARGET) || check(s.subres[0].state & ResourceState::DEPTH_WRITE))
			//	s.need_discard = true;
			}

			ResourceState process_transitions(Barriers& target, std::vector<Resource*>& discards, Transitions* list) const;

			void transition(Transitions* list, ResourceState state, unsigned int subres) const;
			bool transition(Transitions* from, Transitions* to) const;
			void prepare_state(Transitions* from, SubResourcesGPU& subres) const;



			void prepare_after_state(Transitions* from, SubResourcesGPU& subres) const;
		};


	}

}