#pragma once

namespace DX12
{

	typedef D3D12_VIEWPORT Viewport;
	class PipelineState;
	class ComputePipelineState;

	class IndexBufferBase;
	class UploadBuffer;
	class CPUBuffer;

	class GPUBuffer;

	template<class T> class StructuredBuffer;
	//  using BufferBase = GPUBuffer;
	class QueryHeap;

	class Texture;
	class CommandList;

#define DEFAULT_ALIGN 256
	enum class CommandListType : int
	{
		DIRECT = D3D12_COMMAND_LIST_TYPE_DIRECT,
		BUNDLE = D3D12_COMMAND_LIST_TYPE_BUNDLE,
		COMPUTE = D3D12_COMMAND_LIST_TYPE_COMPUTE,
		COPY = D3D12_COMMAND_LIST_TYPE_COPY
	};

	class BufferCache : public Singleton<BufferCache>
	{
		std::multimap<UINT64, std::shared_ptr<CPUBuffer>> read_back_resources;
		std::multimap<UINT64, std::shared_ptr<UploadBuffer>> upload_resources;

		std::mutex upload;
		std::mutex read_back;

	public:

		std::shared_ptr<UploadBuffer> get_upload(UINT size);
		std::shared_ptr<CPUBuffer> get_readback(UINT size);

		void on_execute_list(CommandList* list);
		void unused_upload(std::vector<std::shared_ptr<UploadBuffer>>& upload_resources);
	};
	class GraphicsContext;
	class ComputeContext;
	class FrameResourceManager;
	class FrameResources
	{

		friend class FrameResourceManager;
		std::vector<std::shared_ptr<UploadBuffer>> upload_resources;
		std::uint64_t  resource_offset = 0;

		size_t heap_size = 0x200000;

		std::uint64_t frame_number = 0;
		std::mutex m;
	public:
		using ptr = std::shared_ptr<FrameResources>;
		std::uint64_t get_frame()
		{
			return frame_number;
		}
		struct UploadInfo
		{
			std::shared_ptr<UploadBuffer> resource;
			std::uint64_t offset;
			std::uint64_t frame = -10;

			D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address();
		};

		UploadInfo place_data(std::uint64_t uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN);


