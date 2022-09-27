module;
export module Graphics:CommandList;

import Utils;
import :CommandListCompiler;
import StateContext;
import Profiling;
import Threading;
import Exceptions;
import :RootSignature;
import :Device;
import :Fence;
import :IndirectCommand;
import :Resource;
import :ResourceViews;
import :Descriptors;
//import :GPUTimer;

import Math;
import :Enums;
import Data;
import Singleton;

//import Buffer;
import stl.core;
import stl.memory;

import HAL;

import d3d12;

import :Definitions;
//using namespace HAL;

export{
	//enum class Layouts;

	namespace Graphics
	{
		class GraphicsContext;

#define DEFAULT_ALIGN 256




		struct UploadInfo
		{
			std::shared_ptr<Resource> resource;
			UINT64 offset;
			UINT64 size;
			D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address();
			ResourceAddress get_resource_address();
			Handle create_cbv(CommandList& list);

			std::byte* get_cpu_data() const;
		};

		template<class LockPolicy>
		class Uploader
		{
			ResourceHeapAllocator<LockPolicy> allocator;
			friend class BufferCache;
			std::list<ResourceHandle> handles;
			UINT heap_size = 0x200000;
		protected:

			template<class T>
			size_t size_of(std::span<T>& elem)
			{
				return sizeof(T) * elem.size();
			}
			template<class T>
			size_t size_of(std::vector<T>& elem)
			{
				return sizeof(T) * elem.size();
			}

			template<class T>
			size_t size_of(my_unique_vector<T>& elem)
			{
				return sizeof(T) * elem.size();
			}

			template<class T>
			size_t size_of(T& elem)
			{
				return sizeof(T);
			}

			typename LockPolicy::mutex m;

		public:


			//	UploadInfo place_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN);


			void reset()
			{
				typename LockPolicy::guard g(m);

				for (auto& h : handles)
					h.Free();
				handles.clear();


			}


			UploadInfo place_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN)
			{
				const auto AlignedSize = static_cast<UINT>(Math::roundUp(uploadBufferSize, alignment));


				auto handle = allocator.alloc(AlignedSize, alignment, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, HeapType::UPLOAD);

				//	auto handle = BufferCache::get().get_upload(AlignedSize, alignment);
				typename LockPolicy::guard g(m);

				handles.emplace_back(handle);
				UploadInfo info;
				info.resource = handle.get_heap()->cpu_buffer;
				info.offset = handle.get_offset();
				info.size = uploadBufferSize;
				return info;
			}



			template<class ...Args>
			UploadInfo place_raw(Args... args)
			{
				size_t size = (0 + ... + size_of(args));



				auto info = place_data(size);


				//	memcpy(info.resource->get_data() + info.offset, data, size);
				size_t start = 0;
				{
					(write(info, start, std::forward<Args>(args)), ...);
				}
				return info;
			}

			void write(UploadInfo& info, size_t offset, void* data, size_t size)
			{
				if (size > 0) memcpy(info.resource->buffer_data + info.offset + offset, data, size);
			}





			//void write(UploadInfo& info, size_t offset, void* data, size_t size);

			template<class T>
			void write(UploadInfo& info, const std::span<T>& arg)
			{
				write(info, 0, (void*)arg.data(), arg.size() * sizeof(T));
			}
			template<class T>
			void write(UploadInfo& info, const std::vector<T>& arg)
			{
				write(info, 0, (void*)arg.data(), arg.size() * sizeof(T));
			}

			template<class T>
			void write(UploadInfo& info, const my_unique_vector<T>& arg)
			{
				write(info, 0, (void*)arg.data(), arg.size() * sizeof(T));
			}

			template<class T>
			void write(UploadInfo& info, size_t& offset, const std::vector<T>& arg)
			{
				write(info, offset, (void*)arg.data(), arg.size() * sizeof(T));
				offset += arg.size() * sizeof(T);
			}


			template<class T>
			void write(UploadInfo& info, size_t& offset, const my_unique_vector<T>& arg)
			{
				write(info, offset, (void*)arg.data(), arg.size() * sizeof(T));
				offset += arg.size() * sizeof(T);

			}
			template<class T>
			void write(UploadInfo& info, size_t& offset, const T& arg)
			{
				write(info, offset, (void*)&arg, sizeof(T));
				offset += sizeof(T);

				//Log::get() << "write " <<  Log::endl;
			}
		};


		class Readbacker
		{
			ResourceHeapAllocator<Thread::Free> allocator;
			friend class BufferCache;
			std::vector<ResourceHandle> handles;
		protected:
			void reset();
		public:

			struct ReadBackInfo
			{
				std::shared_ptr<Resource> resource;
				UINT64 offset;
				UINT64 size;

				std::byte* get_cpu_data() const;
			};

			ReadBackInfo read_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN);

		};


		template<class LockPolicy = Thread::Free>
		class GPUCompiledManager : public Uploader<LockPolicy>
		{
			enum_array<HAL::DescriptorHeapType, typename DynamicDescriptor<LockPolicy>::ptr> cpu_heaps;
			enum_array<HAL::DescriptorHeapType, typename DynamicDescriptor<LockPolicy>::ptr> gpu_heaps;

		public:

			DynamicDescriptor<LockPolicy>& get_cpu_heap(HAL::DescriptorHeapType type)
			{
				assert(cpu_heaps[type]);
				return *cpu_heaps[type];
			}

			DynamicDescriptor<LockPolicy>& get_gpu_heap(HAL::DescriptorHeapType type)
			{
				assert(cpu_heaps[type]);
				return *gpu_heaps[type];
			}

			GPUCompiledManager()
			{
				gpu_heaps[DescriptorHeapType::CBV_SRV_UAV] = std::make_shared<DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::SHADER_VISIBLE);
				gpu_heaps[DescriptorHeapType::SAMPLER] = std::make_shared<DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::SAMPLER, DescriptorHeapFlags::SHADER_VISIBLE);

				cpu_heaps[DescriptorHeapType::CBV_SRV_UAV] = std::make_shared<DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::NONE);
				cpu_heaps[DescriptorHeapType::RTV] = std::make_shared<DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::RTV, DescriptorHeapFlags::NONE);
				cpu_heaps[DescriptorHeapType::DSV] = std::make_shared<DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::DSV, DescriptorHeapFlags::NONE);
				cpu_heaps[DescriptorHeapType::SAMPLER] = std::make_shared<DynamicDescriptor<LockPolicy>>(HAL::DescriptorHeapType::SAMPLER, DescriptorHeapFlags::NONE);
			}

			void reset()
			{

				for (auto& h : gpu_heaps)
					if (h)
						h->reset();

				for (auto& h : cpu_heaps)
					if (h)
						h->reset();

				Uploader<LockPolicy>::reset();
			}

		};


		class StaticCompiledGPUData :public Singleton<StaticCompiledGPUData>, public GPUCompiledManager<Thread::Lockable>
		{
		public:
			using Uploader<Thread::Lockable>::place_raw;
		};
		class FrameResources :public SharedObject<FrameResources>, public GPUCompiledManager<Thread::Lockable>
		{

			friend class FrameResourceManager;

			std::uint64_t frame_number = 0;
			std::mutex m;



		public:




			~FrameResources()
			{
				reset();
			}
			using ptr = std::shared_ptr<FrameResources>;
			std::uint64_t get_frame()
			{
				return frame_number;
			}

			std::shared_ptr<CommandList> start_list(std::string name = "", CommandListType type = CommandListType::DIRECT);

		};

		class FrameResourceManager :public Singleton<FrameResourceManager>
		{
			std::atomic_size_t frame_number = 0;

		public:
			FrameResources::ptr begin_frame();
		};

		class CommandListBase : public StateContext
		{
		protected:

			CommandListType type;

			LEAK_TEST(CommandListBase);

			std::vector<std::function<void()>> on_execute_funcs;

			std::list<FenceWaiter> waits;

			CommandListCompilerDelayed compiler;

			std::list<TrackedObject::ptr> tracked_resources;

			CommandListCompilerDelayed* get_native_list()
			{
				return &compiler;
			}
		public:

			template<class T>
			void track_object(const Trackable<T>& obj)
			{
				auto& state = obj.get_state(this);
				if (!state.used)
				{
					state.used = true;
					tracked_resources.emplace_back(obj.tracked_info);
				}
			}

			void free_tracked_objects()
			{
				tracked_resources.clear();
			}
			virtual ~CommandListBase() = default;

			CommandListType get_type()
			{
				return type;
			}
		};

		class TransitionCommandList;


		struct TransitionPoint
		{
			std::list<Transition> transitions;
			std::list<Resource*> uav_transitions;
			std::list<Resource*> aliasing;

			Barriers  compiled_transitions;

			bool start = false;
			TransitionPoint* prev_point = nullptr;
			TransitionPoint* next_point = nullptr;

			TransitionPoint(CommandListType type) :compiled_transitions(type)
			{
			}
		};

		enum class TransitionType :int
		{
			ZERO,
			LAST
		};
		class Transitions : public virtual CommandListBase
		{


			std::list<Resource*> used_resources;

			std::shared_ptr<TransitionCommandList> transition_list;

			friend class SignatureDataSetter;
			friend class Sendable;
			friend class Eventer;
			friend class ResourceStateManager;
		protected:
			void begin();
			void on_execute();
			std::list<TransitionPoint> transition_points;

			void create_transition_point(bool end = true);

			void make_split_barriers();

			void transition(const Resource* resource, ResourceState state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			void transition(const Resource::ptr& resource, ResourceState state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		public:
			void free_resources();

			UINT transition_count = 0;
			//	TransitionPoint* start_transition;
			Transition* create_transition(const Resource* resource, UINT subres, ResourceState state, TransitionType type = TransitionType::LAST)
			{
				TransitionPoint* point = nullptr;

				if (type == TransitionType::LAST) point = &transition_points.back();
				if (type == TransitionType::ZERO) point = &transition_points.front();


				//if (type == TransitionType::LAST) 			assert(!point->start);
				Transition& transition = point->transitions.emplace_back();

				transition.resource = const_cast<Resource*>(resource);
				transition.subres = subres;
				transition.wanted_state = state;

				transition.point = point;
				return &transition;
			}

			void create_uav_transition(const Resource* resource)
			{
				auto& point = transition_points.back();
				point.uav_transitions.emplace_back(const_cast<Resource*>(resource));
			}

			void create_aliasing_transition(const Resource* resource)
			{
				auto& point = transition_points.back();
				point.aliasing.emplace_back(const_cast<Resource*>(resource));
			}

			TransitionPoint* get_last_transition_point()
			{
				return &transition_points.back();
			}

			void use_resource(const Resource* resource);
		public:
			void prepare_transitions(Transitions* to, bool all);


			void merge_transition(Transitions* to, Resource* res);
			void transition_uav(Resource* resource);
			void transition(Resource* from, Resource* to);
			std::shared_ptr<TransitionCommandList> fix_pretransitions();

			void transition_present(const Resource* resource_ptr)
			{

				create_transition_point();

				transition(resource_ptr, ResourceState::PRESENT, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

				create_transition_point(false);
			}


			void transition(const ResourceInfo* info)
			{
				if (!info || !info->resource_ptr) return;

				auto target_state = ResourceState::COMMON;


				if (std::holds_alternative<HAL::Views::ShaderResource>(info->view))
				{
					if (type == CommandListType::DIRECT)
					{
						target_state = ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NON_PIXEL_SHADER_RESOURCE;
					}

					if (type == CommandListType::COMPUTE)
					{
						target_state = ResourceState::NON_PIXEL_SHADER_RESOURCE;
					}

				}
				else 	if (std::holds_alternative<HAL::Views::UnorderedAccess>(info->view))
				{
					target_state = ResourceState::UNORDERED_ACCESS;
				}
				else 	if (std::holds_alternative<HAL::Views::RenderTarget>(info->view))
				{
					target_state = ResourceState::RENDER_TARGET;
				}
				else 	if (std::holds_alternative<HAL::Views::DepthStencil>(info->view))
				{
					target_state = ResourceState::DEPTH_WRITE;

				}
				else assert(false);

				info->for_each_subres([&](const Resource* resource, UINT subres)
					{
						transition(resource, target_state, subres);
					});
			}

			void stop_using(const ResourceInfo* info)
			{
				if (!info || !info->resource_ptr) return;


				info->for_each_subres([&](const Resource* resource, UINT subres)
					{
						resource->stop_using(this, subres);
					});
			}

			//remove this all
			void transition_rtv(const ResourceInfo* info)
			{
				assert(std::holds_alternative<HAL::Views::RenderTarget>(info->view));

				info->for_each_subres([&](const Resource* resource, UINT subres)
					{
						transition(resource, ResourceState::RENDER_TARGET, subres);
					});
			}

			void transition_uav(const ResourceInfo* info)
			{
				assert(std::holds_alternative<HAL::Views::UnorderedAccess>(info->view));

				info->for_each_subres([&](const Resource* resource, UINT subres)
					{
						transition(resource, ResourceState::UNORDERED_ACCESS, subres);
					});
			}


			void transition_dsv(const ResourceInfo* info)
			{
				assert(std::holds_alternative<HAL::Views::DepthStencil>(info->view));

				info->for_each_subres([&](const Resource* resource, UINT subres)
					{
						transition(resource, ResourceState::DEPTH_WRITE, subres);
					});
			}


			void transition_srv(const ResourceInfo* info)
			{
				assert(std::holds_alternative<HAL::Views::ShaderResource>(info->view));

				assert(false);




			}

		};



		class GPUTimer
		{
			friend class GPUTimeManager;

			int id;


		public:
			CommandListType queue_type;
			GPUTimer();

			virtual ~GPUTimer();

			void start(Eventer* list);

			void end(Eventer* list);

			float get_time();


			double get_start();

			double get_end();
		};


		class GPUCounter
		{

		public:
			GPUTimer timer;
			bool enabled = true;

			std::chrono::time_point<std::chrono::high_resolution_clock>  start_time;
			std::chrono::time_point<std::chrono::high_resolution_clock>  end_time;

		};

		class GPUBlock :public TimedBlock
		{
		public:
			using TimedBlock::TimedBlock;

			GPUCounter gpu_counter;
		};


		class Eventer : public virtual CommandListBase, public TimedRoot
		{

			std::list<std::wstring> names;
			TimedBlock* current;

			virtual  void on_start(Timer* timer) override;
			virtual  void on_end(Timer* timer)override;
		protected:
#ifdef DEV
			Exceptions::stack_trace begin_stack;
#endif
			std::string name;
			void reset();
			void begin(std::string name, Timer* t = nullptr);
		public:
			static thread_local Eventer* thread_current;

			virtual Timer start(std::wstring_view name)override;

			std::shared_ptr<Timer> timer;
			// events
			void start_event(std::wstring str);
			void end_event();

			void set_marker(const wchar_t* label);


			// timers
			void insert_time(QueryHeap& pQueryHeap, uint32_t QueryIdx);
			void resolve_times(QueryHeap& pQueryHeap, uint32_t NumQueries, std::function<void(std::span<UINT64>)>);

		};



		class Sendable : public virtual CommandListBase
		{

			friend class Queue;
			std::vector<std::function<void(FenceWaiter)>> on_fence;


			std::promise<FenceWaiter> execute_fence;
			std::shared_future<FenceWaiter> execute_fence_result;


			std::list<FenceWaiter> waits;


			void on_send(FenceWaiter fence)
			{



				execute_fence.set_value(fence);

				for (auto&& e : on_fence)
					e(fence);

				on_fence.clear();
			}
		protected:
			CommandListCompiled compiled;




		public:
			void compile();
			void when_send(std::function<void(FenceWaiter)> e)
			{
				on_fence.emplace_back(e);
			}

			std::shared_future<FenceWaiter> get_execution_fence()
			{
				return execute_fence_result;
			}

			std::shared_future<FenceWaiter> execute(std::function<void()> f = nullptr);
			void execute_and_wait(std::function<void()> f = nullptr);

			void on_done(std::function<void()> f);

		};

		class SignatureDataSetter;


		class CommandList : public SharedObject<CommandList>, public Readbacker, public Transitions, public Eventer, public Sendable, public GPUCompiledManager<Thread::Free>
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

			// TODO: make references?

			virtual void on_execute();

			std::unique_ptr<GraphicsContext> graphics;
			std::unique_ptr<ComputeContext> compute;
			std::unique_ptr<CopyContext> copy;
			PipelineStateBase* current_pipeline;
			PipelineStateBase* first_pipeline;

			std::list<update_tiling_info> tile_updates;



			void set_pipeline_internal(PipelineStateBase* pipeline);

		public:

			void update_tilings(update_tiling_info&& info)
			{
				tile_updates.emplace_back(std::move(info));

				track_object(*info.resource);
			}

			FrameResources::ptr frame_resources;
			void setup_debug(SignatureDataSetter*);
			void print_debug();
			bool first_debug_log = true;
			std::shared_ptr<GPUBuffer> debug_buffer;

			GraphicsContext& get_graphics();
			ComputeContext& get_compute();
			CopyContext& get_copy();


			FrameResources* get_manager()
			{
				return frame_resources.get();
			}

			CommandList(CommandListType);

			void begin(std::string name = "", Timer* t = nullptr);
			void end();


			void clear_uav(const Handle& h, vec4 ClearColor = vec4(0, 0, 0, 0))
			{
				create_transition_point();
				transition_uav(h.get_resource_info());
				auto dx_resource = h.get_resource_info()->resource_ptr->get_hal()->native_resource.Get();

				get_native_list()->ClearUnorderedAccessViewFloat(h.get_gpu(), h.get_cpu(), dx_resource, reinterpret_cast<FLOAT*>(ClearColor.data()), 0, nullptr);
				create_transition_point(false);
			}


			void clear_rtv(const Handle& h, vec4 ClearColor = vec4(0, 0, 0, 0))
			{
				create_transition_point();
				transition_rtv(h.get_resource_info());
				get_native_list()->ClearRenderTargetView(h.get_cpu(), ClearColor.data(), 0, nullptr);
				create_transition_point(false);
			}




			void clear_stencil(Handle dsv, UINT8 stencil = 0)
			{
				create_transition_point();
				transition_dsv(dsv.get_resource_info());

				get_native_list()->ClearDepthStencilView(dsv.get_cpu(), D3D12_CLEAR_FLAG_STENCIL, 0, stencil, 0, nullptr);
				create_transition_point(false);
			}

			void clear_depth(Handle dsv, float depth = 0)
			{
				create_transition_point();
				transition_dsv(dsv.get_resource_info());

				get_native_list()->ClearDepthStencilView(dsv.get_cpu(), D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
				create_transition_point(false);
			}


		};

		class CopyContext
		{
			friend class CommandList;

			CommandList& base;
			CommandListCompilerDelayed* list;

			CopyContext(CommandList& base) :base(base), list(base.get_native_list()) {}
			CopyContext(const CopyContext&) = delete;
			CopyContext(CopyContext&&) = delete;
		public:
			void copy_resource(Resource* dest, Resource* source);
			void copy_resource(const Resource::ptr& dest, const Resource::ptr& source);
			void copy_texture(const Resource::ptr& dest, int, const Resource::ptr& source, int);
			void copy_texture(const Resource::ptr& dest, ivec3, const Resource::ptr& source, ivec3, ivec3);
			void copy_buffer(Resource* dest, int s_dest, Resource* source, int s_source, int size);


			//TODO: remove
			void update_buffer(Resource::ptr resource, UINT offset, const char* data, UINT size);
			void update_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);
			void update_buffer(Resource* resource, UINT offset, const char* data, UINT size);
			void update_texture(Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);



			std::future<bool> read_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)>);
			std::future<bool> read_texture(const Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)>);
			std::future<bool> read_buffer(Resource* resource, unsigned int offset, UINT64 size, std::function<void(const char*, UINT64)>);
			std::future<bool> read_query(std::shared_ptr<QueryHeap>&, unsigned int offset, unsigned int count, std::function<void(const char*, UINT64)>);
		};
		/*
		class StateManager
		{
			std::vector<Handle> rtvs;
			Handle dsv;

			std::vector<Handle> tables;

		public:
			void set_signature(const RootSignature::ptr&);

			template<class Compiled>
			void set_slot(Compiled& compiled)
			{

			}

		};
		*/


		class SignatureDataSetter
		{
			struct RowInfo
			{
				HandleType type;
				bool dirty = false;
				HandleTableLight table;
			};
			std::vector<RowInfo> tables;

			friend class CommandList;


			struct SlotInfo
			{
				SlotID id;
				bool dirty = false;

				const std::vector<UINT>* tables;
			};
			std::vector<SlotInfo> slots;


			RootSignature::ptr root_sig;
			//		UsedSlots used_slots;
		protected:
			CommandList& base;
			SignatureDataSetter(CommandList& base) :base(base) {
				tables.resize(32); // !!!!!!!!!!!
				slots.resize(32); // !!!!!!!!!!!
			}

			virtual void set(UINT, const HandleTableLight&) = 0;
			virtual void set_const_buffer(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS&) = 0;


			void reset_tables()
			{
				for (auto& row : tables)
				{
					row.dirty = false;
				}

			}


			void commit_tables()
			{

				for (auto& row : tables)
				{
					if (!row.dirty) continue;

					auto& table = row.table;
					auto type = row.type;
					for (UINT i = 0; i < (UINT)table.get_count(); ++i)
					{
						const auto& h = table[i];
						get_base().transition(h.get_resource_info());
					}

					row.dirty = false;
				}

				/*
				for (auto& row : slots)
				{
					if (!row.dirty) continue;

					auto& used_tables = *row.tables;

					for (auto &id : used_tables)
					{
						auto& table = tables[id].table;

						for (UINT i = 0; i < (UINT)table.get_count(); ++i)
						{
							const auto& h = table[i];
							if (h.get_resource_info() && h.get_resource_info()->resource_ptr)
							{
								get_base().transition(h.get_resource_info());
							}
						}
					}

					row.dirty = false;
				}*/
			}

		public:

			void reset()
			{
				root_sig = nullptr;
			}
			CommandList& get_base() {
				return base;
			}

			void set_layout(Layouts layout);

			void set_signature(const RootSignature::ptr& signature)
			{
				if (root_sig == signature) return;

				root_sig = signature;

				auto& desc = signature->get_desc();

				on_set_signature(signature);
				//	tables.resize(desc.tables.size());
			}

			virtual void on_set_signature(const RootSignature::ptr& signature) = 0;

			void set_pipeline(std::shared_ptr<PipelineStateBase> pipeline);

			template<HandleType type>
			void set_table(UINT index, const HandleTableLight& table)
			{
				assert(table.valid());

				auto& row = tables[index];

				row.type = type;
				row.table = table;
				row.dirty = true;
				set(index, table);
			}


			void set_cb(UINT index, const ResourceAddress& address)
			{
				if (address.resource)
				{
					get_base().track_object(*address.resource);
					get_base().transition(address.resource, ResourceState::VERTEX_AND_CONSTANT_BUFFER);
				}
				set_const_buffer(index, address.address);
			}

			template<class Compiled>
			void set_slot(Compiled& compiled)
			{
				auto& slot = slots[Compiled::Slot::ID];
				slot.id = Compiled::ID;
				slot.dirty = true;

				//				slot.tables = &Compiled::SlotTable::tables;

				compiled.set_tables(*this);
			}

			template<class T>
			std::unique_ptr<T> wrap()
			{
				auto res = std::make_unique<T>();
				res->begin(this);
				return res;
			}
		};

		class GraphicsContext : public SignatureDataSetter
		{
			friend class CommandList;
			CommandListCompilerDelayed* list;

			GraphicsContext(CommandList& base) :SignatureDataSetter(base), list(base.get_native_list()) {
			}
			GraphicsContext(const GraphicsContext&) = delete;
			GraphicsContext(GraphicsContext&&) = delete;

			template<class T, class W>
			void set(T* _where, W what)
			{
				*_where = what->get_vertex_buffer_view();
				//    what->update(this);
			}

			template<class T, class W, class ...Args>
			void set(T* _where, W what, Args...args)
			{
				*_where = what->get_vertex_buffer_view();
				//what->update(this);
				set(_where++, args...);
			}


			bool valid_scissor = false;
			std::vector<Viewport> viewports;
			D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;



			void begin();
			void end();
			void on_execute();

			void set_const_buffer(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&)override;
			void set(UINT, const HandleTableLight&)override;
			IndexBufferView index;
		public:

			CommandList& get_base()
			{
				return base;
			}

			void set_topology(D3D_PRIMITIVE_TOPOLOGY topology)
			{
				if (this->topology != topology)
					list->IASetPrimitiveTopology(topology);

				this->topology = topology;
			}


			void on_set_signature(const RootSignature::ptr&) override;
			//	void set_pipeline(std::shared_ptr<PipelineState>);




			void set_heaps(DescriptorHeap::ptr& a, DescriptorHeap::ptr& b);

			void set_scissor(sizer_long rect);
			void set_viewport(Viewport viewport);
			void set_viewport(vec4 viewport);
			void set_scissors(sizer_long rect);
			void set_viewports(std::vector<Viewport> viewports);


			void set_rtv(std::initializer_list<Handle> rt, Handle h);
			void set_rtv(const HandleTable&, Handle);
			void set_rtv(int c, Handle rt, Handle h);
			void set_rtv(const HandleTableLight&, Handle);

			void draw(D3D12_DRAW_INDEXED_ARGUMENTS args)
			{
				draw_indexed(args.IndexCountPerInstance, args.StartIndexLocation, args.BaseVertexLocation, args.InstanceCount, args.StartInstanceLocation);
			}

			void dispatch_mesh(ivec3 v);
			void dispatch_mesh(D3D12_DISPATCH_MESH_ARGUMENTS args);


			HandleTableLight place_rtv(UINT count)
			{
				return get_base().get_cpu_heap(DescriptorHeapType::RTV).place(count);
			}

			HandleTableLight place_dsv(UINT count)
			{
				return get_base().get_cpu_heap(DescriptorHeapType::DSV).place(count);
			}

			HandleTableLight place_rtv(std::initializer_list<Handle> list)
			{
				return get_base().get_cpu_heap(DescriptorHeapType::RTV).place(list);
			}

			HandleTableLight place_dsv(std::initializer_list<Handle> list)
			{
				return get_base().get_cpu_heap(DescriptorHeapType::DSV).place(list);
			}


			void set_stencil_ref(UINT ref)
			{
				list->OMSetStencilRef(ref);
			}
			template<class T, size_t N>
			constexpr size_t size(T(&)[N]) { return N; }

			template<class ...Handles>
			void set_rtvs(const Handle& h, Handles... rtvlist)
			{

				auto f = [&](Handle h) {
					get_base().transition_rtv(h.get_resource_info());
				};

				(f(std::forward<Handles>(rtvlist)), ...);


				if (h.is_valid())
					get_base().transition_dsv(h.get_resource_info());


				CD3DX12_CPU_DESCRIPTOR_HANDLE ar[] = { (rtvlist.get_cpu_read())... };
				assert(false);//list->OMSetRenderTargets(size(ar), ar, false, h.is_valid() ? &h.get_cpu_read() : nullptr);
			}


			std::vector<Viewport> get_viewports()
			{
				return viewports;
			}

			void set_index_buffer(IndexBufferView view)
			{
				index = view;

			}



			void draw(UINT vertex_count, UINT vertex_offset = 0, UINT instance_count = 1, UINT instance_offset = 0);
			void draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count = 1, UINT instance_offset = 0);
			void execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset = 0, Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);

		};


		struct GeometryDesc
		{
			D3D12_RAYTRACING_GEOMETRY_TYPE Type;
			D3D12_RAYTRACING_GEOMETRY_FLAGS Flags;

			ResourceAddress Transform3x4;
			DXGI_FORMAT IndexFormat;
			DXGI_FORMAT VertexFormat;
			UINT IndexCount;
			UINT VertexCount;
			ResourceAddress IndexBuffer;

			ResourceAddress VertexBuffer;
			UINT64 VertexStrideInBytes;
		};
		struct InstanceDesc
		{
			FLOAT Transform[3][4];
			UINT InstanceID : 24;
			UINT InstanceMask : 8;
			UINT InstanceContributionToHitGroupIndex : 24;
			UINT Flags : 8;
			ResourceAddress AccelerationStructure;
		};



		struct RaytracingDesc
		{
			ResourceAddress DestAccelerationStructureData;
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS Inputs;
			ResourceAddress SourceAccelerationStructureData;
			ResourceAddress ScratchAccelerationStructureData;
		};

		struct RaytracingBuildDescBottomInputs
		{
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags;



			void add_geometry(GeometryDesc i)
			{
				D3D12_RAYTRACING_GEOMETRY_DESC geom;
				geom.Flags = i.Flags;
				geom.Type = i.Type;
				geom.Triangles.IndexBuffer = i.IndexBuffer.address;
				geom.Triangles.IndexCount = i.IndexCount;
				geom.Triangles.IndexFormat = i.IndexFormat;

				geom.Triangles.VertexBuffer.StartAddress = i.VertexBuffer.address;
				geom.Triangles.VertexBuffer.StrideInBytes = i.VertexStrideInBytes;
				geom.Triangles.VertexFormat = i.VertexFormat;

				geom.Triangles.Transform3x4 = i.Transform3x4.address;
				descs.emplace_back(geom);

				geometry.emplace_back(i);
			}

			std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> descs;
			std::vector<GeometryDesc> geometry;

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native() const
			{
				D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;

				inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
				inputs.Flags = Flags;
				inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY;
				inputs.NumDescs = static_cast<UINT>(descs.size());
				inputs.pGeometryDescs = descs.data();

				return inputs;
			}
		};

		struct RaytracingBuildDescTopInputs
		{
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags;
			UINT NumDescs;
			ResourceAddress instances;
		};

		struct RaytracingBuildDescStructure
		{
			ResourceAddress DestAccelerationStructureData;
			ResourceAddress SourceAccelerationStructureData;
			ResourceAddress ScratchAccelerationStructureData;
		};


		class ComputeContext : public SignatureDataSetter
		{
			friend class CommandList;


			CommandListCompilerDelayed* list;

			ComputeContext(CommandList& base) :SignatureDataSetter(base), list(base.get_native_list()) {}
			ComputeContext(const ComputeContext&) = delete;
			ComputeContext(ComputeContext&&) = delete;


			//	std::shared_ptr<ComputePipelineState> current_compute_pipeline;
			std::shared_ptr<RootSignature> current_compute_root_signature;

			void begin();
			void end();
			void on_execute();




			void set(UINT, const HandleTableLight&)override;






			virtual void set_const_buffer(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&) override;


		public:

			CommandList& get_base()
			{
				return base;
			}


			void on_set_signature(const RootSignature::ptr&) override;
			//	void set_pipeline(std::shared_ptr<ComputePipelineState>);


			void dispach(int = 1, int = 1, int = 1);
			void dispach(ivec2, ivec2 = ivec2(8, 8));
			void dispach(ivec3, ivec3 = ivec3(4, 4, 4));




			void execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset = 0, Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);



			void build_ras(const RaytracingBuildDescStructure& build_desc, const RaytracingBuildDescBottomInputs& bottom);
			void build_ras(const RaytracingBuildDescStructure& build_desc, const RaytracingBuildDescTopInputs& top);

			template<class Hit, class Miss, class Raygen>
			void dispatch_rays(ivec2 size, ResourceAddress hit_buffer, UINT hit_count, ResourceAddress miss_buffer, UINT miss_count, ResourceAddress raygen_buffer)
			{

				base.setup_debug(this);
				D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
				// Since each shader table has only one shader record, the stride is same as the size.
				dispatchDesc.HitGroupTable.StartAddress = hit_buffer.address;
				dispatchDesc.HitGroupTable.SizeInBytes = sizeof(Hit) * hit_count;
				dispatchDesc.HitGroupTable.StrideInBytes = sizeof(Hit);

				dispatchDesc.MissShaderTable.StartAddress = miss_buffer.address;
				dispatchDesc.MissShaderTable.SizeInBytes = sizeof(Miss) * miss_count;
				dispatchDesc.MissShaderTable.StrideInBytes = sizeof(Miss);

				dispatchDesc.RayGenerationShaderRecord.StartAddress = raygen_buffer.address;
				dispatchDesc.RayGenerationShaderRecord.SizeInBytes = sizeof(Raygen);
				dispatchDesc.Width = size.x;
				dispatchDesc.Height = size.y;
				dispatchDesc.Depth = 1;

				base.create_transition_point();

				base.transition(hit_buffer.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
				base.transition(miss_buffer.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
				base.transition(raygen_buffer.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);

				commit_tables();
				list->DispatchRays(&dispatchDesc);

				base.create_transition_point(false);

				get_base().print_debug();
			}

		};


		class TransitionCommandList
		{
			ComPtr<ID3D12GraphicsCommandList4> m_commandList;
			ComPtr<ID3D12CommandAllocator> m_commandAllocator;
			CommandListType type;
		public:
			using ptr = std::shared_ptr<TransitionCommandList>;
			inline CommandListType get_type() { return type; }
			TransitionCommandList(CommandListType type);
			void create_transition_list(const Barriers& transitions, std::vector<Resource*>& duscards);
			ComPtr<ID3D12GraphicsCommandList4> get_native();
		};
	}

}