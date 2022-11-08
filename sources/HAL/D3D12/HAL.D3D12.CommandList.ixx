export module HAL:API.CommandList;
import Core;
import :Types;


import :Private.CommandListCompiler;
import :ResourceStates;
import :Resource;
import :DescriptorHeap;
import :Fence;
import :FrameManager;
import :PipelineState;
import :API.IndirectCommand;

export namespace HAL {

	namespace API
	{
//		class GPUBuffer;
//
//		class QueryHeap
//		{
//
//			ComPtr<ID3D12QueryHeap> heap;
//		public:
//			QueryHeap(UINT max_count, D3D12_QUERY_HEAP_TYPE type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP);
//
//			ComPtr<ID3D12QueryHeap> get_native();
//
//			using ptr = std::shared_ptr<QueryHeap>;
//		};
//
//		class CommandListBase : public StateContext
//		{
//		protected:
//
//			CommandListType type;
//
//			std::vector<std::function<void()>> on_execute_funcs;
//
//			HAL::Private::CommandListCompilerDelayed compiler;
//
//			std::list<TrackedObject::ptr> tracked_resources;
//
//			HAL::Private::CommandListCompilerDelayed* get_native_list()
//			{
//				return &compiler;
//			}
//		public:
//
//			template<TrackableClass T>
//			void track_object(T& obj)
//			{
//				auto& state = obj.ObjectState<TrackedObjectState>::get_state(this);
//				if (!state.used)
//				{
//					state.used = true;
//					tracked_resources.emplace_back(obj.get_tracked());
//				}
//			}
//
//			void free_tracked_objects()
//			{
//				tracked_resources.clear();
//			}
//			virtual ~CommandListBase() = default;
//
//			CommandListType get_type()
//			{
//				return type;
//			}
//		};
//
//		class TransitionCommandList;
//
//
//		class Transitions : public virtual CommandListBase
//		{
//			std::list<HAL::Resource*> used_resources;
//
//			std::shared_ptr<TransitionCommandList> transition_list;
//
//			friend class SignatureDataSetter;
//			friend class Sendable;
//			friend class Eventer;
//			friend class ResourceStateManager;
//		protected:
//			void begin();
//			void on_execute();
//			std::list<HAL::TransitionPoint> transition_points;
//
//			void create_transition_point(bool end = true);
//
//			void make_split_barriers();
//
//			void transition(const HAL::Resource* resource, ResourceState state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
//			void transition(const HAL::Resource::ptr& resource, ResourceState state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
//
//		public:
//			void free_resources();
//
//			UINT transition_count = 0;
//			//	TransitionPoint* start_transition;
//			HAL::Transition* create_transition(const HAL::Resource* resource, UINT subres, ResourceState state, HAL::TransitionType type = HAL::TransitionType::LAST)
//			{
//				HAL::TransitionPoint* point = nullptr;
//
//				if (type == HAL::TransitionType::LAST) point = &transition_points.back();
//				if (type == HAL::TransitionType::ZERO) point = &transition_points.front();
//
//
//				//if (type == TransitionType::LAST) 			assert(!point->start);
//				HAL::Transition& transition = point->transitions.emplace_back();
//
//				transition.resource = const_cast<HAL::Resource*>(resource);
//				transition.subres = subres;
//				transition.wanted_state = state;
//
//				transition.point = point;
//				return &transition;
//			}
//
//			void create_uav_transition(const HAL::Resource* resource)
//			{
//				auto& point = transition_points.back();
//				point.uav_transitions.emplace_back(const_cast<HAL::Resource*>(resource));
//			}
//
//			void create_aliasing_transition(const HAL::Resource* resource)
//			{
//				auto& point = transition_points.back();
//				point.aliasing.emplace_back(const_cast<HAL::Resource*>(resource));
//			}
//
//			HAL::TransitionPoint* get_last_transition_point()
//			{
//				return &transition_points.back();
//			}
//
//			void use_resource(const HAL::Resource* resource);
//		public:
//			void prepare_transitions(Transitions* to, bool all);
//
//
//			void merge_transition(Transitions* to, HAL::Resource* res);
//			void transition_uav(HAL::Resource* resource);
//			void transition(HAL::Resource* from, HAL::Resource* to);
//			std::shared_ptr<TransitionCommandList> fix_pretransitions();
//
//			void transition_present(const HAL::Resource* resource_ptr)
//			{
//
//				create_transition_point();
//
//				transition(resource_ptr, ResourceState::PRESENT, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
//
//				create_transition_point(false);
//			}
//
//
//			void transition(const ResourceInfo* info)
//			{
//				if (!info || !info->resource_ptr) return;
//
//				auto target_state = ResourceState::COMMON;
//
//
//				if (std::holds_alternative<HAL::Views::ShaderResource>(info->view))
//				{
//					if (type == CommandListType::DIRECT)
//					{
//						target_state = ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NON_PIXEL_SHADER_RESOURCE;
//					}
//
//					if (type == CommandListType::COMPUTE)
//					{
//						target_state = ResourceState::NON_PIXEL_SHADER_RESOURCE;
//					}
//
//				}
//				else 	if (std::holds_alternative<HAL::Views::UnorderedAccess>(info->view))
//				{
//					target_state = ResourceState::UNORDERED_ACCESS;
//				}
//				else 	if (std::holds_alternative<HAL::Views::RenderTarget>(info->view))
//				{
//					target_state = ResourceState::RENDER_TARGET;
//				}
//				else 	if (std::holds_alternative<HAL::Views::DepthStencil>(info->view))
//				{
//					target_state = ResourceState::DEPTH_WRITE;
//
//				}
//				else assert(false);
//
//				info->for_each_subres([&](const HAL::Resource* resource, UINT subres)
//					{
//						transition(resource, target_state, subres);
//					});
//			}
//
//			void stop_using(const ResourceInfo* info)
//			{
//				if (!info || !info->resource_ptr) return;
//
//
//				info->for_each_subres([&](const HAL::Resource* resource, UINT subres)
//					{
//						const_cast<HAL::Resource*>(resource)->get_state_manager().stop_using(this, subres);
//					});
//			}
//		};
//
//
//
//		class GPUTimer
//		{
//		public://		friend class GPUTimeManager;
//
//			int id;
//
//
//		public:
//			CommandListType queue_type;
//			GPUTimer();
//
//			virtual ~GPUTimer();
//
//			void start(Eventer* list);
//
//			void end(Eventer* list);
//
//			float get_time();
//
//
//			double get_start();
//
//			double get_end();
//		};
//
//
//		class GPUCounter
//		{
//
//		public:
//			GPUTimer timer;
//			bool enabled = true;
//
//			std::chrono::time_point<std::chrono::high_resolution_clock>  start_time;
//			std::chrono::time_point<std::chrono::high_resolution_clock>  end_time;
//
//		};
//
//		class GPUBlock :public TimedBlock
//		{
//		public:
//			using TimedBlock::TimedBlock;
//
//			GPUCounter gpu_counter;
//		};
//
//
//		class Eventer : public virtual CommandListBase, public TimedRoot
//		{
//
//			std::list<std::wstring> names;
//			TimedBlock* current;
//
//			virtual  void on_start(Timer* timer) override;
//			virtual  void on_end(Timer* timer)override;
//		protected:
//#ifdef DEV
//			Exceptions::stack_trace begin_stack;
//#endif
//			std::string name;
//			void reset();
//			void begin(std::string name, Timer* t = nullptr);
//		public:
//			static thread_local Eventer* thread_current;
//
//			virtual Timer start(std::wstring_view name)override;
//
//			std::shared_ptr<Timer> timer;
//			// events
//			void start_event(std::wstring str);
//			void end_event();
//
//			void set_marker(const wchar_t* label);
//
//
//			// timers
//			void insert_time(QueryHeap& pQueryHeap, uint32_t QueryIdx);
//			void resolve_times(QueryHeap& pQueryHeap, uint32_t NumQueries, std::function<void(std::span<UINT64>)>);
//
//		};
//
//
//
//		class Sendable : public virtual CommandListBase
//		{
//
//			friend class Queue;
//			std::vector<std::function<void(FenceWaiter)>> on_fence;
//
//
//			std::promise<FenceWaiter> execute_fence;
//			std::shared_future<FenceWaiter> execute_fence_result;
//
//			void on_send(FenceWaiter fence)
//			{
//
//
//
//				execute_fence.set_value(fence);
//
//				for (auto&& e : on_fence)
//					e(fence);
//
//				on_fence.clear();
//			}
//		public:
//			HAL::Private::CommandListCompiled compiled;
//
//
//
//
//		public:
//			void compile();
//			void when_send(std::function<void(FenceWaiter)> e)
//			{
//				on_fence.emplace_back(e);
//			}
//
//			std::shared_future<FenceWaiter> get_execution_fence()
//			{
//				return execute_fence_result;
//			}
//
//			std::shared_future<FenceWaiter> execute(std::function<void()> f = nullptr);
//			void execute_and_wait(std::function<void()> f = nullptr);
//
//			void on_done(std::function<void()> f);
//
//		};
//
//		class SignatureDataSetter;
//
//
//		class CommandList :  public Readbacker<Thread::Free>, public Transitions, public Eventer, public Sendable, public GPUCompiledManager<Thread::Free>, public SharedObject<CommandList>
//		{
//
//
//		public:
//			using ptr = std::shared_ptr<CommandList>;
//		protected:
//			CommandList() = default;
//			friend class Queue;
//			friend class FrameResourceManager;
//			friend class CopyContext;
//			friend class GraphicsContext;
//			friend class ComputeContext;
//			friend class SignatureDataSetter;
//
//			// TODO: make references?
//
//			virtual void on_execute();
//
//			std::unique_ptr<GraphicsContext> graphics;
//			std::unique_ptr<ComputeContext> compute;
//			std::unique_ptr<CopyContext> copy;
//			PipelineStateBase* current_pipeline;
//			PipelineStateBase* first_pipeline;
//
//			std::list<HAL::update_tiling_info> tile_updates;
//
//
//
//			void set_pipeline_internal(PipelineStateBase* pipeline);
//
//		public:
//
//			void update_tilings(HAL::update_tiling_info&& info)
//			{
//				tile_updates.emplace_back(std::move(info));
//
//				track_object(*(info.resource));
//			}
//
//			FrameResources::ptr frame_resources;
//			void setup_debug(SignatureDataSetter*);
//			void print_debug();
//			bool first_debug_log = true;
//			std::shared_ptr<GPUBuffer> debug_buffer;
//
//			GraphicsContext& get_graphics();
//			ComputeContext& get_compute();
//			CopyContext& get_copy();
//
//
//			FrameResources* get_manager()
//			{
//				return frame_resources.get();
//			}
//
//			CommandList(CommandListType);
//
//			void begin(std::string name = "", Timer* t = nullptr);
//			void end();
//
//
//			void clear_uav(const Handle& h, vec4 ClearColor = vec4(0, 0, 0, 0))
//			{
//				create_transition_point();
//				transition(h.get_resource_info());
//				auto dx_resource = h.get_resource_info()->resource_ptr->native_resource.Get();
//
//
//				if (BuildOptions::Debug)
//				{
//					auto uav = std::get<HAL::Views::UnorderedAccess>(h.get_resource_info()->view);
//					if (h.get_resource_info()->resource_ptr->get_desc().is_buffer())
//					{
//						auto buffer = std::get<HAL::Views::UnorderedAccess::Buffer>(uav.View);
//						assert(buffer.StructureByteStride == 0);
//					}
//				}
//
//				get_native_list()->ClearUnorderedAccessViewFloat(h.get_gpu(), h.get_cpu(), dx_resource, reinterpret_cast<FLOAT*>(ClearColor.data()), 0, nullptr);
//				create_transition_point(false);
//			}
//
//
//			void clear_rtv(const Handle& h, vec4 ClearColor = vec4(0, 0, 0, 0))
//			{
//				create_transition_point();
//				transition(h.get_resource_info());
//				get_native_list()->ClearRenderTargetView(h.get_cpu(), ClearColor.data(), 0, nullptr);
//				create_transition_point(false);
//			}
//
//
//
//
//			void clear_stencil(Handle dsv, UINT8 stencil = 0)
//			{
//				create_transition_point();
//				transition(dsv.get_resource_info());
//
//				get_native_list()->ClearDepthStencilView(dsv.get_cpu(), D3D12_CLEAR_FLAG_STENCIL, 0, stencil, 0, nullptr);
//				create_transition_point(false);
//			}
//
//			void clear_depth(Handle dsv, float depth = 0)
//			{
//				create_transition_point();
//				transition(dsv.get_resource_info());
//
//				get_native_list()->ClearDepthStencilView(dsv.get_cpu(), D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
//				create_transition_point(false);
//			}
//
//
//		};
//
//		class CopyContext
//		{
//			friend class CommandList;
//
//			CommandList& base;
//			HAL::Private::CommandListCompilerDelayed* list;
//
//			CopyContext(CommandList& base) :base(base), list(base.get_native_list()) {}
//			CopyContext(const CopyContext&) = delete;
//			CopyContext(CopyContext&&) = delete;
//		public:
//			void copy_resource(HAL::Resource* dest, HAL::Resource* source);
//			void copy_resource(const HAL::Resource::ptr& dest, const HAL::Resource::ptr& source);
//			void copy_texture(const HAL::Resource::ptr& dest, int, const HAL::Resource::ptr& source, int);
//			void copy_texture(const HAL::Resource::ptr& dest, ivec3, const HAL::Resource::ptr& source, ivec3, ivec3);
//			void copy_buffer(HAL::Resource* dest, int s_dest, HAL::Resource* source, int s_source, int size);
//
//
//			//TODO: remove
//			void update_buffer(HAL::Resource::ptr resource, UINT offset, const char* data, UINT size);
//			void update_texture(HAL::Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);
//			void update_buffer(HAL::Resource* resource, UINT offset, const char* data, UINT size);
//			void update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);
//
//
//
//			std::future<bool> read_texture(HAL::Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)>);
//			std::future<bool> read_texture(const HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)>);
//			std::future<bool> read_buffer(HAL::Resource* resource, unsigned int offset, UINT64 size, std::function<void(const char*, UINT64)>);
//			std::future<bool> read_query(std::shared_ptr<QueryHeap>&, unsigned int offset, unsigned int count, std::function<void(const char*, UINT64)>);
//		};
//
//
//		class SignatureDataSetter
//		{
//			struct RowInfo
//			{
//				HandleType type;
//				bool dirty = false;
//				HandleTableLight table;
//			};
//			std::vector<RowInfo> tables;
//
//			friend class CommandList;
//
//			RootSignature::ptr root_sig;
//			//		UsedSlots used_slots;
//		protected:
//			CommandList& base;
//			SignatureDataSetter(CommandList& base) :base(base) {
//				tables.resize(32); // !!!!!!!!!!!
//			}
//
//			virtual void set(UINT, const HandleTableLight&) = 0;
//			virtual void set_const_buffer(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS&) = 0;
//
//
//			void reset_tables()
//			{
//				for (auto& row : tables)
//				{
//					row.dirty = false;
//				}
//			}
//
//			void commit_tables()
//			{
//
//				for (auto& row : tables)
//				{
//					if (!row.dirty) continue;
//
//					auto& table = row.table;
//					auto type = row.type;
//					for (UINT i = 0; i < (UINT)table.get_count(); ++i)
//					{
//						const auto& h = table[i];
//						get_base().transition(h.get_resource_info());
//					}
//
//					row.dirty = false;
//				}
//			}
//
//		public:
//
//			void reset()
//			{
//				root_sig = nullptr;
//			}
//			CommandList& get_base() {
//				return base;
//			}
//
//			void set_signature(const RootSignature::ptr& signature)
//			{
//				if (root_sig == signature) return;
//
//				root_sig = signature;
//
//				auto& desc = signature->get_desc();
//
//				on_set_signature(signature);
//			}
//
//			virtual void on_set_signature(const RootSignature::ptr& signature) = 0;
//
//			void set_pipeline(std::shared_ptr<PipelineStateBase> pipeline);
//
//			template<HandleType type>
//			void set_table(UINT index, const HandleTableLight& table)
//			{
//				assert(table.valid());
//
//				auto& row = tables[index];
//
//				row.type = type;
//				row.table = table;
//				row.dirty = true;
//				set(index, table);
//			}
//
//
//			void set_cb(UINT index, const HAL::ResourceAddress& address)
//			{
//				if (address.resource)
//				{
//					get_base().track_object(*address.resource);
//					get_base().transition(address.resource, ResourceState::VERTEX_AND_CONSTANT_BUFFER);
//				}
//				set_const_buffer(index, to_native(address));
//			}
//
//			template<class Compiled>
//			void set_slot(Compiled& compiled)
//			{
//				compiled.set_tables(*this);
//			}
//
//			template<class T>
//			std::unique_ptr<T> wrap()
//			{
//				auto res = std::make_unique<T>();
//				res->begin(this);
//				return res;
//			}
//		};
//
//		class GraphicsContext : public SignatureDataSetter
//		{
//			friend class CommandList;
//			HAL::Private::CommandListCompilerDelayed* list;
//
//			GraphicsContext(CommandList& base) :SignatureDataSetter(base), list(base.get_native_list()) {
//			}
//			GraphicsContext(const GraphicsContext&) = delete;
//			GraphicsContext(GraphicsContext&&) = delete;
//
//			bool valid_scissor = false;
//			std::vector<Viewport> viewports;
//			D3D_PRIMITIVE_TOPOLOGY  native_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
//
//
//			void begin();
//			void end();
//			void on_execute();
//
//			void set_const_buffer(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&)override;
//			void set(UINT, const HandleTableLight&)override;
//			HAL::Views::IndexBuffer index;
//		public:
//
//			CommandList& get_base()
//			{
//				return base;
//			}
//
//			void set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType = HAL::PrimitiveTopologyFeed::LIST, bool adjusted = false, uint controlpoints = 0)
//			{
//				auto native = to_native(topology, feedType, adjusted, controlpoints);
//				if (this->native_topology != native)
//					list->IASetPrimitiveTopology(native);
//
//				this->native_topology = native;
//			}
//
//
//			void on_set_signature(const RootSignature::ptr&) override;
//			//	void set_pipeline(std::shared_ptr<PipelineState>);
//
//
//
//
//			void set_heaps(DescriptorHeap::ptr& a, DescriptorHeap::ptr& b);
//
//			void set_scissor(sizer_long rect);
//			void set_viewport(Viewport viewport);
//			void set_viewport(vec4 viewport);
//			void set_scissors(sizer_long rect);
//			void set_viewports(std::vector<Viewport> viewports);
//
//
//			void set_rtv(std::initializer_list<Handle> rt, Handle h);
//			void set_rtv(const HandleTable&, Handle);
//			void set_rtv(int c, Handle rt, Handle h);
//			void set_rtv(const HandleTableLight&, Handle);
//
//			void draw(D3D12_DRAW_INDEXED_ARGUMENTS args)
//			{
//				draw_indexed(args.IndexCountPerInstance, args.StartIndexLocation, args.BaseVertexLocation, args.InstanceCount, args.StartInstanceLocation);
//			}
//
//			void dispatch_mesh(ivec3 v);
//			void dispatch_mesh(D3D12_DISPATCH_MESH_ARGUMENTS args);
//
//
//			HandleTableLight place_rtv(UINT count)
//			{
//				return get_base().get_cpu_heap(DescriptorHeapType::RTV).place(count);
//			}
//
//			HandleTableLight place_dsv(UINT count)
//			{
//				return get_base().get_cpu_heap(DescriptorHeapType::DSV).place(count);
//			}
//
//			HandleTableLight place_rtv(std::initializer_list<Handle> list)
//			{
//				return get_base().get_cpu_heap(DescriptorHeapType::RTV).place(list);
//			}
//
//			HandleTableLight place_dsv(std::initializer_list<Handle> list)
//			{
//				return get_base().get_cpu_heap(DescriptorHeapType::DSV).place(list);
//			}
//
//
//			void set_stencil_ref(UINT ref)
//			{
//				list->OMSetStencilRef(ref);
//			}
//
//			std::vector<Viewport> get_viewports()
//			{
//				return viewports;
//			}
//
//			void set_index_buffer(HAL::Views::IndexBuffer view)
//			{
//				index = view;
//
//			}
//
//
//
//			void draw(UINT vertex_count, UINT vertex_offset = 0, UINT instance_count = 1, UINT instance_offset = 0);
//			void draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count = 1, UINT instance_offset = 0);
//			void execute_indirect(IndirectCommand& command_types, UINT max_commands, HAL::Resource* command_buffer, UINT64 command_offset = 0, HAL::Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);
//
//		};
//
//
//
//
//
//		class ComputeContext : public SignatureDataSetter
//		{
//			friend class CommandList;
//
//
//			HAL::Private::CommandListCompilerDelayed* list;
//
//			ComputeContext(CommandList& base) :SignatureDataSetter(base), list(base.get_native_list()) {}
//			ComputeContext(const ComputeContext&) = delete;
//			ComputeContext(ComputeContext&&) = delete;
//
//
//			//	std::shared_ptr<ComputePipelineState> current_compute_pipeline;
//			std::shared_ptr<RootSignature> current_compute_root_signature;
//
//			void begin();
//			void end();
//			void on_execute();
//
//
//
//
//			void set(UINT, const HandleTableLight&)override;
//
//
//
//
//
//
//			virtual void set_const_buffer(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&) override;
//
//
//		public:
//
//			CommandList& get_base()
//			{
//				return base;
//			}
//
//
//			void on_set_signature(const RootSignature::ptr&) override;
//			//	void set_pipeline(std::shared_ptr<ComputePipelineState>);
//
//
//			void dispach(int = 1, int = 1, int = 1);
//			void dispach(ivec2, ivec2 = ivec2(8, 8));
//			void dispach(ivec3, ivec3 = ivec3(4, 4, 4));
//
//
//
//
//			void execute_indirect(IndirectCommand& command_types, UINT max_commands, HAL::Resource* command_buffer, UINT64 command_offset = 0, HAL::Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);
//
//
//
//			void build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom);
//			void build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top);
//
//			template<class Hit, class Miss, class Raygen>
//			void dispatch_rays(ivec2 size, HAL::ResourceAddress hit_buffer, UINT hit_count, HAL::ResourceAddress miss_buffer, UINT miss_count, HAL::ResourceAddress raygen_buffer)
//			{
//
//				base.setup_debug(this);
//				D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
//				// Since each shader table has only one shader record, the stride is same as the size.
//				dispatchDesc.HitGroupTable.StartAddress = to_native(hit_buffer);
//				dispatchDesc.HitGroupTable.SizeInBytes = sizeof(Hit) * hit_count;
//				dispatchDesc.HitGroupTable.StrideInBytes = sizeof(Hit);
//
//				dispatchDesc.MissShaderTable.StartAddress = to_native(miss_buffer);
//				dispatchDesc.MissShaderTable.SizeInBytes = sizeof(Miss) * miss_count;
//				dispatchDesc.MissShaderTable.StrideInBytes = sizeof(Miss);
//
//				dispatchDesc.RayGenerationShaderRecord.StartAddress = to_native(raygen_buffer);
//				dispatchDesc.RayGenerationShaderRecord.SizeInBytes = sizeof(Raygen);
//				dispatchDesc.Width = size.x;
//				dispatchDesc.Height = size.y;
//				dispatchDesc.Depth = 1;
//
//				base.create_transition_point();
//
//				base.transition(hit_buffer.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
//				base.transition(miss_buffer.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
//				base.transition(raygen_buffer.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
//
//				commit_tables();
//				list->DispatchRays(&dispatchDesc);
//
//				base.create_transition_point(false);
//
//				get_base().print_debug();
//			}
//
//		};


		class TransitionCommandList
		{
		protected:
			ComPtr<ID3D12GraphicsCommandList4> m_commandList;
			ComPtr<ID3D12CommandAllocator> m_commandAllocator;
			
		public:
			ComPtr<ID3D12GraphicsCommandList4> get_native();
		};
	}



}