		template<class T>
		UploadInfo set_const_buffer(const T& data)
		{
			auto info = place_data(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			memcpy(info.resource->get_data() + info.offset, &data, sizeof(T));
			info.frame = frame_number;
			return info;
		}
		template<class T>
		UploadInfo set_data(T* data, int size)
		{
			auto info = place_data(size * sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			memcpy(info.resource->get_data() + info.offset, data, size * sizeof(T));
			info.frame = frame_number;
			return info;
		}
		/*
				void begin()
				{
					resource_offset = 0;

					if (upload_resources.size())
						upload_resources.clear();
				}

				void end()
				{

				}*/
		~FrameResources()
		{
			std::lock_guard<std::mutex> g(m);
			BufferCache::get().unused_upload(upload_resources);
		}

	};


	class FrameResourceManager
	{
		std::uint64_t frame_number = 0;
		std::list<FrameResources::ptr> frames;
		std::mutex m;

		FrameResources* current;
		void end_frame()
		{
			std::lock_guard<std::mutex> g(m);
			frames.pop_front();
		}
	public:
		std::uint64_t get_frame()
		{
			return frame_number;
		}

		std::shared_ptr<CommandList> start_frame(std::string name);

		std::shared_ptr<CommandList> start_frame(std::shared_ptr<CommandList>, std::string name);
		std::shared_ptr<CommandList> set_frame(std::shared_ptr<CommandList>, std::string name);

		FrameResources& get_creator()
		{
			return *current;
		}
	};

	class FrameResource
	{
	protected:
		std::mutex m;
	public:
		virtual FrameResources::UploadInfo get_for(FrameResources& manager) = 0;
	};


	template<class T>
	class FrameStorageConstBuffer : public FrameResource
	{
		FrameResources::UploadInfo info;
		T _data;
	public:

		using ptr = std::shared_ptr<FrameStorageConstBuffer>;

		T& data()
		{
			return _data;
		}
		const T& data() const
		{
			return _data;
		}
		FrameResources::UploadInfo get_for(FrameResources& manager) override
		{
			m.lock();
			if (info.frame != manager.get_frame())
				info = manager.set_const_buffer(_data);

			assert(info.resource);
			m.unlock();
			return info;
		}
	};

	template<class T>
	class FrameStorage : public FrameResource
	{
		FrameResources::UploadInfo info;
		std::vector<T> _data;
	public:

		using ptr = std::shared_ptr<FrameStorage<T>>;

		std::vector<T>& data()
		{
			return _data;
		}

		FrameResources::UploadInfo get_for(FrameResources& manager) override
		{
			m.lock();
			if (info.frame != manager.get_frame())
				info = manager.set_data(_data.data(), _data.size());

			assert(info.resource);
			m.unlock();
			return info;
		}

		using type = T;
	};

	class CommandListBase
	{
	protected:
		int id = -1;
		std::uint64_t global_id;
		bool executed = false;
		CommandListType type;

		LEAK_TEST(CommandListBase)

		std::vector<std::function<void()>> on_execute_funcs;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
	public:
		ComPtr<ID3D12GraphicsCommandList> get_native_list()
		{
			return m_commandList;
		}
		virtual ~CommandListBase() = default;
	};

	class TransitionCommandList;

	class Transitions : public virtual CommandListBase
	{
		std::array<D3D12_RESOURCE_BARRIER, 16> transitions;
		unsigned int transition_count = 0;

		std::list<Resource*> used_resources;
		std::shared_ptr<TransitionCommandList> transition_list;
	protected:
		void reset();
	public:

		std::shared_ptr<TransitionCommandList> fix_pretransitions();
		
	//	void assume_state(const Resource::ptr& resource, ResourceState state);
	//	void assume_state(Resource* resource, ResourceState state);

		void transition(const Resource* resource, unsigned int state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		void transition(const Resource::ptr& resource, unsigned int state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		void transition(const std::shared_ptr<Texture>& resource, unsigned int state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		//void transition(Resource* resource, unsigned int from, unsigned int to, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		//void transition(const Resource::ptr& resource, unsigned int from, unsigned int to, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		void transition_uav(Resource* resource);
		void transition(Resource* from, Resource* to);

		void flush_transitions();
	};



	class Uploader : public virtual CommandListBase
	{
		friend class BufferCache;

		std::vector<std::shared_ptr<UploadBuffer>> upload_resources;
		UINT64 resource_offset;
		size_t heap_size = 0x200000;
	protected:
		void reset();


	public:
		struct UploadInfo
		{
			std::shared_ptr<UploadBuffer> resource;
			UINT64 offset;
		};

		UploadInfo place_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN);

	};


	class Readbacker : public virtual CommandListBase
	{
		friend class BufferCache;

		std::vector<std::shared_ptr<CPUBuffer>> read_back_resources;
	protected:
		void reset();
	public:

		struct ReadBackInfo
		{
			std::shared_ptr<CPUBuffer> resource;
			UINT64 offset;
		};

		ReadBackInfo read_data(UINT64 uploadBufferSize);

	};
	class GPUTimeManager;
	class Eventer;
	
	class GPUTimer
	{
		friend class GPUTimeManager;

		int id;
	public:
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

		//Events::prop<float2>::event_type on_time;

		std::chrono::time_point<std::chrono::high_resolution_clock>  start_time;
		std::chrono::time_point<std::chrono::high_resolution_clock>  end_time;

		//Events::prop<float2> time;

		//std::chrono::duration<double> elapsed_time;


	};

	class GPUBlock:public TimedBlock
	{
	public:
		GPUBlock(std::wstring name) :TimedBlock(name) {};


		GPUCounter gpu_counter;
	};


	class Eventer : public virtual CommandListBase, public TimedRoot
	{
		std::shared_ptr<Timer> timer;
		std::list<std::wstring> names;
		TimedBlock* current;

		virtual  void on_start(Timer* timer) override;
		virtual  void on_end(Timer* timer)override;
	protected:
		void reset();
		void begin(std::string name, Timer*t = nullptr);
	public:
		virtual Timer start(const wchar_t* name)override;


		// events
		void start_event(std::wstring str);
		void end_event();

		void set_marker(const wchar_t* label);


		// timers
		void insert_time(QueryHeap& pQueryHeap, uint32_t QueryIdx);
		void resolve_times(Resource* resource, QueryHeap& pQueryHeap, uint32_t NumQueries, std::function<void()>);

	};

	class Sendable : public virtual CommandListBase
	{

		friend class Queue;
		std::vector<std::function<void()>> on_send_funcs;
		std::vector<std::function<void(UINT64)>> on_fence;
		
	
		std::promise<UINT64> execute_fence;
		std::shared_future<UINT64> execute_fence_result;

		std::mutex execution_mutex;

		bool child_executed()
		{
			execution_mutex.lock();
			--wait_for_execution_count;
			bool res = executed && (wait_for_execution_count == 0);
			execution_mutex.unlock();
			return res;
		}

		void on_send()
		{
			auto fence = execute_fence_result.get();
			for (auto&&e : on_fence)
				e(fence);
			on_fence.clear();
		}

		std::shared_ptr<Sendable> parent_list = nullptr;
		std::atomic_int wait_for_execution_count;
	protected:
	
		virtual void on_execute()
		{
			parent_list = nullptr;
		}
		void set_parent(std::shared_ptr<Sendable> parent)
		{
			parent_list = parent;
			parent->wait_for_execution_count++;
		}

		void reset()
		{
			wait_for_execution_count = 0;
		}
	public:


		void when_send(std::function<void(UINT64)> e)
		{
			on_fence.emplace_back(e);
		}

		std::shared_future<UINT64> get_execution_fence()
		{
			return execute_fence_result;
		}

		std::shared_future<UINT64> execute(std::function<void()> f = nullptr);
		void execute_and_wait(std::function<void()> f = nullptr);

		void on_done(std::function<void()> f);

	};
	class CommandList : public std::enable_shared_from_this<CommandList>, public Uploader,public Readbacker, public Transitions, public Eventer, public Sendable
	{
	public:
		using ptr = shared_ptr<CommandList>;
		std::map<Resource*, int> resource_update_counter;
	protected:
		CommandList() = default;
		friend class Queue;
		friend class FrameResourceManager;
	
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;

		bool valid_scissor = false;
		std::vector<Viewport> viewports;
		D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		//	std::vector<std::shared_ptr<BufferBase>>::iterator upload_iterator;

		//	std::vector<std::function<void(Queue*)>> on_pre_execute_funcs;



		//  UINT64 resource_index;
	
		std::shared_ptr<PipelineState> current_pipeline;
		std::shared_ptr<RootSignature> current_root_signature;

		std::shared_ptr<ComputePipelineState> current_compute_pipeline;

		std::shared_ptr<RootSignature> current_compute_root_signature;

	

		// TODO: make references?
	


		virtual void on_execute() override;
		DynamicDescriptorManager descriptor_manager_shared;
	
		DynamicDescriptorManager descriptor_manager_graphics;
		DynamicDescriptorManager descriptor_manager_compute;

		bool heaps_changed = false;

		std::array<DescriptorHeap::ptr, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> heaps;
	
	
	public:
		ptr get_sub_list();
		FrameResources* frame_resources = nullptr;

		
		void set_heap(DescriptorHeapType type, DescriptorHeap::ptr heap)
		{
			auto i_type = static_cast<int>(type);
			if (heaps[i_type] == heap) return;

			heaps_changed = true;
			heaps[i_type] = heap;
			flush_heaps();

		}

		void flush_heaps(bool force=false)
		{
			if (!heaps_changed&&!force) return;
			heaps_changed = false;
			std::array<ID3D12DescriptorHeap*, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES - 1> native_heaps;

			int count = 0;
			for (auto&e : heaps)
			{
				if (!e) continue;
				native_heaps[count] = e->get_native().Get();
				count++;
			}
			if (count)
				m_commandList->SetDescriptorHeaps(count, native_heaps.data());

		}
		FrameResources* get_manager()
		{
			return frame_resources;
		}
	
		ptr get_ptr()
		{
			return std::static_pointer_cast<CommandList>(shared_from_this());
		}
		CommandList(CommandListType);

		void begin(std::string name = "", Timer*t=nullptr);
		void end();

		int get_id()
		{
			return id;
		}



		template<class T> 
		void clear_uav(T& resource, const Handle& h, ivec4 ClearColor = ivec4(0,0,0,0))
		{
			flush_transitions();
			auto handle = descriptor_manager_shared.place(h);	
			get_native_list()->ClearUnorderedAccessViewUint(handle.gpu, h.cpu, resource->get_native().Get(), reinterpret_cast<UINT*>(ClearColor.data()), 0, nullptr);
		}

		template<class T>
		void clear_uav(T& resource, const Handle& h, vec4 ClearColor)
		{
			flush_transitions();
			auto handle = descriptor_manager_shared.place(h);
			get_native_list()->ClearUnorderedAccessViewFloat(handle.gpu, h.cpu, resource->get_native().Get(), reinterpret_cast<FLOAT*>(ClearColor.data()), 0, nullptr);
		}

	
		void clear_rtv(const Handle& h, vec4 ClearColor = vec4(0, 0, 0, 0))
		{
			flush_transitions();
			get_native_list()->ClearRenderTargetView( h.cpu, ClearColor.data(), 0, nullptr);
		}

		template<class T>
		void clear_counter(std::shared_ptr<StructuredBuffer<T>>& buffer)
		{
			clear_uav(buffer->help_buffer, buffer->counted_uav[0]);
		}

		void copy_resource(Resource* dest, Resource* source);
		void copy_resource(const Resource::ptr& dest, const Resource::ptr& source);
		void copy_texture(const Resource::ptr& dest, int, const Resource::ptr& source, int);
		void copy_texture(const Resource::ptr& dest, ivec3, const Resource::ptr& source, ivec3, ivec3);

		void copy_buffer(Resource* dest, int s_dest, Resource* source, int s_source, int size);


		GraphicsContext& get_graphics();
		ComputeContext& get_compute();




	

		//TODO: remove
		void update_resource(Resource::ptr resource, UINT first_subresource, UINT sub_count, D3D12_SUBRESOURCE_DATA* data);
		void update_buffer(Resource::ptr resource, UINT offset, const char* data, UINT size);
		void update_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);

		void update_buffer(Resource* resource, UINT offset, const char* data, UINT size);
		void update_texture(Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);

		std::future<bool> read_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)>);
		std::future<bool> read_texture(const Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)>);
		std::future<bool> read_buffer(Resource* resource, unsigned int offset, UINT64 size, std::function<void(const char*, UINT64)>);
		std::future<bool> read_query(std::shared_ptr<QueryHeap>&, unsigned int offset, unsigned int count, std::function<void(const char*, UINT64)>);

	};



