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
				// Unqualified: the state type is whatever this object stores per
				// context -- TrackedObjectState for most things, a type derived
				// from it where the object needs more (HAL::Resource keeps its
				// per-list barrier tracking there). Naming the base explicitly
				// would only compile for the former.
				auto& state = obj.get_state(this);
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

			// The list's debug name -- the owning pass's name for FrameGraph
			// lists. Used by diagnostics that need to say WHICH list a barrier
			// belongs to; an index within a group is meaningless across groups.
			LiteralWStr get_name() const { return name; }
		};

		class TransitionCommandList;


		class Transitions : public virtual CommandListBase
		{
			std::vector<HAL::Resource*> used_resources;

			friend class SignatureDataSetter;
			friend class Sendable;
			friend class Eventer;
			friend class CommandListGroup;
		protected:
			void begin();
			void on_execute();

			// This list's work, split into contiguous same-class runs. Appended
			// to by begin_op() only when the operation class actually changes.
			//
			// deque, not vector: the recorder holds CmdListOperation* (see
			// DelayedCommandList::func_barrier) so entries must not move as
			// more operations are appended.
			std::deque<HAL::CmdListOperation> operations;

			// Start (or keep growing) the current operation. Consecutive calls
			// with the same class are a no-op; a different class closes the
			// previous operation and appends a new one, reserving both barrier
			// points in the command stream as it goes.
			void begin_op(BarrierSync op);

			// Close the operation at the back, reserving the point for its
			// barriers_after. Called by begin_op when the class changes, and
			// once at end of recording for the final operation.
			void end_op();

			// Force a new operation of the SAME class. begin_op deliberately
			// merges a run of same-class work into one operation, which is what
			// makes barriers batch -- but two dispatches in that run that both
			// touch the same UAV have to be ordered against each other, and a
			// barrier can only sit between operations. record_usage calls this
			// when it spots that case.
			void split_op();

			// Bumped on every begin_op call, including the ones that merge into
			// the current operation. Stamped onto each recorded usage so
			// record_usage can tell one dispatch's binds from the next's.
			uint op_step = 0;

			// op_step at the moment the operation at the back was appended. When
			// it still equals op_step the operation is empty, and forcing
			// another one would leave a stray operation behind holding two
			// reserved-but-unused barrier points.
			uint op_first_step = 0;

			// Set by break_op, consumed by the next begin_op, which then refuses
			// to merge even though the class matches.
			bool force_new_op = false;

		public:
		protected:
			// Append one use to the operation currently being recorded (the
			// back of `operations`), on this resource's per-list
			// TrackedResourceState.
			void record_usage(const HAL::Resource* resource, const HAL::OperationUsage& usage);

		public://temporarily to allow transition into read mode
			void add_resource_usage(const HAL::Resource* resource, ResourceState state, UINT subres = ALL_SUBRESOURCES);
			void add_resource_usage(const HAL::Resource::ptr& resource, ResourceState state, UINT subres = ALL_SUBRESOURCES);


		public:
			// End the current operation so the next command starts a fresh one,
			// even though it is the same class. Unlike split_op this is a no-op
			// on an operation nothing has recorded into yet, so it is safe to
			// call unconditionally at the head of a command.
			//
			// Used by set_rtv: a run of draws merges into one operation, and one
			// operation gets ONE set of entry barriers, so every resource it
			// touches has to hold a single state throughout. That breaks the
			// moment the render target changes -- a pass that renders to a
			// texture and then samples it records RENDER_TARGET and
			// SHADER_RESOURCE into the same operation, and the later use wins,
			// so the clear/draw that needed RENDER_TARGET runs against
			// SHADER_RESOURCE (#1334). Draws with different render targets can't
			// overlap on the GPU anyway, so there is nothing to lose by not
			// batching across the change.
			void break_op();

			void use_resource(const HAL::Resource* resource);

			// Resources this list touched (populated by use_resource). This is the
			// set CommandListGroup::compile_transitions walks -- a resource shared
			// between lists in the group is diffed against where the previous list
			// actually left it, not against the resting layout.
			const std::vector<HAL::Resource*>& get_used_resources() const { return used_resources; }
		public:

			// Declare what state this list will find `resource` in. Only used
			// when the list holds its group's FIRST touch of the resource --
			// otherwise the group has tracked it directly and knows better.
			// Lets the FrameGraph link a resource across a group boundary, which
			// the group cannot see over.
			void set_entry_state(const HAL::Resource* resource, ResourceState state);

			// Open an operation for work this list cannot introspect -- an
			// external SDK recording straight into the native command list
			// (Streamline/DLSS). Unlike begin_op it never merges into the
			// operation in front of it, so the states declared afterwards land
			// in THIS operation's barriers_before and bracket exactly that work.
			//
			// The caller follows it with add_resource_usage for each resource the
			// external call touches; nothing else can know those.
			void begin_external_op(BarrierSync op);

			// Record a trailing use that leaves `resource` in `state`, in an
			// operation of its own so the barrier lands AFTER the work already
			// recorded rather than merging into its state.
			//
			// This is how the FrameGraph converges a resource at its PRODUCER
			// rather than at whichever consumer happens to run first: several
			// passes reading the same version each need the same "before", and
			// only one of them could ever supply it.
			void transition_to(const HAL::Resource* resource, ResourceState state);

			// transition_to() with the resource's resting state. CommandListGroup
			// deliberately does not rest frame_graph_managed resources (a group
			// is only part of the frame), so the FrameGraph calls this on the
			// last pass that touches one.
			void transition_to_rest(const HAL::Resource* resource);

			// The state this list leaves `resource` in: the state of its last
			// recorded usage, or nullopt if it never touched it. The FrameGraph
			// pairs this with set_entry_state on the consuming list to hand a
			// resource from one pass to the next.
			//
			// Subresource-accurate only when the list used the resource
			// uniformly; a list that touched subresources individually reports
			// its last usage's state, which is why the FrameGraph declares a
			// whole-resource state at the boundaries it links.
			std::optional<ResourceState> get_exit_state(const HAL::Resource* resource) const;

			// The state this list needs `resource` to already be in when it
			// starts -- its first recorded usage. The counterpart of
			// get_exit_state: the FrameGraph converges a producer INTO this so
			// the consuming pass needs no barrier of its own, which is the only
			// hand-off that works when a phase has several passes (none of them
			// ordered against each other).
			std::optional<ResourceState> get_first_use_state(const HAL::Resource* resource) const;

			void alias_begin(HAL::Resource*);
			void alias_end(HAL::Resource*);


			void transition_present(const HAL::Resource* resource_ptr);

			// whole_resource: record the use as ALL_SUBRESOURCES instead of the
			// mip/array range the view names. Set by a table member declared
			// [Barrier = ALL] in its .sig -- see HAL::BoundResource.
			void add_resource_usage(const ResourceInfo& info, BarrierSync operation = BarrierSync::NONE, bool whole_resource = false);
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
					setup_debug(&context);
					context.commit_tables(operation, slots);
					if constexpr (graphics) context.validate();
				}
			}
			template<bool compute, bool graphics, class T>
			void post_command(T& context, BarrierSync operation)
			{
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

			// Move the records out rather than copying them. Used by the
			// FrameGraph debug snapshot, which rewrites what it takes.
			std::vector<CommandRecord> take_debug_records();

			void discard(HAL::Resource* resource);

			CommandList(CommandListType, Device&);

			void begin(LiteralWStr name = L"");

			// Call at record time, right after a compiler.func() callback whose
			// deferred body lets an external SDK rebind its own compute state
			// on this list (see nvidia::DLSS::upscale()). Must NOT be called
			// from inside that callback — see HAL.CommandList.cpp.
			void invalidate_state();


			// DEBUG TOOL -- a full "wait for everything, make everything
			// visible" global barrier at this point in the list.
			//
			// For bisecting a MISSING BARRIER: drop it into a pass that renders
			// wrong, and if the corruption goes away the pass is missing a
			// sync/access barrier that the operation batching should have
			// produced. Bracketing a suspect sub-section narrows it further.
			//
			// What it can and cannot find:
			//   CAN  -- missing execution/cache barriers: two dispatches in one
			//           operation racing on a UAV, a write not made visible to a
			//           later read, a producer not waited on.
			//   CANNOT -- missing LAYOUT transitions. A global barrier carries no
			//           layout by design (that is what makes it safe to insert
			//           anywhere). Layout mistakes surface as debug-layer errors
			//           anyway, so the two tools are complements, not overlaps.
			//
			// Breaks the current operation, so the barrier lands between
			// operations rather than inside one. NOT free -- it serialises the
			// queue at this point. Never leave one in committed code.
			void add_heaviest_barrier();

			// whole_resource: see clear_dsv below. Declares the barrier over the
			// entire resource instead of the subresources this view names, so a
			// caller clearing selected slices/mips of a resource it then uses as
			// a whole does not diverge the group's per-subresource tracking.
			void clear_uav(const Handles::UAV& h, vec4 ClearColor = vec4(0, 0, 0, 0),
			               bool whole_resource = false);

			// Clears a DSV directly (ClearDepthStencilView-equivalent) without
			// binding it as the active render target the way set_rtv's
			// RTOptions::ClearDepth does -- set_rtv's clear path also does a
			// full OM bind, resource-state transitions for every attachment,
			// and render-target-size bookkeeping, all irrelevant when the
			// caller only wants the clear. Mirrors clear_uav's shape.
			// whole_resource: declare the barrier over the ENTIRE resource rather
			// than the subresources this view names. For a caller that clears
			// selected slices of an atlas and then binds the whole thing as a
			// DSV anyway, that is both accurate and far cheaper -- per-slice
			// declarations diverge the group's per-subresource tracking, and the
			// next whole-resource use then has to reconcile every subresource
			// individually.
			void clear_dsv(const Handles::DSV& h, bool clear_depth = true, bool clear_stencil = false, float depth = 0, UINT8 stencil = 0,
			               bool whole_resource = false);

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

				// Whether this table's root argument is currently LIVE on the
				// command list -- i.e. commit_tables has actually sent it and no
				// root signature change has invalidated it since.
				//
				// Distinct from `dirty`, which only means "set() recorded new
				// data that has not been committed yet". A table can be
				// committed (dirty=false) and still not live, because changing
				// the root signature discards every root argument -- and since
				// commit_tables only re-sends dirty tables, the binding is then
				// gone from the GPU with nothing to restore it. That is how
				// DenoiserHistoryFix ended up reading FrameInfo from descriptor
				// 0 (GBV #939).
				bool bound = false;

				SlotID slot_id;
				Handles::CBV const_buffer;
				std::vector<HAL::BoundResource> resources;
			};
			std::vector<RowInfo> tables;

			friend class CommandList;

			RootSignature::ptr root_sig;
			UsedSlots used_slots;
		protected:
			CommandList& base;
			SignatureDataSetter(CommandList& base);


			virtual void set_const_buffer(UINT i, UINT offset, UINT v) = 0;

			void reset_tables();

			void commit_tables(BarrierSync operation, UsedSlots* slots = nullptr);
			virtual void on_set_signature(const RootSignature::ptr& signature) = 0;

			void set_cb(UINT index, const Handles::CBV& cb, BarrierSync operation);

		public:

			void reset();

			CommandList& get_base();

			void set_signature(const RootSignature::ptr& signature);

			void set_signature(Layouts layout);

			// Forces the next set_signature() call to reissue. See CommandList::invalidate_state().
			void invalidate_signature() { root_sig = nullptr; }

			// Forces commit_tables() to re-bind every already-set CBV. Skips
			// never-set slots — set_cb() has no validity check.
			void invalidate_tables() { for (auto& t : tables) if (t.const_buffer) t.dirty = true; }

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
			void set_scissors(std::vector<sizer_long> rects);
			void set_viewports(std::vector<Viewport> viewports);
			// Clears only the given rects of the currently-bound DSV (from the
			// last set_rtv call) -- for a shared/persistent depth target where
			// other regions must keep their prior-frame content.
			void clear_depth_rects(std::vector<sizer_long> rects, float depth);

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
			void clear(HAL::StructuredBufferView<T>& view, vec4 ClearColor = vec4(0, 0, 0, 0),
			           bool whole_resource = false)
			{
				get_base().clear_uav(view.rwRAW, ClearColor, whole_resource);
			}

			template<class T>
			void clear_counter(HAL::StructuredBufferView<T>& view, vec4 ClearColor = vec4(0, 0, 0, 0),
			                   bool whole_resource = false)
			{
				get_base().clear_uav(view.counter_view.rwRAW, ClearColor, whole_resource);
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

				base.add_resource_usage(hit_buffer.resource, { BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED });
				base.add_resource_usage(miss_buffer.resource, { BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED });
				base.add_resource_usage(raygen_buffer.resource, { BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED });

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


		// A set of command lists that will be submitted together, in this order,
		// as one ExecuteCommandLists batch. Everything reaches a queue through a
		// group -- there is no way to submit a bare array of lists -- because
		// barriers can only be computed correctly with the whole batch in view.
		//
		// Barriers are computed ACROSS the group, not per list: a resource is
		// entered from its resting layout once, at the group's first use of it,
		// and returned there once, after the group's last use. Between those,
		// lists hand the resource to each other directly, so a resource read (or
		// written) by several consecutive lists no longer bounces through the
		// resting layout at every list boundary.
		class CommandListGroup
		{
			std::vector<CommandList::ptr> lists;

			// One resource this group touches, plus what the group must do
			// about its first-ever use. Decided by plan_resources() on the
			// submitting thread, in submission order, so that
			// compile_transitions() itself reads only group-local data and
			// several groups can compile concurrently.
			struct PlannedResource
			{
				Resource* resource = nullptr;

				// This group is the first ever to touch the resource, so it
				// enters from the creation (undefined) layout with SyncBefore
				// NONE. Otherwise something has already touched it and NONE is
				// illegal (#1417).
				bool from_undefined = false;

				// This group holds the resource's first-ever WRITE and therefore
				// owns its initializing discard (#1422).
				bool owns_discard = false;
			};

			// Every resource any list in the group touched, deduplicated, in
			// first-seen order. Built ONCE by plan_resources and then just
			// walked by compile_transitions -- which used to rebuild the same
			// list through a std::set (a tree-node allocation per resource) and
			// then pay a hash lookup per resource to find its plan.
			std::vector<PlannedResource> planned;

			// Whether plan_resources() has run for this group. Not derivable
			// from `planned` being non-empty: a group whose lists touched no
			// resources at all plans to an empty list, which is legitimate.
			bool planned_built = false;

		public:
			void add(const CommandList::ptr& list) { lists.emplace_back(list); }

			bool empty() const { return lists.empty(); }
			size_t size() const { return lists.size(); }
			void clear() { lists.clear(); }

			const std::vector<CommandList::ptr>& get_lists() const { return lists; }

			// Compute every barrier for the group and fill the groups the
			// reserved points in each list's command stream refer to. Runs after
			// recording and before the lists are compiled -- compile() consumes
			// the barrier groups, so filling them afterwards would emit nothing.
			//
			// Must run exactly once per list. A list processed by two groups (or
			// by a group and again on its own) gets two independently computed
			// barrier sets spliced into one command stream.
			// Resolve the first-use questions for every resource this group
			// touches. MUST run on the submitting thread, in submission order,
			// BEFORE compile_transitions -- it is the only step that reads or
			// writes the per-resource `virgin` / `initialized` flags, and "which
			// group owns the first write" is meaningless out of order.
			//
			// Splitting it out is what makes compile_transitions pure: after
			// this, a group's barrier computation touches nothing shared, so
			// groups can be compiled in parallel.
			void plan_resources();

			void compile_transitions();

			// Replay every list into its API command list. Must follow
			// compile_transitions -- this is what consumes the barrier groups,
			// so compiling first would emit them empty. Lists compile
			// independently, so this fans out across the thread pool and joins
			// before returning.
			void compile();
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
