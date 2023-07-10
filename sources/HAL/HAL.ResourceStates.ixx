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

		//const ResourceState& GetSupportedStates(CommandListType type)
		//{
		//	if (type == CommandListType::COPY) return COPY_STATES;
		//	if (type == CommandListType::COMPUTE) return COMPUTE_STATES;
		//	return GRAPHIC_STATES;
		//}


		bool IsCompatible(CommandListType a,CommandListType b)
		{
			if (a == CommandListType::DIRECT) return true;
			if (b == CommandListType::DIRECT) return false;

			if (a == CommandListType::COMPUTE) return true;
			if (b == CommandListType::COMPUTE) return false;

			return true;

		}

		
		bool IsFullySupport(CommandListType type, const ResourceState& states)
		{
			return IsCompatible(type, states.get_best_cmd_type());
		}



		CommandListType Merge(CommandListType a,CommandListType b)
		{
			if (a == CommandListType::DIRECT||b == CommandListType::DIRECT) return CommandListType::DIRECT;
			if (a == CommandListType::COMPUTE||b == CommandListType::COMPUTE) return CommandListType::COMPUTE;
			return CommandListType::COPY;
		}



		std::optional<ResourceState> merge_state(const ResourceState& source, const ResourceState& need)
		{
			if (source == ResourceStates::UNKNOWN) return need;
		
			if (source.has_write_bits()||need.has_write_bits())
			{
				return std::nullopt;
			}

			if(source.is_no_access()||need.is_no_access())
			{
				return std::nullopt;
			}

			return source | need;
		}
		std::optional<TextureLayout> merge_layout(const TextureLayout& source, const TextureLayout& need)
		{
			if (source == TextureLayout::NONE) return need;
			if (source == need) return need;

			static const TextureLayout LAYOUT_WRITE = TextureLayout::UNORDERED_ACCESS | TextureLayout::DEPTH_STENCIL_WRITE | TextureLayout::RENDER_TARGET | TextureLayout::COPY_DEST;
			if (check(source & LAYOUT_WRITE) || check(need & LAYOUT_WRITE))
				return std::nullopt;

			return source | need;
		}
		enum class BarrierFlags : UINT
		{
			BEGIN = 1,
			END = 2,
			SINGLE = BEGIN | END,
			DISCARD = 4
		};

		struct Barrier
		{
			Resource* resource;
			ResourceState before;
			ResourceState after;
			UINT subres;
			BarrierFlags flags;
		};


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

			void transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags = BarrierFlags::SINGLE);

		};

		struct UsagePoint;

		struct ResourceUsage
		{
			Resource* resource = nullptr;
			ResourceState wanted_state;
			UINT subres = -1;

			ResourceUsage* prev_usage = nullptr;

			UsagePoint* usage = nullptr;
			UsagePoint* last_usage = nullptr;

			bool need_discard = false;
		};

		struct UsagePoint
		{
			std::list<HAL::ResourceUsage> usages;

			bool start = false;
			UsagePoint* prev_point = nullptr;
			UsagePoint* next_point = nullptr;

			UsagePoint(CommandListType type)
			{
			}
		};
		struct ResourceListStateCPU
		{
			ResourceUsage* first_usage = nullptr;
			ResourceUsage* last_usage = nullptr;

			ResourceState get_first_usage()
			{
				return first_usage->wanted_state;
			}

			ResourceState get_usage()
			{
				return last_usage->wanted_state;
			}

			bool used = false;
			bool need_discard = false;

			void reset()
			{
				need_discard = false;
				used = false;
				first_usage = nullptr;
				last_usage = nullptr;
			}


			ResourceUsage* add_usage(ResourceUsage* usage)
			{
				auto prev = last_usage;
				last_usage = usage;
				last_usage->prev_usage = prev;

				return usage;
			}

			ResourceUsage* set_zero_transition(ResourceUsage* usage)
			{
				first_usage->prev_usage = usage;
				first_usage = usage;

				return first_usage;
			}

		};

		struct SubResourcesGPU;
		struct SubResourcesCPU
		{
			std::vector<ResourceListStateCPU> subres;
		//	ResourceListStateCPU all_state;

		//	bool need_discard = false;
			bool used = false;

		//	bool all_states_same = true;
			void reset()
			{
				used = false;
		//		all_states_same = true;

		//		all_state.reset();

				for (auto& s : subres)
				{
					s.reset();
				}
			}

		/*	void make_all_state(Transition* last_usage)
			{
				if (all_states_same) return;
				assert(last_usage);
				all_states_same = true;

				all_state.used = true;
				all_state.last_usage = last_usage;
			}

			void make_unique_state()
			{
				if (!all_states_same) return;
				all_states_same = false;

				for (auto& s : subres)
				{
					if (!s.used)
					{
						s.first_usage = all_state.first_usage;
						s.used = s.first_usage;
					}

					s.last_usage = all_state.last_usage;
				}
			}*/

			const ResourceListStateCPU& get_subres_state(UINT id) const
			{
		/*		if ((!force && all_states_same) || id == ALL_SUBRESOURCES)
					return all_state;*/

				return subres[id];
			}


			ResourceListStateCPU& get_subres_state(UINT id) 
			{


				//if (all_states_same || id == ALL_SUBRESOURCES)
				//	return all_state;

				return subres[id];
			}

			ResourceUsage* get_first_usage(UINT id) const
			{
				//if (all_state.first_usage)
				//	return all_state.first_usage;

				return subres[id].first_usage;
			}

			ResourceUsage* get_last_usage(UINT id) const
			{
				//if (all_states_same)
				//	return all_state.last_usage;

				//if (id == ALL_SUBRESOURCES) return nullptr;

				return subres[id].last_usage;
			}


			ResourceState get_first_state(UINT id) const
			{
				//if (all_state.first_usage)
				//	return all_state.first_usage->wanted_state;

				return get_first_usage(id)->wanted_state;
			}

			ResourceState get_last_state(UINT id) const
			{
				return get_last_usage(id)->wanted_state;
			}


			void merge_read_state(CommandListType type, SubResourcesGPU& state);
		};


		struct ResourceListStateGPU
		{
			TextureLayout layout ;//= HAL::ResourceState::UNKNOWN;
		};

		struct SubResourcesGPU
		{
			std::vector<ResourceListStateGPU> subres;
		//	ResourceListStateGPU all_state;

		//	bool all_states_same = true;

			bool is_valid() const
			{
			//	if (all_states_same) return all_state.state!=HAL::ResourceState::UNKNOWN;
			for (auto& s : subres)
				{
			//	if(s.state !=HAL::ResourceState::UNKNOWN) 
					return true;

				}

			return false;
			}
//			void make_all_state(ResourceState state)
//			{
//				//		if (all_states_same) return;
//				all_states_same = true;
//				all_state.state = state;
//
//#ifdef DEV
//				for (auto& s : subres)
//					s.state = ResourceState::UNKNOWN;
//#endif
//			}
//
//			void make_unique_state()
//			{
//				if (!all_states_same) return;
//				all_states_same = false;
//
//				for (auto& s : subres)
//				{
//					s.state = all_state.state;
//				}
//#ifdef DEV
//				all_state.state = ResourceState::UNKNOWN;
//#endif
//			}


			CommandListType get_best_list_type()
			{
			/*	if (all_states_same)
					return	GetBestType(all_state.state);*/


				CommandListType type = CommandListType::COPY;

				for (auto& s : subres)
				{
					if(s.layout!=TextureLayout::NONE)
					type= Merge(type,get_best_cmd_type(s.layout));
				}


				return type;
			}
			void set_cpu_state(const SubResourcesCPU& cpu_state)
			{
				/*if (cpu_state.all_states_same)
				{
					auto all_state = cpu_state.get_last_state(ALL_SUBRESOURCES);
					make_all_state(all_state);
				}
				else
				{
					make_unique_state();*/

					for (int i = 0; i < subres.size(); i++)
					{
						auto& gpu = get_subres_state(i);
						auto& cpu = cpu_state.get_subres_state(i);

						if (!cpu.used)
						{
							continue;
						}

						auto last_usage = cpu_state.get_last_usage(i);

						auto last_state = last_usage->wanted_state;
					//	assert(last_state != ResourceState::UNKNOWN);
						gpu.layout = last_state.layout;
					}

				//}
			}

			void set_cpu_state_first(const SubResourcesCPU& cpu_state)
			{
				/*if (cpu_state.all_states_same)
				{
					auto all_state = cpu_state.get_last_state(ALL_SUBRESOURCES);
					make_all_state(all_state);
				}
				else
				{
					make_unique_state();*/

					for (int i = 0; i < subres.size(); i++)
					{
						auto& gpu = get_subres_state(i);
						auto& cpu = cpu_state.get_subres_state(i);

						if (!cpu.used)
						{
							continue;
						}

						auto last_usage = cpu_state.get_first_usage(i);

						auto last_state = last_usage->wanted_state;
					//	assert(last_state != ResourceState::UNKNOWN);
						gpu.layout = last_state.layout;
					}

				//}
			}


					const ResourceListStateGPU& get_subres_state(UINT id) const
			{
			/*	if (all_states_same || id == ALL_SUBRESOURCES)
					return all_state;*/

				return subres[id];
			}
 
			ResourceListStateGPU& get_subres_state(UINT id)
			{
			/*	if (all_states_same || id == ALL_SUBRESOURCES)
					return all_state;*/

				return subres[id];
			}



			void merge(SubResourcesCPU& other)
			{
					for (int i = 0; i < subres.size(); i++)
					{
						auto transition = other.get_first_usage(i);

						if (transition)
						{
							
							auto res = merge_layout(subres[i].layout, transition->wanted_state.layout);
							if(res)
								subres[i].layout = *res;
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

			bool manual_controlled = false;
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

			void init_subres(int count, TextureLayout layout) const
			{
				gpu_state.subres.resize(count);
			//	gpu_state.all_states_same = true;
			//	gpu_state.all_state.state = state;
				for (auto& e : gpu_state.subres)
					e.layout = layout;

				states.set_init_func([count](SubResourcesCPU& state)
					{
						state.subres.resize(count);
					});
			}


			SubResourcesCPU& get_cpu_state(Transitions* list) const;

	//		void stop_using(Transitions* list, UINT subres) const;


			bool is_used(Transitions* list) const;
		

			CommandListType process_transitions(Barriers& target, std::vector<Resource*>& discards, Transitions* list) const;

			void transition(Transitions* list, ResourceState state, unsigned int subres) const;
			bool transition(Transitions* from, Transitions* to) const;
			void prepare_state(Transitions* from,const SubResourcesGPU& subres) const;


			void alias_begin(Transitions* list)const;
void alias_end(Transitions* list)const;

			void prepare_after_state(Transitions* from,const SubResourcesGPU& subres) const;
		};


	}

}