	class GraphicsContext : public CommandList
	{
		GraphicsContext() = default;
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
		template<class T>
		void set_constants_internal(UINT i, UINT offset, T args)
		{
			m_commandList->SetGraphicsRoot32BitConstant(i, *reinterpret_cast<UINT*>(&args), offset);
		}

		template<class T, class... Args>
		void set_constants_internal(UINT i, UINT offset, T v, Args...args)
		{
			set_constants_internal(i, offset, v);
			set_constants_internal(i, offset + 1, args...);
		}
	public:
		void flush_binds(bool force = false);
		void set_topology(D3D_PRIMITIVE_TOPOLOGY topology)
		{
			if (this->topology != topology)
				m_commandList->IASetPrimitiveTopology(topology);

			this->topology = topology;
		}

		DynamicDescriptorManager& get_desc_manager()
		{
			return descriptor_manager_graphics;
		}

		void set_signature(const RootSignature::ptr&);
		void set_pipeline(std::shared_ptr<PipelineState>&);
		void set(UINT, const HandleTable&);
		void set(UINT, const Handle&);
		void set(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&);
		void set_uav(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&);

		void set_const_buffer(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&);
		void set(UINT, std::vector<Handle>&);

		void set_heaps(DescriptorHeap::ptr& a, DescriptorHeap::ptr& b);

