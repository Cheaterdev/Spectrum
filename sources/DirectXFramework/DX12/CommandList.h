#pragma once

enum class Layouts;
namespace DX12
{

	class PipelineState;
	template<class T> class  PipelineStateTyped;
	class ComputePipelineState;
	class PipelineStateBase;

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

		std::shared_ptr<UploadBuffer> get_upload(UINT64 size);
		std::shared_ptr<CPUBuffer> get_readback(UINT64 size);

		void on_execute_list(CommandList* list);
		void unused_upload(std::vector<std::shared_ptr<UploadBuffer>>& upload_resources);
	};
	class GraphicsContext;
	class ComputeContext;
	class CopyContext;
	class FrameResourceManager;



	class Uploader
	{
		friend class BufferCache;

		std::vector<std::shared_ptr<UploadBuffer>> upload_resources;
		UINT64 resource_offset;
		UINT heap_size = 0x200000;
	protected:
		void reset();

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


	public:
		struct UploadInfo
		{
			std::shared_ptr<UploadBuffer> resource;
			UINT64 offset;
			UINT64 size;

			D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address();
			ResourceAddress get_resource_address();

			Handle create_cbv(CommandList& list);
		};

		UploadInfo place_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN);



		template<class ...Args>
		UploadInfo place_raw(Args... args)
		{
			size_t size = (0 + ... + size_of(args));



			auto info = place_data(size);


			//	memcpy(info.resource->get_data() + info.offset, data, size);
			size_t start = 0;
			{
				auto timer = Profiler::get().start(L"write");
				(write(info, start, std::forward<Args>(args)), ...);

			}
			return info;
		}

		template<class T>
		void write(UploadInfo& info, std::vector<T>& arg)
		{
			memcpy(info.resource->get_data() + info.offset, arg.data(), arg.size() * sizeof(T));
		}

		template<class T>
		void write(UploadInfo& info, my_unique_vector<T>& arg)
		{
			memcpy(info.resource->get_data() + info.offset, arg.data(), arg.size() * sizeof(T));
		}

		template<class T>
		void write(UploadInfo& info, size_t& offset, std::vector<T>& arg)
		{
			memcpy(info.resource->get_data() + info.offset + offset, arg.data(), arg.size() * sizeof(T));
			offset += arg.size() * sizeof(T);
		}


		template<class T>
		void write(UploadInfo& info, size_t& offset, my_unique_vector<T>& arg)
		{
			memcpy(info.resource->get_data() + info.offset + offset, arg.data(), arg.size() * sizeof(T));
			offset += arg.size() * sizeof(T);

		}
		template<class T>
		void write(UploadInfo& info, size_t& offset, T& arg)
		{
			memcpy(info.resource->get_data() + info.offset + offset, &arg, sizeof(T));
			offset += sizeof(T);
		}
	};




	template<class LockPolicy = Free>
	class GPUCompiledManager : public Uploader
	{
		//	using cb_buffer = virtual_gpu_buffer<std::byte>;

		//	static Cache<std::shared_ptr<cb_buffer>> cbv_cache;
	public:
		DynamicDescriptor<DescriptorHeapType::CBV_SRV_UAV, LockPolicy, DescriptorHeapFlags::SHADER_VISIBLE> srv;
		DynamicDescriptor<DescriptorHeapType::SAMPLER, LockPolicy, DescriptorHeapFlags::SHADER_VISIBLE> smp;

		DynamicDescriptor<DescriptorHeapType::CBV_SRV_UAV, LockPolicy> srv_uav_cbv_cpu;
		DynamicDescriptor<DescriptorHeapType::RTV, LockPolicy> rtv_cpu;
		DynamicDescriptor<DescriptorHeapType::DSV, LockPolicy> dsv_cpu;
		DynamicDescriptor<DescriptorHeapType::SAMPLER, LockPolicy> smp_cpu;

		//	std::shared_ptr<cb_buffer> cb;

		using Uploader::place_raw;

		void reset()
		{
			srv.reset();
			smp.reset();

			srv_uav_cbv_cpu.reset();
			rtv_cpu.reset();
			dsv_cpu.reset();
			smp_cpu.reset();
			Uploader::reset();

		}

	};



	class StaticCompiledGPUData :public Singleton<StaticCompiledGPUData>, public GPUCompiledManager<Lockable>
	{

	};
	class FrameResources :public std::enable_shared_from_this<FrameResources>, public GPUCompiledManager<Lockable>
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

		std::shared_ptr<CommandList> start_list(std::string name = "");

	};

	class Finalizer
	{
		std::function<void()> f;
	public:
		Finalizer(std::function<void()> f) :f(f) {};

		~Finalizer()
		{
			f();
		}
	};


	class FrameResourceManager :public Singleton<FrameResourceManager>
	{
		std::atomic_size_t frame_number = 0;

	public:
		FrameResources::ptr begin_frame();
	};
	/*
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
	//		if (info.frame != manager.get_frame())
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
		//	if (info.frame != manager.get_frame())
				info = manager.set_data(_data.data(), _data.size());

			assert(info.resource);
			m.unlock();
			return info;
		}

		using type = T;
	};
	*/
	class CommandListBase
	{

	protected:
		int id = -1;
		std::uint64_t global_id;
		bool executed = false;
		CommandListType type;

		LEAK_TEST(CommandListBase)

			std::vector<std::function<void()>> on_execute_funcs;
		ComPtr<ID3D12GraphicsCommandList4> m_commandList;
	public:
		ComPtr<ID3D12GraphicsCommandList4>& get_native_list()
		{
			return m_commandList;
		}
		virtual ~CommandListBase() = default;
	};

	class TransitionCommandList;

	class Transitions : public virtual CommandListBase
	{
		std::vector<D3D12_RESOURCE_BARRIER> transitions;
		//unsigned int transition_count = 0;

		std::list<Resource*> used_resources;
		std::shared_ptr<TransitionCommandList> transition_list;
	protected:
		void reset();

		
	public:
		void flush_transitions();

		void transition(const Resource* resource, unsigned int state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		void transition(const Resource::ptr& resource, unsigned int state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		void transition(const std::shared_ptr<Texture>& resource, unsigned int state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);





	public:

		void transition_uav(Resource* resource);
		void transition(Resource* from, Resource* to);
		std::shared_ptr<TransitionCommandList> fix_pretransitions();

		void transition_rtv(const ResourceInfo* info)
		{

			auto& desc = info->resource_ptr->get_desc();
			if (info->rtv.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE2D)
			{
				if (desc.MipLevels == 1 && desc.Depth() == 1)
				{
					transition(info->resource_ptr, ResourceState::RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				
			}
			else 	if (info->rtv.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE2DARRAY)
			{

				auto& rtv = info->rtv.Texture2DArray;

				for (int array = rtv.FirstArraySlice; array < rtv.FirstArraySlice + rtv.ArraySize; array++)
				{
					UINT res = desc.CalcSubresource(rtv.MipSlice, array, rtv.PlaneSlice);
					transition(info->resource_ptr, ResourceState::RENDER_TARGET, res);
				}
			}
		
		}

		void transition_uav(const ResourceInfo* info)
		{


			auto& desc = info->resource_ptr->get_desc();

			if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_BUFFER)
			{
				transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				if (info->uav.counter) transition(info->uav.counter, ResourceState::UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

			}
			else if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE2D)
			{
				auto& uav = info->uav.desc.Texture2D;

				if (desc.MipLevels == 1 && desc.Depth() == 1)
				{
					transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					UINT res = desc.CalcSubresource(uav.MipSlice, 0, uav.PlaneSlice);
					transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, res);
				}
			}
			else if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE3D)
			{
				auto& uav = info->uav.desc.Texture3D;
				if (uav.FirstWSlice == 0 && uav.WSize == desc.Depth() && desc.MipLevels == 1)
				{
					transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					UINT res = desc.CalcSubresource(uav.MipSlice, 0, 0);
					transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, res);
				}
			}

			else assert(false);

		}


		void transition_dsv(const ResourceInfo* info)
		{

			auto& desc = info->resource_ptr->get_desc();
			auto& dsv = info->dsv.Texture2D;

			UINT res = desc.CalcSubresource(dsv.MipSlice, 0, 0);
			transition(info->resource_ptr, ResourceState::DEPTH_WRITE, res);

		}


		void transition_srv(const ResourceInfo* info)
		{

			auto& desc = info->resource_ptr->get_desc();
			UINT total = desc.CalcSubresource(desc.MipLevels - 1, desc.ArraySize() - 1, desc.Depth() - 1);


			if (info->srv.ViewDimension == D3D12_UAV_DIMENSION_BUFFER)
			{
				transition(info->resource_ptr, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NON_PIXEL_SHADER_RESOURCE, 0);// D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

			}
			else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D)
			{
				auto& srv = info->srv.Texture2D;

				if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0 && desc.Depth() == 1)
				{
					transition(info->resource_ptr, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
					{
						UINT res = desc.CalcSubresource(mip, 0, srv.PlaneSlice);
						transition(info->resource_ptr, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NON_PIXEL_SHADER_RESOURCE, res);
					}
				}

			}
			else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2DARRAY)
			{
				auto& srv = info->srv.Texture2DArray;

				if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0 && srv.FirstArraySlice == 0 && srv.ArraySize == desc.ArraySize() && desc.Depth() == 1)
				{
					transition(info->resource_ptr, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
						for (auto array = srv.FirstArraySlice; array < srv.FirstArraySlice + srv.ArraySize; array++)
						{
							UINT res = desc.CalcSubresource(mip, array, srv.PlaneSlice);
							transition(info->resource_ptr, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NON_PIXEL_SHADER_RESOURCE, res);
						}
				}
			}
			else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE3D)
			{
				auto& srv = info->srv.Texture3D;

				if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0)
				{
					transition(info->resource_ptr, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
					{
						UINT res = desc.CalcSubresource(mip, 0, 0);
						transition(info->resource_ptr, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NON_PIXEL_SHADER_RESOURCE, res);
					}
				}
			}
			else
				assert(false);




		}

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

		std::chrono::time_point<std::chrono::high_resolution_clock>  start_time;
		std::chrono::time_point<std::chrono::high_resolution_clock>  end_time;

	};

	class GPUBlock :public TimedBlock
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
		void begin(std::string name, Timer* t = nullptr);
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
			for (auto&& e : on_fence)
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




	class CommandList : public std::enable_shared_from_this<CommandList>, public Readbacker, public Transitions, public Eventer, public Sendable, public GPUCompiledManager<Free>
	{


	public:
		using ptr = shared_ptr<CommandList>;
		std::map<Resource*, int> resource_update_counter;
	protected:
		CommandList() = default;
		friend class Queue;
		friend class FrameResourceManager;
		friend class CopyContext;
		friend class GraphicsContext;
		friend class ComputeContext;

		ComPtr<ID3D12CommandAllocator> m_commandAllocator;

		// TODO: make references?

		virtual void on_execute() override;

		bool heaps_changed = false;

		std::array<DescriptorHeap::ptr, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> heaps;

		std::unique_ptr<GraphicsContext> graphics;
		std::unique_ptr<ComputeContext> compute;
		std::unique_ptr<CopyContext> copy;
		PipelineStateBase* current_pipeline;

		void set_pipeline_internal(PipelineStateBase* pipeline);

		void set_heap(DescriptorHeapType type, DescriptorHeap::ptr heap)
		{
			assert(this->type != CommandListType::COPY);
			auto i_type = static_cast<int>(type);
			if (heaps[i_type] == heap) return;

			heaps_changed = true;
			heaps[i_type] = heap;
			flush_heaps();

		}

	public:
		ptr get_sub_list();
		FrameResources::ptr frame_resources;

		//	DynamicDescriptor<DescriptorHeapType::CBV_SRV_UAV, Free, DescriptorHeapFlags::SHADER_VISIBLE> srv_descriptors;
		//	DynamicDescriptor<DescriptorHeapType::SAMPLER, Free, DescriptorHeapFlags::SHADER_VISIBLE> smp_descriptors;

		GraphicsContext& get_graphics();
		ComputeContext& get_compute();
		CopyContext& get_copy();



		void flush_heaps(bool force = false)
		{
			assert(type != CommandListType::COPY);
			if (!heaps_changed && !force) return;
			heaps_changed = false;
			std::array<ID3D12DescriptorHeap*, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES - 1> native_heaps;

			int count = 0;
			for (auto& e : heaps)
			{
				if (!e) continue;
				native_heaps[count] = e->get_native().Get();
				count++;
			}
			if (count)
			{
				if (count == 2 && native_heaps[1] == native_heaps[0])
					return;
				m_commandList->SetDescriptorHeaps(count, native_heaps.data());
			}

		}
		FrameResources* get_manager()
		{
			return frame_resources.get();
		}

		ptr get_ptr()
		{
			return std::static_pointer_cast<CommandList>(shared_from_this());
		}
		CommandList(CommandListType);

		void begin(std::string name = "", Timer* t = nullptr);
		void end();

		int get_id()
		{
			return id;
		}
		int get_global_id()
		{
			return global_id;
		}
		/*
		void set_my_heap()
		{
			set_heap(DescriptorHeapType::CBV_SRV_UAV, srv_descriptors.get_heap());
			set_heap(DescriptorHeapType::SAMPLER, smp_descriptors.get_heap());

		}*/
		/*
		HandleTable place_srv(UINT count)
		{
			return srv_descriptors.place(count);
		}
		*/
		template<class T>
		void clear_uav(T& resource, const Handle& h, ivec4 ClearColor = ivec4(0, 0, 0, 0))
		{
			transition_uav(h.resource_info);

			flush_transitions();
			auto handle = srv.place(h);
			get_native_list()->ClearUnorderedAccessViewUint(handle.gpu, h.cpu, resource->get_native().Get(), reinterpret_cast<UINT*>(ClearColor.data()), 0, nullptr);
		}

		template<class T>
		void clear_uav(T& resource, const Handle& h, vec4 ClearColor)
		{
			transition_uav(h.resource_info);

			flush_transitions();
			auto handle = srv.place(h);
			get_native_list()->ClearUnorderedAccessViewFloat(handle.gpu, h.cpu, resource->get_native().Get(), reinterpret_cast<FLOAT*>(ClearColor.data()), 0, nullptr);
		}


		void clear_rtv(const Handle& h, vec4 ClearColor = vec4(0, 0, 0, 0))
		{
			transition_rtv(h.resource_info);
			flush_transitions();
			get_native_list()->ClearRenderTargetView(h.cpu, ClearColor.data(), 0, nullptr);
		}

		template<class T>
		void clear_counter(std::shared_ptr<StructuredBuffer<T>>& buffer)
		{
			clear_uav(buffer->help_buffer, buffer->counted_uav[0]);
		}


		void clear_stencil(Handle dsv, UINT8 stencil = 0)
		{
			transition_dsv(dsv.resource_info);
			flush_transitions();
			get_native_list()->ClearDepthStencilView(dsv.cpu, D3D12_CLEAR_FLAG_STENCIL, 0, stencil, 0, nullptr);
		}

		void clear_depth(Handle dsv, float depth = 0)
		{
			transition_dsv(dsv.resource_info);
			flush_transitions();
			get_native_list()->ClearDepthStencilView(dsv.cpu, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
		}


	};

	class CopyContext
	{
		friend class CommandList;

		CommandList& base;

		CopyContext(CommandList& base) :base(base) {}
		CopyContext(const CopyContext&) = delete;
		CopyContext(CopyContext&&) = delete;
	public:
		void copy_resource(Resource* dest, Resource* source);
		void copy_resource(const Resource::ptr& dest, const Resource::ptr& source);
		void copy_texture(const Resource::ptr& dest, int, const Resource::ptr& source, int);
		void copy_texture(const Resource::ptr& dest, ivec3, const Resource::ptr& source, ivec3, ivec3);
		void copy_buffer(Resource* dest, int s_dest, Resource* source, int s_source, int size);


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



	class SignatureDataSetter
	{


		template<class T>
		void set_constants_internal(UINT i, UINT offset, T args)
		{
			set_constant(i, offset, *reinterpret_cast<UINT*>(&args));
		}

		template<class T, class... Args>
		void set_constants_internal(UINT i, UINT offset, T v, Args...args)
		{
			set_constants_internal(i, offset, v);
			set_constants_internal(i, offset + 1, args...);
		}
	protected:
		CommandList& base;
		SignatureDataSetter(CommandList& base) :base(base) {	}
	public:
		bool use_dynamic = true;
		CommandList& get_base() {
			return base;
		}
		virtual void set_signature(const RootSignature::ptr&) = 0;

		virtual void set(UINT, const HandleTable&) = 0;
		virtual void set(UINT, const HandleTableLight&) = 0;
		virtual void set(UINT, const Handle&) = 0;

		virtual void set_uav(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&) = 0;


		virtual void set_const_buffer(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS&) = 0;
		virtual	void set_const_buffer(UINT i, std::shared_ptr<GPUBuffer>& buff) = 0;
		virtual void set_const_buffer(UINT i, const FrameResources::UploadInfo& info) = 0;
		//virtual void set_const_buffer(UINT i, FrameResource& info) = 0;

		virtual void set_constant(UINT i, UINT offset, UINT data) = 0;
		template<class... Args>
		void set_constants(UINT i, Args...args)
		{
			set_constants_internal(i, 0, args...);
		}



		//	virtual void set_srv(UINT i, FrameResource& info) = 0;
		virtual void set_srv(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&) = 0;


		template<class T>
		void set_const_buffer_raw(UINT i, const T* data, UINT64 size)
		{
			//       assert(Math::IsAligned(data.data(), 16));
			//  size_t BufferSize = Math::AlignUp(sizeof(T), 16);
			auto info = base.place_data(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			memcpy(info.resource->get_data() + info.offset, data, size);
			set_const_buffer(i, info.resource->get_gpu_address() + info.offset);
		}


		template<class T>
		void set_const_buffer_raw(UINT i, const T& data)
		{
			//       assert(Math::IsAligned(data.data(), 16));
			//  size_t BufferSize = Math::AlignUp(sizeof(T), 16);
			auto info = base.place_data(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			memcpy(info.resource->get_data() + info.offset, &data, sizeof(T));
			set_const_buffer(i, info.resource->get_gpu_address() + info.offset);
		}


		template<class T>
		void set_srv(UINT i, const std::vector<T>& data)
		{
			size_t BufferSize = Math::AlignUp(data.size() * sizeof(T), 16);
			auto info = base.place_data(BufferSize);
			memcpy(info.resource->get_data() + info.offset, data.data(), data.size() * sizeof(T));
			set_srv(i, info.resource->get_gpu_address() + info.offset);
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
		ComPtr<ID3D12GraphicsCommandList4>& list;

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

		//	std::shared_ptr<PipelineState> current_pipeline;
		std::shared_ptr<RootSignature> current_root_signature;

		//	DynamicDescriptor<DescriptorHeapType::RTV> rtv_descriptors;
		//	DynamicDescriptor<DescriptorHeapType::DSV> dsv_descriptors;

		MyVariant current_shader_data;

		void begin();
		void end();
		void on_execute();
	public:

		void set_srv(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&)override;
		using SignatureDataSetter::set_srv;


		void set_uav(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&)override;


		void set_const_buffer(UINT i, std::shared_ptr<GPUBuffer>& buff) override;
		void set_const_buffer(UINT i, const FrameResources::UploadInfo& info) override;
		//	void set_const_buffer(UINT i, FrameResource& info);
		using SignatureDataSetter::set_const_buffer;
		void set_const_buffer(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&)override;


		void set_constant(UINT i, UINT offset, UINT data)
		{
			list->SetGraphicsRoot32BitConstant(i, data, offset);
		}


		void set(UINT, const HandleTable&)override;
		void set(UINT, const Handle&)override;
		void set(UINT, const HandleTableLight&)override;


	public:

		CommandList& get_base()
		{
			return base;
		}

		ComPtr<ID3D12GraphicsCommandList4>& get_native_list()
		{
			return list;
		}
		void flush_binds(bool force = false);
		void set_topology(D3D_PRIMITIVE_TOPOLOGY topology)
		{
			if (this->topology != topology)
				list->IASetPrimitiveTopology(topology);

			this->topology = topology;
		}




		template<class T>
		typename T::Signature& set_signature_typed(typename RootSignatureTyped<T>::ptr& s)
		{

			if (current_root_signature != s)
			{
				base.get_native_list()->SetGraphicsRootSignature(s->get_native().Get());
				current_root_signature = s;
				current_shader_data.create<typename T::Signature>(this);// = std::any(std::in_place_type, this);// std::make_any<typename T::Signature>(this);// .emplace<typename T::Signature>(this);
			}

			return get_shader_data<T>();
		}

		template<class T>
		typename T::Signature& set_signature_typed(const RootSignature::ptr& s)
		{

			if (current_root_signature != s)
			{
				base.get_native_list()->SetGraphicsRootSignature(s->get_native().Get());
				current_root_signature = s;
				current_shader_data.create<typename T::Signature>(this);// = std::any(std::in_place_type, this);// std::make_any<typename T::Signature>(this);// .emplace<typename T::Signature>(this);
			}

			return get_shader_data<T>();
		}


		template<class T>
		typename T::Signature& get_shader_data()
		{
			return  current_shader_data.get< T::Signature>();
		}


		template<class T>
		typename T::Signature& set_pipeline_typed(std::shared_ptr<PipelineStateTyped<T>>& state)
		{
			set_signature_typed<T>(state->desc.root_signature);
			base.set_pipeline_internal(state.get());

			return get_shader_data<T>();
		}


		void set_signature(const RootSignature::ptr&) override;
		void set_pipeline(std::shared_ptr<PipelineState>&);




		void set_layout(Layouts layout);

		void set_heaps(DescriptorHeap::ptr& a, DescriptorHeap::ptr& b);

		void set_scissor(sizer_long rect);
		void set_viewport(Viewport viewport);

		void set_viewport(vec4 viewport);

		void set_scissors(sizer_long rect);
		void set_viewports(std::vector<Viewport> viewports);



		void set_rtv(const HandleTable&, Handle);
		void set_rtv(int c, Handle rt, Handle h);

		void set_rtv(const HandleTableLight&, Handle);

		void draw(D3D12_DRAW_INDEXED_ARGUMENTS args)
		{
			draw_indexed(args.IndexCountPerInstance, args.StartIndexLocation, args.BaseVertexLocation, args.InstanceCount, args.StartInstanceLocation);
		}
		/*
				HandleTable place_uav(UINT count)
				{
					return uav_descriptors.place(count);
				}
				*/

		HandleTableLight place_rtv(UINT count)
		{
			return get_base().rtv_cpu.place(count);
		}

		HandleTableLight place_dsv(UINT count)
		{
			return get_base().dsv_cpu.place(count);
		}

		HandleTableLight place_rtv(std::initializer_list<Handle> list)
		{
			return get_base().rtv_cpu.place(list);
		}

		HandleTableLight place_dsv(std::initializer_list<Handle> list)
		{
			return get_base().dsv_cpu.place(list);
		}

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
			list->IASetVertexBuffers(offset, count, views);
		}

		void set_vertex_buffers(UINT offset, std::vector<D3D12_VERTEX_BUFFER_VIEW>& data)
		{
			list->IASetVertexBuffers(offset, static_cast<UINT>(data.size()), data.data());
		}

		void set_vertex_buffer(UINT offset, D3D12_VERTEX_BUFFER_VIEW& data)
		{
			list->IASetVertexBuffers(offset, 1, &data);
		}

		void set_index_buffer(D3D12_INDEX_BUFFER_VIEW view)
		{
			list->IASetIndexBuffer(&view);
		}

		template<class T>
		D3D12_VERTEX_BUFFER_VIEW place_vertex_buffer(const std::vector<T>& data)
		{
			//       assert(Math::IsAligned(data.data(), 16));
			size_t BufferSize = Math::AlignUp(data.size() * sizeof(T), 16);
			auto info = base.place_data(BufferSize);
			memcpy(info.resource->get_data() + info.offset, data.data(), data.size() * sizeof(T));
			D3D12_VERTEX_BUFFER_VIEW res;
			res.SizeInBytes = static_cast<UINT>(BufferSize);
			res.StrideInBytes = sizeof(T);
			res.BufferLocation = info.resource->get_gpu_address() + info.offset;
			return res;
		}
		template<class T>
		D3D12_VERTEX_BUFFER_VIEW place_vertex_buffer(const T* data, size_t size)
		{
			//       assert(Math::IsAligned(data.data(), 16));
			auto info = base.place_data(size * sizeof(T));
			memcpy(info.resource->get_data() + info.offset, data, size * sizeof(T));
			D3D12_VERTEX_BUFFER_VIEW res;
			res.SizeInBytes = static_cast<UINT>(size * sizeof(T));
			res.StrideInBytes = sizeof(T);
			res.BufferLocation = info.resource->get_gpu_address() + info.offset;
			return res;
		}



		void draw(UINT vertex_count, UINT vertex_offset = 0, UINT instance_count = 1, UINT instance_offset = 0);
		void draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count = 1, UINT instance_offset = 0);



		void execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset = 0, Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);

	};



	class ComputeContext : public SignatureDataSetter
	{
		friend class CommandList;


		ComPtr<ID3D12GraphicsCommandList4>& list;

		ComputeContext(CommandList& base) :SignatureDataSetter(base), list(base.get_native_list()) {}
		ComputeContext(const ComputeContext&) = delete;
		ComputeContext(ComputeContext&&) = delete;


		//	std::shared_ptr<ComputePipelineState> current_compute_pipeline;
		std::shared_ptr<RootSignature> current_compute_root_signature;

		void begin();
		void end();
		void on_execute();



		void set(UINT, const HandleTable&);
		void set(UINT, const HandleTableLight&)override;


		void set_table(UINT, const Handle&);

		void set_const_buffer(UINT i, std::shared_ptr<GPUBuffer> buff);
		void set_const_buffer(UINT i, const FrameResources::UploadInfo& info);
		//	void set_const_buffer(UINT i, FrameResource& info);

		void set_srv(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&);
		void set_uav(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&);
		using SignatureDataSetter::set_srv;
		using SignatureDataSetter::set_const_buffer;


		virtual void set(UINT, const Handle&) override;
		virtual void set_const_buffer(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&) override;
		virtual void set_const_buffer(UINT i, std::shared_ptr<GPUBuffer>& buff) override;
		//		virtual void set_srv(UINT i, FrameResource& info) override;

		MyVariant current_shader_data;

	public:

		void set_constant(UINT i, UINT offset, UINT data)
		{
			list->SetComputeRoot32BitConstant(i, data, offset);
		}

		CommandList& get_base()
		{
			return base;
		}

		void flush_binds(bool force = false);
		void set_signature(const RootSignature::ptr&);
		void set_pipeline(std::shared_ptr<ComputePipelineState>&);




		template<class T>
		typename T::Signature& set_signature_typed(typename RootSignatureTyped<T>::ptr& s)
		{

			if (current_compute_root_signature != s)
			{
				list->SetComputeRootSignature(s->get_native().Get());
				current_compute_root_signature = s;
				current_shader_data.create<typename T::Signature>(this);// = std::any(std::in_place_type, this);// std::make_any<typename T::Signature>(this);// .emplace<typename T::Signature>(this);
			}

			return get_shader_data<T>();
		}

		template<class T>
		typename T::Signature& get_shader_data()
		{
			return  current_shader_data.get< T::Signature>();
		}

		void dispach(int = 1, int = 1, int = 1);
		void dispach(ivec2, ivec2 = ivec2(8, 8));
		void dispach(ivec3, ivec3 = ivec3(4, 4, 4));




		void execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset = 0, Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);


	};


	class TransitionCommandList
	{
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;

	public:
		using ptr = std::shared_ptr<TransitionCommandList>;

		TransitionCommandList(CommandListType type);
		void create_transition_list(const std::vector<D3D12_RESOURCE_BARRIER>& transitions);
		ComPtr<ID3D12GraphicsCommandList> get_native();
	};
}
