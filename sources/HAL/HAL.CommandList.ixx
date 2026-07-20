export module HAL:CommandList;

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
			LiteralWStr name{L""};
			std::vector<std::function<void()>> on_execute_funcs;


			std::vector<TrackedObject::ptr> tracked_resources;
			FenceWaiter dstorage_fence;
			DelayedCommandList* get_native_list();
		public:
			FrameResources::ptr frame_resources;
			bool owns_frame_resources = false;
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

			void free_tracked_objects();
			virtual ~CommandListBase() = default;

			CommandListType get_type();
		};

		class TransitionCommandList;


		class Transitions : public virtual CommandListBase
		{
			std::vector<HAL::Resource*> used_resources;

			friend class SignatureDataSetter;
			friend class Sendable;
			friend class Eventer;
			friend class ResourceStateManager;
		protected:
			void begin();
			void on_execute();
			std::deque<HAL::UsagePoint> usage_points;
			std::deque<HAL::ResourceUsage> usage_pool;
			size_t pool_used = 0;
			std::set<Resource*> need_check_transitions;
			void create_usage_point(BarrierSync operation, bool end = true);

			// --- Operation batching (phase 3) ---------------------------------
			// Consecutive ops of the same class share ONE usage point (one
			// barrier group) instead of each bracketing itself. open_op is the
			// currently open batch's class (NONE = none open). current_batch_id
			// is the OP-CLASS batch epoch — advanced ONLY by begin_op (op-class
			// change), NOT by intra-batch hazard splits, so a resource touched
			// anywhere in the op-class batch stays detectable across splits. Each
			// resource's per-list state stamps batch_touch_id/state, so the hazard
			// check reads the resource directly (no parallel map): if it was
			// already touched in THIS epoch with a different, non-mergeable state,
			// the batch is split. A different op class closes the batch (begin_op);
			// list end closes it (close_op).
			BarrierSync open_op = BarrierSync::NONE;
			uint current_batch_id = 0;

			void begin_op(BarrierSync op);
			void batch_hazard_check(const HAL::Resource* resource, ResourceState to);
		public:
			// Close the open op-batch. Called at list end (compile_transitions)
			// and by the FrameGraph before it appends cross-pass transitions, so
			// those land after the last op instead of inside its batch.
			void close_op();
		protected:

		public://temporarily to allow transition into read mode
			void transition(const HAL::Resource* resource, ResourceState state, UINT subres = ALL_SUBRESOURCES);
			void transition(const HAL::Resource::ptr& resource, ResourceState state, UINT subres = ALL_SUBRESOURCES);


				  	void compile_transitions();
		public:
			void free_resources();
			 bool transitions_compiled = false;
			UINT transition_count = 0;
			HAL::ResourceUsage* add_usage(const HAL::Resource* resource, UINT subres, ResourceState state, HAL::TransitionType type = HAL::TransitionType::LAST);

			HAL::UsagePoint* get_last_usage_point();

			void use_resource(const HAL::Resource* resource);
		public:

			void alias_begin(HAL::Resource*);
			void alias_end(HAL::Resource*);


#ifdef PRETRANSITIONS_FIX
			std::shared_ptr<TransitionCommandList> fix_pretransitions();
#endif

			void transition_present(const HAL::Resource* resource_ptr);

			void transition(const ResourceInfo& info, BarrierSync operation = BarrierSync::NONE);
			void stop_using(const ResourceInfo& info);
		};



		class GPUTimer :public GPUTimerInterface
		{
		public:

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

		class GPUBlock :public TimedBlock
		{
			Device& device;


		public:
			using ptr = std::shared_ptr<GPUBlock>;
					 GPUTimer gpu_timer;

			GPUBlock(LiteralWStr name, TimedBlock* parent, Device& device);
			void start(Eventer* list);

			void end(Eventer* list);
		};


		class Eventer : public virtual CommandListBase, public TimedRoot
		{
			friend class GPUBlock;
			std::list< GPUBlock::ptr> gpu_timers;

			TimedBlock* current;
			bool started = false;
			virtual  void on_start(Timer* timer) override;
			virtual  void on_end(Timer* timer)override;
		protected:
#ifdef DEV
			Exceptions::stack_trace begin_stack;
#endif
			Device& device;

			void reset();
			void begin(LiteralWStr name);
		public:
			void end();
			Eventer(Device& device);
			Device& get_device() { return device; }
			static thread_local Eventer* thread_current;

			virtual Timer start(LiteralWStr name) override;

			std::shared_ptr<Timer> timer;
			// events
			void start_event(std::wstring_view str);
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


		class CommandList : public Transitions, public Eventer, public Sendable, public SharedObject<CommandList>, public TypedObject<CommandList>
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
				// Operation batching: open (or continue) a batch of this class
				// instead of bracketing every op with its own usage point.
				begin_op(operation);
				if constexpr (compute || graphics)
				{
					if constexpr (Debug::GfxDebug)	setup_debug(&context);
					context.commit_tables(operation, slots);
					if constexpr (graphics) context.validate();
				}
			}
			template<bool compute, bool graphics, class T>
			void post_command(T& context, BarrierSync operation)
			{
				// Leave the batch OPEN — it is closed lazily by the next op of a
				// different class (begin_op) or at list end (close_op). A hazard
				// on a shared resource splits it (see batch_hazard_check).
				if constexpr (Debug::GfxDebug)
					if constexpr (compute || graphics)	print_debug();
			}
		public:
			void end();

			void update_tilings(HAL::update_tiling_info&& info);

			void setup_debug(SignatureDataSetter*);
			void print_debug();
			bool first_debug_log = true;
			StructuredBufferView<Table::DebugStruct> debug_buffer;

			GraphicsContext& get_graphics();
			ComputeContext& get_compute();
			CopyContext& get_copy();

			const std::vector<CommandRecord>& get_debug_records() const;

			void discard(HAL::Resource* resource);

			CommandList(CommandListType, Device&);

			void begin(LiteralWStr name = L"");


			void clear_uav(const Handles::UAV& h, vec4 ClearColor = vec4(0, 0, 0, 0));

		};

		class CopyContext
		{
			friend class CommandList;
			friend class Resource;
					friend class Buffer;

			CommandList& base;
			DelayedCommandList* list;

			CopyContext(CommandList& base);
			CopyContext(const CopyContext&) = delete;
			CopyContext(CopyContext&&) = delete;


			void update_buffer(HAL::Resource* resource, uint64 offset, const char* data, uint64 size);
			void update_buffer(HAL::Resource::ptr resource, uint64 offset, const char* data, uint64 size);
		public:
			// todo: make it better
			std::future<bool> read_buffer(HAL::Resource* resource, uint64 offset, UINT64 size, std::function<void(std::span<std::byte>)>);

		public:
			void copy_resource(HAL::Resource* dest, HAL::Resource* source);
			void copy_resource(const HAL::Resource::ptr& dest, const HAL::Resource::ptr& source);
			void copy_texture(const HAL::Resource::ptr& dest, int, const HAL::Resource::ptr& source, int);
			void copy_texture(const HAL::Resource::ptr& dest, ivec3, const HAL::Resource::ptr& source, ivec3, ivec3);
			void copy_buffer(HAL::Resource* dest, uint64 s_dest, HAL::Resource* source, uint64 s_source, uint64 size);



			//TODO: remove
			void update_texture(HAL::Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);

			void update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);



			std::future<bool> read_texture(const HAL::Resource* resource, UINT sub_resource, std::function<void(std::span<std::byte>, texture_layout)>);

			std::future<bool> read_texture(HAL::Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(std::span<std::byte>, texture_layout)>);
			std::future<bool> read_texture(const HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(std::span<std::byte>, texture_layout)>);
			std::future<bool> read_query(std::shared_ptr<QueryHeap>&, unsigned int offset, unsigned int count, std::function<void(std::span<std::byte>)>);



			template<class T>
			void update(HAL::StructuredBufferView<T>& view, uint64 offset, std::span<typename HAL::StructuredBufferView<T>::UnderlyingType> data)
			{
				update_buffer(view.resource, view.desc.offset + offset * sizeof(HAL::StructuredBufferView<T>::UnderlyingType), reinterpret_cast<const char*>(data.data()), data.size_bytes());

			}


			template<class T>
			std::future<bool> read(HAL::StructuredBufferView<T>& view, uint64 offset, UINT64 count, std::function<void(std::span<T>)> f)
			{
				return read_buffer(view.resource.get(), view.desc.offset + offset * sizeof(HAL::StructuredBufferView<T>::UnderlyingType), count * sizeof(HAL::StructuredBufferView<T>::UnderlyingType),
					[f](std::span<std::byte> memory)
					{
						uint read = uint(memory.size())/sizeof(HAL::StructuredBufferView<T>::UnderlyingType);
						auto data = reinterpret_cast<T*>(memory.data());


						f({ data,read });
					});

			}

				template<class T>
			std::future<bool> read_counter(HAL::StructuredBufferView<T>& view, std::function<void(uint)> f)
			{
				return read_buffer(view.counter_view.resource.get(), view.counter_view.offset , sizeof(unsigned int),
					[f](std::span<std::byte> memory)
					{
						uint64 read = memory.size();
						auto data = reinterpret_cast<unsigned int*>(memory.data());


						f(*data);
					});

			}


		};


		class SignatureDataSetter : public GPUEntityStorageProxy
		{
			struct RowInfo
			{
				bool dirty = false;

				SlotID slot_id;
				Handles::CBV const_buffer;
				std::vector<HAL::ResourceInfo*> resources;
			};
			std::vector<RowInfo> tables;

			friend class CommandList;

			RootSignature::ptr root_sig;
			UsedSlots used_slots;
		protected:
			CommandList& base;
			SignatureDataSetter(CommandList& base);


			virtual void set_const_buffer(UINT i, UINT offset, UINT v) = 0;

			void stop_using(uint id);
			void reset_tables();

			void commit_tables(BarrierSync operation, UsedSlots* slots = nullptr);
			virtual void on_set_signature(const RootSignature::ptr& signature) = 0;

			void set_cb(UINT index, const Handles::CBV& cb, BarrierSync operation);

		public:

			void reset();

			CommandList& get_base();

			void set_signature(const RootSignature::ptr& signature);

			void set_signature(Layouts layout);

			void set_pipeline(std::shared_ptr<PipelineStateBase> pipeline);

			template<class T>
			void set_pipeline(KeyPair<typename T::Keys> k = KeyPair<typename T::Keys>())
			{
				set_pipeline(base.get_device().get_engine_pso_holder().GetPSO<T>(k));
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
				table.dirty = true;

				table.const_buffer = compiled.const_buffer;
				table.resources = compiled.resources;
				for (auto& d : compiled.descriptors)
					get_base().track_object(*d);
			}

		};




		class GraphicsContext;
		struct CompiledRT
		{
			static constexpr SIG_TYPE_COMPILED TYPE = SIG_TYPE_COMPILED::RT;


			HAL::Handles::RTV table_rtv;
			HAL::Handles::DSV table_dsv;

			std::vector<HAL::Format> get_formats() const;
			HAL::Format get_depth_format() const;

		};


		class GraphicsContext : public SignatureDataSetter
		{
			friend class CommandList;

			DelayedCommandList* list;

			GraphicsContext(CommandList& base);
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
			void execute_indirect(IndirectCommand& command_types, UINT max_commands, HAL::Resource* command_buffer, UINT64 command_offset = 0, HAL::Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);

		public:
			std::vector<HAL::Format> get_formats() const;

			template<SIG_TYPES::RT RT>
			CompiledRT set_rtv(const RT& rt, RTOptions options = RTOptions::Default, float depth = 0, uint stencil = 0, vec4 clear_color = vec4(0, 0, 0, 0))
			{
				auto compiled = rt.compile(*this);
				set_rtv(compiled, options, depth, stencil, clear_color);
				return compiled;
			}

			void set_rtv(const CompiledRT& rt, RTOptions options = RTOptions::Default, float depth = 0, uint stencil = 0, vec4 clear_color = vec4(0, 0, 0, 0));

			CommandList& get_base();

			void set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType = HAL::PrimitiveTopologyFeed::LIST, bool adjusted = false, uint controlpoints = 0);

			void set_scissor(sizer_long rect);
			void set_viewport(Viewport viewport);
			void set_viewport(vec4 viewport);
			void set_scissors(sizer_long rect);
			void set_viewports(std::vector<Viewport> viewports);

			void dispatch_mesh(ivec3 v);
			void dispatch_mesh(DispatchMeshArguments args);

			void set_stencil_ref(UINT ref);

			std::vector<Viewport> get_viewports();

			void set_index_buffer(HAL::Views::IndexBuffer view);

			void draw(UINT vertex_count, UINT vertex_offset = 0, UINT instance_count = 1, UINT instance_offset = 0);
			void draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count = 1, UINT instance_offset = 0);



			template<IndirectCommandType T>
			void exec_indirect(HAL::StructuredBufferView<T>& buffer, UINT max_commands, UINT offset = 0)
			{
				execute_indirect(
						base.get_device().get_engine_pso_holder().GetCommand(T::CommandID),
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

			ComputeContext(CommandList& base);
			ComputeContext(const ComputeContext&) = delete;
			ComputeContext(ComputeContext&&) = delete;


			std::shared_ptr<RootSignature> current_compute_root_signature;

			void begin();
			void end();
			void on_execute();



			virtual void set_const_buffer(UINT i, UINT offset, UINT v) override;

			void on_set_signature(const RootSignature::ptr&) override;

			void execute_indirect(IndirectCommand& command_types, UINT max_commands, HAL::Resource* command_buffer, UINT64 command_offset = 0, HAL::Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);

		public:

			CommandList& get_base();

			template<class T>
			void clear(HAL::StructuredBufferView<T>& view, vec4 ClearColor = vec4(0, 0, 0, 0))
			{
				get_base().clear_uav(view.rwRAW, ClearColor);
			}

			template<class T>
			void clear_counter(HAL::StructuredBufferView<T>& view, vec4 ClearColor = vec4(0, 0, 0, 0))
			{
				get_base().clear_uav(view.counter_view.rwRAW, ClearColor);
			}

			void dispatch(int = 1, int = 1, int = 1);
			void dispatch(ivec2, ivec2 = ivec2(8, 8));
			void dispatch(ivec3, ivec3 = ivec3(4, 4, 4));
			void dispatch2(ivec2, ivec2 = ivec2(4, 4));


			void dispatch_graph(ResourceAddress addr);
			void set_program(StateObject* id, ResourceAddress buffer, uint size, bool init);

				 template <class PSO>
				void set_program(ResourceAddress buffer, bool init)
				 {

					auto  work_pso = get_base().device.get_engine_pso_holder().GetPSO<PSO>();

					set_program(work_pso.get(),
						buffer,
						uint(work_pso->buffer_size),
						init);
				 }

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

				base.post_command<true, false>(*this, BarrierSync::COMPUTE_SHADING);
			}


			template<class T>
			void exec_indirect(HAL::StructuredBufferView<T>& buffer, UINT max_commands, UINT offset = 0)
			{
				execute_indirect(
						base.get_device().get_engine_pso_holder().GetCommand(T::CommandID),
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
			CommandListType get_type();
			TransitionCommandList(CommandListType type, Device& device);
			void create_transition_list(FrameResources& frame, const HAL::Barriers& transitions);

			const API::CommandList& get_compiled() const;

			void end() override;
			void on_execute();
		};
	}

	namespace Helpers
	{
		template<class T>
			 auto make_buffer(HAL::Device& device, std::span<T> v)
			{
				HAL::StructuredBufferView<T> buffer(device, v.size());

		auto list = (device.get_upload_list());
			list->get_copy().update(buffer, 0, v);
			list->execute_and_wait();

				return buffer;
			}
	}

}