		void set_scissor(sizer_long rect);
		void set_viewport(Viewport viewport);

		void set_viewport(vec4 viewport);

		void set_scissors(sizer_long rect);
		void set_viewports(std::vector<Viewport> viewports);
		//        void set_const_buffer(UINT i, std::shared_ptr<BufferBase>& buff);
		void set_const_buffer(UINT i, std::shared_ptr<GPUBuffer>& buff);
		void set_const_buffer(UINT i, const FrameResources::UploadInfo& info);
		void set(UINT i, FrameResource& info);

		template<class... Args>
		void set_constants(UINT i, Args...args)
		{
			set_constants_internal(i, 0, args...);
		}


		void set_rtv(const HandleTable&, Handle);
		void set_rtv(int c, Handle rt, Handle h);
		void set_dynamic(UINT, UINT, const Handle&);
		void set_dynamic(UINT, UINT, const HandleTable&);

		void set_srv(UINT i, FrameResource& info);
		void set_srv(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&);

		void set_rtvs_internal(D3D12_CPU_DESCRIPTOR_HANDLE* t, Handle h)
		{
			*t = h.cpu;
		}

		template<class ...Handles>
		void set_rtvs_internal(D3D12_CPU_DESCRIPTOR_HANDLE* t, Handle h, Handles... list)
		{
			set_rtvs_internal(t, h);
			set_rtvs_internal(t + 1, list...);
		}

