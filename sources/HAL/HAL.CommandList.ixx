export module HAL:CommandList;
import <HAL.h>;
import Core;
import :Types;
import :ResourceStates;
import :Resource;
import :DescriptorHeap;
import :Fence;
import :FrameManager;
import :PipelineState;
import :API.IndirectCommand;
import :API.CommandList;
import :QueryHeap;
import :ResourceViews;
import :PSO;
import :CommandAllocator;
import :CommandListRecorder;

import :Autogen.Tables.DebugStruct;

export{

	namespace HAL
	{
	
		class CommandListBase : public StateContext, public GPUEntityStorageProxy
		{
		protected:
			CommandListType type;
			std::string name;
			std::vector<std::function<void()>> on_execute_funcs;


			std::list<TrackedObject::ptr> tracked_resources;
			FenceWaiter dstorage_fence;
			DelayedCommandList* get_native_list()
			{
				return &compiler;
			}
		public:
			FrameResources::ptr frame_resources;
			DelayedCommandList compiler;
			template<TrackableClass T>
			void track_object(T& obj)
			{
				auto& state = obj.ObjectState<TrackedObjectState>::get_state(this);
				if (!state.used)
				{
					state.used = true;
					tracked_resources.emplace_back(obj.get_tracked());

					if constexpr (std::is_same_v<T, Resource>)
					{
						dstorage_fence.combine(obj.load_waiter);
					}
				}
			}

			void free_tracked_objects()
			{
				tracked_resources.clear();
				dstorage_fence = FenceWaiter();
			}
			virtual ~CommandListBase() = default;

			CommandListType get_type()
			{
				return type;
			}
		};

		class TransitionCommandList;


		class Transitions : public virtual CommandListBase
		{
			std::list<HAL::Resource*> used_resources;

			//		std::shared_ptr<TransitionCommandList> transition_list;

			friend class SignatureDataSetter;
			friend class Sendable;
			friend class Eventer;
			friend class ResourceStateManager;
		protected:
			void begin();
			void on_execute();
			std::list<HAL::UsagePoint> usage_points;

			void create_usage_point(bool end = true)

			{
				auto prev_point = usage_points.empty() ? nullptr : &usage_points.back();
				auto point = &usage_points.emplace_back(type);


				if (prev_point) prev_point->next_point = point;
				point->prev_point = prev_point;

				point->start = !end;
				point->index = usage_points.size();

				compiler.func([point](auto& list)
					{
						list.transitions(point->transitions);
					});
			}

			void compile_transitions()
			{

				for (auto& point : usage_points)
				{
					for (auto& usage : point.usages)
					{
						auto prev_usage = usage.prev_usage;
						auto prev_state = ResourceStates::NO_ACCESS;

						if (!prev_usage && !usage.need_discard)  continue;
						if (prev_usage)prev_state = prev_usage->wanted_state;

						if (prev_state == usage.wanted_state) continue;

						assert(prev_state.is_valid());
						assert(usage.wanted_state.is_valid());


						auto a = prev_state.get_best_cmd_type();
						auto b = usage.wanted_state.get_best_cmd_type();

						assert(IsCompatible(type, a));
						assert(IsCompatible(type, b));
						BarrierFlags flags = BarrierFlags::NONE;

						if (usage.need_discard)
							flags |= BarrierFlags::DISCARD;

						auto last_point = prev_usage ? prev_usage->last_usage : nullptr;

						if (false && last_point)
						{

							auto sync_state = usage.wanted_state;

							sync_state.operation = BarrierSync::SPLIT;

							last_point->transitions.transition(usage.resource,
								prev_state,
								sync_state,
								usage.subres, flags);


							point.transitions.transition(usage.resource,
								sync_state,
								usage.wanted_state,
								usage.subres, flags);


						}
						else
						{
							point.transitions.transition(usage.resource,
								prev_state,
								usage.wanted_state,
								usage.subres, flags);
						}


					}
				}

			}

			void transition(const HAL::Resource* resource, ResourceState state, UINT subres = ALL_SUBRESOURCES);
			void transition(const HAL::Resource::ptr& resource, ResourceState state, UINT subres = ALL_SUBRESOURCES);

		public:
			void free_resources();

			UINT transition_count = 0;
			//	UsagePoint* start_transition;
			HAL::ResourceUsage* add_usage(const HAL::Resource* resource, UINT subres, ResourceState state, HAL::TransitionType type = HAL::TransitionType::LAST)
			{
				HAL::UsagePoint* point = nullptr;

				if (type == HAL::TransitionType::LAST) point = &usage_points.back();
				if (type == HAL::TransitionType::ZERO) point = &usage_points.front();


				//if (type == TransitionType::LAST) 			assert(!point->start);
				HAL::ResourceUsage& usage = point->usages.emplace_back();

				usage.resource = const_cast<HAL::Resource*>(resource);
				usage.subres = subres;
				usage.wanted_state = state;

				usage.usage = point;
				return &usage;
			}

			HAL::UsagePoint* get_last_usage_point()
			{
				return &usage_points.back();
			}

			void use_resource(const HAL::Resource* resource);
		public:

			void alias_begin(HAL::Resource*);
			void alias_end(HAL::Resource*);

			std::shared_ptr<TransitionCommandList> fix_pretransitions();

			void transition_present(const HAL::Resource* resource_ptr)
			{

				create_usage_point();

				transition(resource_ptr, { BarrierSync::NONE, BarrierAccess::NO_ACCESS, TextureLayout::PRESENT }, ALL_SUBRESOURCES);

				create_usage_point(false);
			}


			void transition(const ResourceInfo& info, BarrierSync operation = BarrierSync::NONE)
			{
				if (!info.is_valid()) return;

				ResourceState target_state;//= ResourceState::COMMON;


				if (std::holds_alternative<HAL::Views::ShaderResource>(info.view))
				{
					if (type == CommandListType::DIRECT)
					{
						operation = BarrierSync::ALL_SHADING;// | BarrierSync::DRAW ;
					}

					if (type == CommandListType::COMPUTE)
					{
						operation = BarrierSync::COMPUTE_SHADING;//  ResourceStates::NON_PIXEL_SHADER_RESOURCE;
					}

					target_state = { operation, BarrierAccess::SHADER_RESOURCE, TextureLayout::SHADER_RESOURCE };  //TODO BarrierSync::ALL

				}
				else 	if (std::holds_alternative<HAL::Views::UnorderedAccess>(info.view))
				{
					//assert( operation != BarrierSync::NONE);

					if (type == CommandListType::DIRECT)
					{
						operation = BarrierSync::ALL_SHADING;// | BarrierSync::DRAW ;
					}

					if (type == CommandListType::COMPUTE)
					{
						operation = BarrierSync::COMPUTE_SHADING;//  ResourceStates::NON_PIXEL_SHADER_RESOURCE;
					}

					target_state = { operation, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNORDERED_ACCESS };  //TODO BarrierSync::ALL
				}
				else 	if (std::holds_alternative<HAL::Views::RenderTarget>(info.view))
				{
					target_state = ResourceStates::RENDER_TARGET;
				}
				else 	if (std::holds_alternative<HAL::Views::DepthStencil>(info.view))
				{
					target_state = ResourceStates::DEPTH_STENCIL;

				}
				else if (std::holds_alternative<HAL::Views::ConstantBuffer>(info.view))
				{
					if (type == CommandListType::DIRECT)
					{
						operation = BarrierSync::ALL_SHADING;// | BarrierSync::DRAW ;
					}

					if (type == CommandListType::COMPUTE)
					{
						operation = BarrierSync::COMPUTE_SHADING;//  ResourceStates::NON_PIXEL_SHADER_RESOURCE;
					}

					target_state = { operation, BarrierAccess::CONSTANT_BUFFER, TextureLayout::UNDEFINED };  //TODO BarrierSync::ALL

				}
				else assert(false);


				//if(GetAsyncKeyState('4'))
				//{
				//	if (type == CommandListType::DIRECT)
				//	{
				//		operation =BarrierSync::ALL_DIRECT;// operation = ResourceStates::PIXEL_SHADER_RESOURCE | ResourceStates::NON_PIXEL_SHADER_RESOURCE;
				//	}

				//	if (type == CommandListType::COMPUTE)
				//	{
				//		operation =BarrierSync::ALL_COMPUTE;//  ResourceStates::NON_PIXEL_SHADER_RESOURCE;
				//	}
				//
				//}
				info.for_each_subres([&](const HAL::Resource::ptr& resource, UINT subres)
					{
						transition(resource.get(), target_state, subres);
					});
			}

			void stop_using(const ResourceInfo& info)
			{
				if (!info.is_valid()) return;


				info.for_each_subres([&](const HAL::Resource::ptr& resource, UINT subres)
					{
						const_cast<HAL::Resource*>(resource.get())->get_state_manager().stop_using(this, subres);
					});
			}
		};



		class GPUTimer :public GPUTimerInterface
		{
		public://		friend class GPUTimeManager;

			QueryHandle querys;

		public:
			CommandListType queue_type;
			GPUTimer();

			virtual ~GPUTimer();

			void start(Eventer* list);

			void end(Eventer* list);



			uint64 get_start() override;

			uint64 get_end() override;


		};
		struct GPUTimers
		{
			std::list<GPUTimer> timers;
			void reset()
			{
				timers.clear();
			}
		};

		class GPUBlock :public TimedBlock, public ObjectState<GPUTimers>
		{
			Device& device;
		public:


			GPUBlock(std::wstring_view name, Device& device) :TimedBlock(name), device(device) {}
			void start(Eventer* list);

			void end(Eventer* list);
		};


		class Eventer : public virtual CommandListBase, public TimedRoot
		{
			friend class GPUBlock;
			std::list< GPUBlock*> gpu_timers;
			std::list<std::wstring> names;
			TimedBlock* current;
			bool started = false;
			virtual  void on_start(Timer* timer) override;
			virtual  void on_end(Timer* timer)override;
		protected:
#ifdef DEV
			Exceptions::stack_trace begin_stack;
#endif


			void reset();
			void begin(std::string name, Timer* t = nullptr);
		public:
			void end();
			Eventer(Device& device) {}
			static thread_local Eventer* thread_current;

			virtual Timer start(std::wstring_view name)override;

			std::shared_ptr<Timer> timer;
			// events
			void start_event(std::wstring str);
			void end_event();

			void set_marker(const wchar_t* label);

			// timers
			void insert_time(QueryHandle& handle, uint offset);
			void resolve_times(QueryHeap* pQueryHeap, uint32_t NumQueries, std::function<void(std::span<UINT64>)>);



		};



		class Sendable : public virtual CommandListBase
		{
		protected:
			bool active = false;
			friend class Queue;

		public:
			virtual void end() = 0;
			void compile();


			FenceWaiter execute(std::function<void()> f = nullptr);
			void execute_and_wait(std::function<void()> f = nullptr);

			void on_done(std::function<void()> f);

		};

		class SignatureDataSetter;


		class CommandList : public Transitions, public Eventer, public Sendable, public SharedObject<CommandList>
		{

		public:
			using ptr = std::shared_ptr<CommandList>;
		protected:
			CommandList() = default;
			friend class Queue;
			friend class FrameResourceManager;
			friend class CopyContext;
			friend class GraphicsContext;
			friend class ComputeContext;
			friend class SignatureDataSetter;
			friend class Sendable;
			friend class Eventer;

			// TODO: make references?

			virtual void on_execute();

			std::unique_ptr<GraphicsContext> graphics;
			std::unique_ptr<ComputeContext> compute;
			std::unique_ptr<CopyContext> copy;
			PipelineStateBase* current_pipeline;
			PipelineStateBase* first_pipeline;

			std::list<HAL::update_tiling_info> tile_updates;



			void set_pipeline_internal(PipelineStateBase* pipeline);

			template<bool compute, bool graphics, class T>
			void pre_command(T& context, BarrierSync operation, UsedSlots* slots = nullptr)
			{
				create_usage_point();
				if constexpr (compute || graphics)
				{
					if constexpr (Debug::GfxDebug)	setup_debug(&context);
					context.commit_tables(operation, slots);
					if constexpr (graphics) context.validate();
				}
			}
			template<bool compute, bool graphics, class T>
			void post_command(T& context)
			{
				create_usage_point(false);
				if constexpr (Debug::GfxDebug)
					if constexpr (compute || graphics)	print_debug();
			}
		public:
			void end();

			void update_tilings(HAL::update_tiling_info&& info)
			{
				tile_updates.emplace_back(std::move(info));

				track_object(*(info.resource));
			}


			void setup_debug(SignatureDataSetter*);
			void print_debug();
			bool first_debug_log = true;
			StructuredBufferView<Table::DebugStruct> debug_buffer;

			GraphicsContext& get_graphics();
			ComputeContext& get_compute();
			CopyContext& get_copy();


			CommandList(CommandListType);

			void begin(std::string name = "", Timer* t = nullptr);


			void clear_uav(const UAVHandle& h, vec4 ClearColor = vec4(0, 0, 0, 0))
			{
				create_usage_point();
				transition(h.get_resource_info(), BarrierSync::CLEAR_UNORDERED_ACCESS_VIEW);
				compiler.clear_uav(h, ClearColor);
				create_usage_point(false);
			}

		};

		class CopyContext
		{
			friend class CommandList;

			CommandList& base;
			DelayedCommandList* list;

			CopyContext(CommandList& base) :base(base), list(base.get_native_list()) {}
			CopyContext(const CopyContext&) = delete;
			CopyContext(CopyContext&&) = delete;
		public:
			void copy_resource(HAL::Resource* dest, HAL::Resource* source);
			void copy_resource(const HAL::Resource::ptr& dest, const HAL::Resource::ptr& source);
			void copy_texture(const HAL::Resource::ptr& dest, int, const HAL::Resource::ptr& source, int);
			void copy_texture(const HAL::Resource::ptr& dest, ivec3, const HAL::Resource::ptr& source, ivec3, ivec3);
			void copy_buffer(HAL::Resource* dest, uint64 s_dest, HAL::Resource* source, uint64 s_source, uint64 size);



			//TODO: remove
			void update_buffer(HAL::Resource::ptr resource, uint64 offset, const char* data, uint64 size);
			void update_texture(HAL::Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);
			void update_buffer(HAL::Resource* resource, uint64 offset, const char* data, uint64 size);
			void update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);

		

			std::future<bool> read_texture(const HAL::Resource* resource, UINT sub_resource, std::function<void(std::span<std::byte>, texture_layout)>);

			std::future<bool> read_texture(HAL::Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(std::span<std::byte>, texture_layout)>);
			std::future<bool> read_texture(const HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(std::span<std::byte>, texture_layout)>);
			std::future<bool> read_buffer(HAL::Resource* resource, unsigned int offset, UINT64 size, std::function<void(std::span<std::byte>)>);
			std::future<bool> read_query(std::shared_ptr<QueryHeap>&, unsigned int offset, unsigned int count, std::function<void(std::span<std::byte>)>);



			template<class T>
			void update(HAL::StructuredBufferView<T>& view, uint64 offset, std::span<typename HAL::StructuredBufferView<T>::UnderlyingType> data)
			{
				update_buffer(view.resource, view.desc.offset + offset * sizeof(HAL::StructuredBufferView<T>::UnderlyingType), reinterpret_cast<const char*>(data.data()), data.size_bytes());

			}

				
			template<class T>
			std::future<bool> read(HAL::StructuredBufferView<T>& view, unsigned int offset, UINT64 count, std::function<void(std::span<T>)> f)
			{
				return read_buffer(view.resource.get(), view.desc.offset + offset * sizeof(HAL::StructuredBufferView<T>::UnderlyingType), count * sizeof(HAL::StructuredBufferView<T>::UnderlyingType),
					[f](std::span<std::byte> memory)
					{
						uint read = memory.size();
						auto data = reinterpret_cast<T*>(memory.data());


						f({ data,read });
					});

			}
			



		};


		class SignatureDataSetter : public GPUEntityStorageProxy
		{
			struct RowInfo
			{
				bool dirty = false;

				SlotID slot_id;
				CBVHandle const_buffer;
				std::vector<HAL::ResourceInfo*> resources;
				std::set<std::shared_ptr<HAL::DescriptorHeapStorage>> descriptors;
			};
			std::vector<RowInfo> tables;

			friend class CommandList;

			RootSignature::ptr root_sig;
			UsedSlots used_slots;
		protected:
			CommandList& base;
			SignatureDataSetter(CommandList& base) :base(base) {
				tables.resize(32); // !!!!!!!!!!!
			}


			virtual void set_const_buffer(UINT i, UINT offset, UINT v) = 0;

			void stop_using(uint id);
			void reset_tables()
			{
				root_sig = nullptr;
				for (auto& table : tables)
				{
					table.const_buffer = CBVHandle();
					table.resources.clear();
					table.descriptors.clear();
					table.dirty = false;
				}
			}
			//	public:
			void commit_tables(BarrierSync operation, UsedSlots* slots = nullptr);
			virtual void on_set_signature(const RootSignature::ptr& signature) = 0;

		public:

			void reset()
			{
				used_slots.clear();
				tables.clear();
				tables.resize(32);

			}
			CommandList& get_base() {
				return base;
			}

			void set_signature(const RootSignature::ptr& signature)
			{
				if (root_sig == signature) return;

				root_sig = signature;

				auto& desc = signature->get_desc();

				on_set_signature(signature);
			}


			void set_signature(Layouts layout)
			{
				set_signature(HAL::Device::get().get_engine_pso_holder().GetSignature(layout));
			}



			void set_pipeline(std::shared_ptr<PipelineStateBase> pipeline);

			template<class T>
			void set_pipeline(KeyPair<typename T::Keys> k = KeyPair<typename T::Keys>())
			{
				set_pipeline(Device::get().get_engine_pso_holder().GetPSO<T>(k));
			}

			void set_cb(UINT index, const CBVHandle& cb, BarrierSync operation)
			{
				get_base().transition(cb.get_resource_info(), operation);
				set_const_buffer(index, 0, cb.get_offset());
			}

			template<class T>void set(const T& compiled)
			{
				set(compiled.compile(*this));
			}

			template<SIG_TYPES_COMPILED::Slot Compiled>
			void set(const Compiled& compiled)
			{
				stop_using(Compiled::Slot::ID);
				auto& table = tables[Compiled::Slot::ID];
				table.slot_id = Compiled::ID;
				//	assert(!table.dirty);
				table.dirty = true;

				table.const_buffer = compiled.const_buffer;
				table.resources = compiled.resources;
				table.descriptors = compiled.descriptors;
				//	compiled.set_tables(*this);
			}



			template<class T>
			std::unique_ptr<T> wrap()
			{
				auto res = std::make_unique<T>();
				res->begin(this);
				return res;
			}
		};




		class GraphicsContext;
		struct CompiledRT
		{
			static constexpr SIG_TYPE_COMPILED TYPE = SIG_TYPE_COMPILED::RT;


			HAL::RTVHandle table_rtv;
			HAL::DSVHandle table_dsv;

			std::vector<HAL::Format> get_formats() const;
			HAL::Format get_depth_format() const;

		};


		class GraphicsContext : public SignatureDataSetter
		{
			friend class CommandList;

			DelayedCommandList* list;

			GraphicsContext(CommandList& base) :SignatureDataSetter(base), list(base.get_native_list()) {
			}
			GraphicsContext(const GraphicsContext&) = delete;
			GraphicsContext(GraphicsContext&&) = delete;

			bool valid_scissor = false;
			std::vector<Viewport> viewports;


			void begin();
			void end();
			void on_execute();

			void set_const_buffer(UINT i, UINT offset, UINT v)override;

			HAL::Views::IndexBuffer index;
			CompiledRT compiled_rt;


			void set_heaps(DescriptorHeap::ptr& a, DescriptorHeap::ptr& b);

			void on_set_signature(const RootSignature::ptr&) override;

			void validate();
		public:
			std::vector<HAL::Format> get_formats() const
			{
				return compiled_rt.get_formats();
			}

			template<SIG_TYPES::RT RT>
			CompiledRT set_rtv(const RT& rt, RTOptions options = RTOptions::Default, float depth = 1, uint stencil = 0)
			{

				auto compiled = rt.compile(*this);

				set_rtv(compiled, options, depth, stencil);

				return compiled;
			}

			void set_rtv(const CompiledRT& rt, RTOptions options = RTOptions::Default, float depth = 1, uint stencil = 0);

			CommandList& get_base()
			{
				return base;
			}

			void set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType = HAL::PrimitiveTopologyFeed::LIST, bool adjusted = false, uint controlpoints = 0)
			{
				list->set_topology(topology, feedType, adjusted, controlpoints);
			}




			void set_scissor(sizer_long rect);
			void set_viewport(Viewport viewport);
			void set_viewport(vec4 viewport);
			void set_scissors(sizer_long rect);
			void set_viewports(std::vector<Viewport> viewports);



			void draw(D3D12_DRAW_INDEXED_ARGUMENTS args)
			{
				draw_indexed(args.IndexCountPerInstance, args.StartIndexLocation, args.BaseVertexLocation, args.InstanceCount, args.StartInstanceLocation);
			}

			void dispatch_mesh(ivec3 v);
			void dispatch_mesh(D3D12_DISPATCH_MESH_ARGUMENTS args);

			void set_stencil_ref(UINT ref)
			{
				list->set_stencil_ref(ref);
			}

			std::vector<Viewport> get_viewports()
			{
				return viewports;
			}

			void set_index_buffer(HAL::Views::IndexBuffer view)
			{
				index = view;

			}



			void draw(UINT vertex_count, UINT vertex_offset = 0, UINT instance_count = 1, UINT instance_offset = 0);
			void draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count = 1, UINT instance_offset = 0);
			void execute_indirect(IndirectCommand& command_types, UINT max_commands, HAL::Resource* command_buffer, UINT64 command_offset = 0, HAL::Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);


			
			template<class T>
			void exec_indirect(IndirectCommand& command_types,HAL::StructuredBufferView<T>& buffer, UINT max_commands, UINT offset = 0  )
			{
				execute_indirect(
						command_types,
						max_commands,
						buffer.resource.get(),
						buffer.get_data_offset_in_bytes(offset),
						buffer.get_counter_buffer().get(),
						buffer.get_counter_offset()
					);
			}

		};





		class ComputeContext : public SignatureDataSetter
		{
			friend class CommandList;


			DelayedCommandList* list;

			ComputeContext(CommandList& base) :SignatureDataSetter(base), list(base.get_native_list()) {}
			ComputeContext(const ComputeContext&) = delete;
			ComputeContext(ComputeContext&&) = delete;


			//	std::shared_ptr<ComputePipelineState> current_compute_pipeline;
			std::shared_ptr<RootSignature> current_compute_root_signature;

			void begin();
			void end();
			void on_execute();



			virtual void set_const_buffer(UINT i, UINT offset, UINT v) override;

			void on_set_signature(const RootSignature::ptr&) override;


		public:

			CommandList& get_base()
			{
				return base;
			}
			
			template<class T>
			void clear_counter(HAL::StructuredBufferView<T>& view)
			{
				get_base().clear_uav(view.counter_view.rwRAW);
			}

			void dispatch(int = 1, int = 1, int = 1);
			void dispatch(ivec2, ivec2 = ivec2(8, 8));
			void dispatch(ivec3, ivec3 = ivec3(4, 4, 4));




			void execute_indirect(IndirectCommand& command_types, UINT max_commands, HAL::Resource* command_buffer, UINT64 command_offset = 0, HAL::Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);



			void build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom);
			void build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top);

			template<class Hit, class Miss, class Raygen>
			void dispatch_rays(ivec2 size, HAL::ResourceAddress hit_buffer, UINT hit_count, HAL::ResourceAddress miss_buffer, UINT miss_count, HAL::ResourceAddress raygen_buffer)
			{
				base.pre_command<true, false>(*this, BarrierSync::COMPUTE_SHADING);

				base.transition(hit_buffer.resource, { BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED });
				base.transition(miss_buffer.resource, { BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED });
				base.transition(raygen_buffer.resource, { BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED });

				list->dispatch_rays<Hit, Miss, Raygen>(size, hit_buffer, hit_count, miss_buffer, miss_count, raygen_buffer);

				base.post_command<true, false>(*this);
			}


			template<class T>
			void exec_indirect(IndirectCommand& command_types,HAL::StructuredBufferView<T>& buffer, UINT max_commands, UINT offset = 0  )
			{
				execute_indirect(
						command_types,
						max_commands,
						buffer.resource.get(),
						buffer.get_data_offset_in_bytes(offset),
						buffer.get_counter_buffer().get(),
						buffer.get_counter_offset()
					);
			}


		};


		class TransitionCommandList : public Eventer, public Sendable
		{

		public:
			using ptr = std::shared_ptr<TransitionCommandList>;
			inline CommandListType get_type() { return type; }
			TransitionCommandList(CommandListType type);
			void create_transition_list(FrameResources& frame, const HAL::Barriers& transitions, std::vector<HAL::Resource*>& duscards);

			const API::CommandList& get_compiled() const { return compiler.get_list(); }

			void end() override {}
			void on_execute()
			{

				for (auto&& t : on_execute_funcs)
					t();

				on_execute_funcs.clear();

				Eventer::reset();
			}


		};
	}

	namespace Helpers
	{
	template<class T>
			static auto make_buffer(std::span<T> v)
			{
				HAL::StructuredBufferView<T> buffer(v.size());

	auto list = (HAL::Device::get().get_upload_list());
			list->get_copy().update(buffer, 0, v);
			list->execute_and_wait();

				return buffer;
			}
	}

}