		template<class ...Handles>
		void set_rtvs(const Handle& h, Handles... list)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE handles[sizeof...(Handles)];
			set_rtvs_internal(handles, list...);
			m_commandList->OMSetRenderTargets(sizeof...(Handles), handles, false, h.is_valid() ? &h.cpu : nullptr);
		}


		std::vector<Viewport> get_viewports()
		{
			return viewports;
		}



		template<class ...Args>
		void set_vertex_buffers(UINT offset, Args... args)
		{
			int count = sizeof...(Args);
			D3D12_VERTEX_BUFFER_VIEW views[sizeof...(Args)];
			set(views, args...);
			m_commandList->IASetVertexBuffers(offset, count, views);
		}

		void set_vertex_buffers(UINT offset, std::vector<D3D12_VERTEX_BUFFER_VIEW>& list)
		{
			m_commandList->IASetVertexBuffers(offset, list.size(), list.data());
		}

		void set_vertex_buffer(UINT offset, D3D12_VERTEX_BUFFER_VIEW& list)
		{
			m_commandList->IASetVertexBuffers(offset, 1, &list);
		}

		void set_index_buffer(D3D12_INDEX_BUFFER_VIEW view)
		{
			//   buffer->update(this);
			m_commandList->IASetIndexBuffer(&view);
		}
		template<class T>
		D3D12_VERTEX_BUFFER_VIEW place_vertex_buffer(const std::vector<T>& data)
		{
			//       assert(Math::IsAligned(data.data(), 16));
			size_t BufferSize = Math::AlignUp(data.size() * sizeof(T), 16);
			auto info = place_data(BufferSize);
			memcpy(info.resource->get_data() + info.offset, data.data(), data.size() * sizeof(T));
			D3D12_VERTEX_BUFFER_VIEW res;
			res.SizeInBytes = BufferSize;
			res.StrideInBytes = sizeof(T);
			res.BufferLocation = info.resource->get_gpu_address() + info.offset;
			return res;
		}
		template<class T>
		D3D12_VERTEX_BUFFER_VIEW place_vertex_buffer(const T* data, size_t size)
		{
			//       assert(Math::IsAligned(data.data(), 16));
			auto info = place_data(size * sizeof(T));
			memcpy(info.resource->get_data() + info.offset, data, size * sizeof(T));
			D3D12_VERTEX_BUFFER_VIEW res;
			res.SizeInBytes = size * sizeof(T);
			res.StrideInBytes = sizeof(T);
			res.BufferLocation = info.resource->get_gpu_address() + info.offset;
			return res;
		}

		template<class T>
		void set_const_buffer(UINT i, const T& data)
		{
			//       assert(Math::IsAligned(data.data(), 16));
			//  size_t BufferSize = Math::AlignUp(sizeof(T), 16);
			auto info = place_data(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			memcpy(info.resource->get_data() + info.offset, &data, sizeof(T));
			m_commandList->SetGraphicsRootConstantBufferView(i, info.resource->get_gpu_address() + info.offset);
		}




		void draw(UINT vertex_count, UINT vertex_offset = 0, UINT instance_count = 1, UINT instance_offset = 0);
		void draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count = 1, UINT instance_offset = 0);
		template<class T>
		void set_srv(UINT i, const std::vector<T>& data)
		{
			size_t BufferSize = Math::AlignUp(data.size() * sizeof(T), 16);
			auto info = place_data(BufferSize);
			memcpy(info.resource->get_data() + info.offset, data.data(), data.size() * sizeof(T));
			m_commandList->SetGraphicsRootShaderResourceView(i, info.resource->get_gpu_address() + info.offset);
		}


		void execute_indirect(ComPtr<ID3D12CommandSignature> command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset = 0, Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);

	};



	class ComputeContext : public CommandList
	{
		ComputeContext() = default;
		ComputeContext(const ComputeContext&) = delete;
		ComputeContext(ComputeContext&&) = delete;

		template<class T>
		void set_constants_internal(UINT i, UINT offset, T args)
		{
			m_commandList->SetComputeRoot32BitConstant(i, *reinterpret_cast<UINT*>(&args), offset);
		}

		template<class T, class... Args>
		void set_constants_internal(UINT i, UINT offset, T v, Args...args)
		{
			set_constants_internal(i, offset, v);
			set_constants_internal(i, offset + 1, args...);
		}
	public:
		void flush_binds(bool force = false);
		void set_signature(const RootSignature::ptr&);
		void set_pipeline(std::shared_ptr<ComputePipelineState>&);
		void set(UINT, const HandleTable&);
		//        void set(UINT, const Handle&);
		void set_dynamic(UINT, UINT, const Handle&);
		void set_dynamic(UINT, UINT, const HandleTable&);

		void set_table(UINT, const Handle&);
		void dispach(int=1,int= 1,int=1);
		void dispach(ivec2, ivec2 = ivec2(8, 8));
		void dispach(ivec3, ivec3 = ivec3(4, 4, 4));

		void set_const_buffer(UINT i, std::shared_ptr<GPUBuffer> buff);
		void set_const_buffer(UINT i, const FrameResources::UploadInfo& info);
		void set_srv(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&);
		void set_uav(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&);

		void set(UINT i, FrameResource& info);

		/* template<class T>
		 void set_constants(UINT i, T v1)
		 {
			 m_commandList->SetComputeRoot32BitConstant(i, *reinterpret_cast<UINT*>(&v1), offset);
		 }*/

		template<class... Args>
		void set_constants(UINT i, Args...args)
		{
			set_constants_internal(i, 0, args...);
		}

		template<class T>
		void set_srv(UINT i, const std::vector<T>& data)
		{
			size_t BufferSize = Math::AlignUp(data.size() * sizeof(T), 16);
			auto info = place_data(BufferSize);
			memcpy(info.resource->get_data() + info.offset, data.data(), data.size() * sizeof(T));
			m_commandList->SetComputeRootShaderResourceView(i, info.resource->get_gpu_address() + info.offset);
		}


		template<class T>
		void set_const_buffer(UINT i, const T& data)
		{
			//       assert(Math::IsAligned(data.data(), 16));
			//  size_t BufferSize = Math::AlignUp(sizeof(T), 16);
			auto info = place_data(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			memcpy(info.resource->get_data() + info.offset, &data, sizeof(T));
			m_commandList->SetComputeRootConstantBufferView(i, info.resource->get_gpu_address() + info.offset);
		}

		template<class T>
		void set_const_buffer(UINT i, const T* data, int size)
		{
			//       assert(Math::IsAligned(data.data(), 16));
			//  size_t BufferSize = Math::AlignUp(sizeof(T), 16);
			auto info = place_data(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			memcpy(info.resource->get_data() + info.offset, data, size);
			m_commandList->SetComputeRootConstantBufferView(i, info.resource->get_gpu_address() + info.offset);
		}


		void execute_indirect(ComPtr<ID3D12CommandSignature> command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset = 0, Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);

	};



	class TransitionCommandList
	{
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;

	public:
		using ptr = std::shared_ptr<TransitionCommandList>;

		TransitionCommandList();
		void create_transition_list(const std::vector<D3D12_RESOURCE_BARRIER> &transitions);
		ComPtr<ID3D12GraphicsCommandList> get_native();
	};
}
