#pragma once
namespace DX12
{
	enum class HeapType : int
	{
		DEFAULT = D3D12_HEAP_TYPE_DEFAULT,
		UPLOAD = D3D12_HEAP_TYPE_UPLOAD,
		READBACK = D3D12_HEAP_TYPE_READBACK,
		CUSTOM = D3D12_HEAP_TYPE_CUSTOM,
		PRESENT = 5
	};

	enum ResourceState
	{
		COMMON = D3D12_RESOURCE_STATE_COMMON,
		VERTEX_AND_CONSTANT_BUFFER = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		INDEX_BUFFER = D3D12_RESOURCE_STATE_INDEX_BUFFER,
		RENDER_TARGET = D3D12_RESOURCE_STATE_RENDER_TARGET,
		UNORDERED_ACCESS = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		DEPTH_WRITE = D3D12_RESOURCE_STATE_DEPTH_WRITE,
		DEPTH_READ = D3D12_RESOURCE_STATE_DEPTH_READ,
		NON_PIXEL_SHADER_RESOURCE = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		PIXEL_SHADER_RESOURCE = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		STREAM_OUT = D3D12_RESOURCE_STATE_STREAM_OUT,
		INDIRECT_ARGUMENT = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
		COPY_DEST = D3D12_RESOURCE_STATE_COPY_DEST,
		COPY_SOURCE = D3D12_RESOURCE_STATE_COPY_SOURCE,
		RESOLVE_DEST = D3D12_RESOURCE_STATE_RESOLVE_DEST,
		RESOLVE_SOURCE = D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		GEN_READ = D3D12_RESOURCE_STATE_GENERIC_READ,
		PRESENT = D3D12_RESOURCE_STATE_PRESENT,
		PREDICATION = D3D12_RESOURCE_STATE_PREDICATION,
		RAYTRACING_STRUCTURE = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		UNKNOWN = -1,
		DIFFERENT = -2
	};


	class ResourceStateManager
	{

		struct ResourceListStateCPU
		{
			unsigned int first_state = ResourceState::UNKNOWN;
			unsigned int state = ResourceState::UNKNOWN;

			uint64_t command_list_id = -1;
		};

		struct SubResourcesCPU
		{
			std::vector<ResourceListStateCPU> subres;

			ResourceListStateCPU all;

			volatile uint64_t command_list_id = -1;
		};


		struct ResourceListStateGPU
		{
			unsigned int state = ResourceState::UNKNOWN;
		};

		struct SubResourcesGPU
		{
			std::vector<ResourceListStateGPU> subres;

			ResourceListStateGPU all;
		};


		mutable  SpinLock states_lock;

		mutable std::map<int,SubResourcesCPU> cpu_state;

		SubResourcesCPU& get_state(int id) const
		{
			std::lock_guard<SpinLock> guard(states_lock);

			auto it = cpu_state.find(id);

			if (it == cpu_state.end())
			{
				auto & state = cpu_state[id];
				state.subres.resize(gpu_state.subres.size());

				return state;
			}

			return it->second;

		}



	protected:
		SubResourcesGPU gpu_state;

	public:

		using ptr = std::unique_ptr<ResourceStateManager>;

		ResourceStateManager()
		{
			//cpu_state.resize(64);
		}

		void init_subres(int count, ResourceState state)
		{
			gpu_state.subres.resize(count);
			gpu_state.all.state = state;

			for (auto& e : gpu_state.subres)
				e.state = state;
		}

		int get_cpu_state(int id, uint64_t full_id)
		{
			auto& state = get_state(id);

			return state.subres[0].state;
		}
		/*
		unsigned int gpu_transition(unsigned int state)
		{
			if (gpu_state == state) return ResourceState::UNKNOWN;
			unsigned int result = gpu_state;

			gpu_state = state;
			return result;
		}
		*/
		
		bool is_new(int id, uint64_t full_id) const
		{
			SubResourcesCPU& s = get_state(id);

			bool result = (s.command_list_id != full_id);

			s.command_list_id = full_id;

			return  result;
		}

		
		void process_transitions(std::vector<D3D12_RESOURCE_BARRIER>& target, const Resource* resource, int id, uint64_t full_id);

		void transition(std::vector<D3D12_RESOURCE_BARRIER>& target,const  Resource* resource, unsigned int state, unsigned int subres, int id, uint64_t full_id) const;

	};

	class ResourceHeap
	{
	public:
		ComPtr<ID3D12Heap > heap;
		HeapType type;
		D3D12_HEAP_FLAGS flags;

		size_t heap_size = 0;
		using ptr = std::shared_ptr<ResourceHeap>;

		ResourceHeap(ResourceHeap& other) = default;

		ResourceHeap(HeapType type, D3D12_HEAP_FLAGS flags) :type(type), flags(flags)
		{

		}

		ResourceHeap(size_t size, HeapType type, D3D12_HEAP_FLAGS flags) :type(type), flags(flags)
		{
			init(size);
		}
		virtual ~ResourceHeap() = default;
	protected:
		void init(size_t size);
	};
	
	template<class AllocatorType>
	struct ResourceHeapPage: AllocatorType, ResourceHeap
	{
		using ptr = std::shared_ptr<ResourceHeapPage>;
		ResourceHeapPage(size_t size, HeapType type, D3D12_HEAP_FLAGS flags) :ResourceHeap(size,type,flags), AllocatorType(size)
		{

		}
		ResourceHeapPage(ResourceHeapPage& heap) :ResourceHeap(heap), AllocatorType(size)
		{

		}
		/*ResourceHeap::ptr m_heap;
	
		ResourceHeapPage(ResourceHeap::ptr heap): m_heap(heap), AllocatorType(m_heap->heap_size)
		{
		
		}*/

		/*
		Allocator::Handle Allocate(size_t size, size_t alignment)
		{
			return allocator.Allocate(size, alignment);
		}*/
	};

	/*
	template<class AllocatorType>
	struct ResourceHeapPage : AllocatorType
	{
		Allocator::Handle handle;
		ResourceHeap::ptr m_heap;

		ResourceHeapPage(ResourceHeap::ptr heap, Allocator::Handle handle) :handle(handle), m_heap(heap), AllocatorType(m_heap->heap_size)
		{

		}

	};
	*/

	class Resource;


	struct ResourceAddress
	{
		ResourceAddress()
		{
			address = 0;
		resource = nullptr;

		}

		ResourceAddress(D3D12_GPU_VIRTUAL_ADDRESS address, Resource* resource):address(address), resource(resource)
		{

		}

		D3D12_GPU_VIRTUAL_ADDRESS address = 0;
		Resource* resource = nullptr;

		explicit operator bool() const
		{
			return address;
		}

		operator D3D12_GPU_VIRTUAL_ADDRESS() const
		{
			return address;
		}
	};
	

	// for tiles now, only
	class ResourceHeapPageManager : public Singleton<ResourceHeapPageManager>
	{
		using heap_list = std::queue<ResourceHeapPage<CommonAllocator>::ptr>;
		using flags_map = std::map<D3D12_HEAP_FLAGS, heap_list>;
		flags_map all_heaps;
		std::mutex m;

		ResourceHeapPage<CommonAllocator>::ptr current_heap;

		std::set<ResourceHeapPage<CommonAllocator>::ptr> total_heaps;

		ResourceHeapPage<CommonAllocator>::ptr AllocateHeap(/*size_t size, HeapType type,*/ D3D12_HEAP_FLAGS flags)
		{
		
			auto heaps = all_heaps[flags];

			if (heaps.size())
			{
				auto top = heaps.front();
				heaps.pop();
				return top;
			}


			auto res = std::make_shared<ResourceHeapPage<CommonAllocator>>(64 * 1024, HeapType::DEFAULT, flags);
			total_heaps.insert(res);
			return res;
		}
	public:
		
		TileHeapPosition create_tile(D3D12_HEAP_FLAGS flags)
		{
			std::lock_guard<std::mutex> g(m);


			if (!current_heap )
			{
				current_heap = AllocateHeap(flags);
			}

			auto handle = current_heap->TryAllocate(64 * 1024);

			if (!handle)
			{
				current_heap = AllocateHeap(flags);
				handle = current_heap->TryAllocate(64 * 1024);
			}

			TileHeapPosition result;

			result.offset = handle->get_offset()/ (64 * 1024);
			result.heap = current_heap.get();

			return result;
		}
	};

	template<class AllocatorType>
	struct ResourceHeapAllocator : public ResourceHeap, public AllocatorType
	{
		using ResourceHeap::ResourceHeap;

		void allocate_for_allocator(bool reset = false)
		{
			auto max_usage = AllocatorType::get_max_usage();
			if (max_usage > heap_size)
			{
				ResourceHeap::init(max_usage);
			}
		}
	};


	class Resource;
	using Resource_ptr = std::shared_ptr<Resource>;

	class ResourceAllocator
	{

	public:
		virtual HeapType get_type() = 0;
		virtual Resource_ptr create_resource(const CD3DX12_RESOURCE_DESC& desc, ResourceState state, vec4 clear_value) = 0;
	};

	class DefaultAllocator :public Singleton<DefaultAllocator>, public ResourceAllocator
	{
		virtual HeapType get_type()override;
	public:
		virtual Resource_ptr create_resource(const CD3DX12_RESOURCE_DESC& desc, ResourceState state, vec4 clear_value)  override;
	};

	class ReservedAllocator :public Singleton<ReservedAllocator>, public ResourceAllocator
	{
		virtual HeapType get_type()override;
	public:
		virtual Resource_ptr create_resource(const CD3DX12_RESOURCE_DESC& desc, ResourceState state, vec4 clear_value)  override;
	};


	class FrameResources;

	class Resource :public std::enable_shared_from_this<Resource>, public ResourceStateManager, public TiledResourceManager
	{
		LEAK_TEST(Resource)

			CD3DX12_RESOURCE_DESC desc;
		bool force_delete = false;
		D3D12_GPU_VIRTUAL_ADDRESS gpu_adress;
		HeapType heap_type = HeapType::CUSTOM;
		//  std::vector< unsigned int> states;
		size_t id = 0;

		CommonAllocator::Handle alloc_handle;

	protected:
		ComPtr<ID3D12Resource> m_Resource;
		void init(const CD3DX12_RESOURCE_DESC& desc, ResourceAllocator& heap = DefaultAllocator::get(), ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
	public:
		std::byte* buffer_data = nullptr;
		bool debug = false;
		std::string name;
		void set_name(std::string name)
		{
			this->name = name;
			//	auto& timer = Profiler::get().start(L"set_name");
			m_Resource->SetName(convert(name).c_str());
		}
		const CD3DX12_RESOURCE_DESC &get_desc() const
		{
			return desc;
		}
		using ptr = std::shared_ptr<Resource>;
	//	Resource::ptr delete_me;
		Resource(const CD3DX12_RESOURCE_DESC& desc, ResourceAllocator& heap = DefaultAllocator::get(), ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
		Resource(const ComPtr<ID3D12Resource>& resouce, ResourceState state, bool own = false);
		Resource(const ComPtr<ID3D12Resource>& resouce, CommonAllocator::Handle, ResourceState state);

		Resource():TiledResourceManager(m_Resource)
		{
			//  states.resize(50);
		}
		virtual ~Resource();

		HeapType get_heap_type() const
		{
			return heap_type;
		}

		bool is_placed()
		{
			return alloc_handle;
		}
		void release_memory()
		{
//			alloc_handle.Free();
		}

		ComPtr<ID3D12Resource> get_native() const
		{
			return m_Resource;
		}

		D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address() const
		{
			return gpu_adress;
		}

		ResourceAddress get_resource_address() 
		{
			assert(gpu_adress > 0);
			return ResourceAddress( gpu_adress , this);
		}

		// TODO:: works only for buffer now
		auto get_size()
		{
			return desc.Width;
		}

		ptr get_ptr()
		{
			return shared_from_this();
		}

		template<class T, class ...Args>
		typename T create_view(FrameResources& frame, Args ...args)
		{
			return T(shared_from_this(), frame, args...);
		}
	};



	class UploadAllocator :public Singleton<UploadAllocator>, public ResourceAllocator
	{
		virtual HeapType get_type()override;
	public:	virtual Resource::ptr create_resource(const CD3DX12_RESOURCE_DESC& desc, ResourceState state, vec4 clear_value)  override;
	};

	class ReadbackAllocator :public Singleton<ReadbackAllocator>, public ResourceAllocator
	{
		virtual HeapType get_type()override;
	public:	virtual Resource::ptr create_resource(const CD3DX12_RESOURCE_DESC& desc, ResourceState state, vec4 clear_value)  override;
	};


	class PlacedAllocator
	{
	public:
		ResourceHeapAllocator<CommonAllocator> heap_srv;
		ResourceHeapAllocator<CommonAllocator> heap_rtv;
		ResourceHeapAllocator<CommonAllocator> heap_uav;


		struct FrameHeaps
		{
			ResourceHeapAllocator<LinearAllocator> heap_upload_buffer;
			ResourceHeapAllocator<LinearAllocator> heap_upload_texture;
			ResourceHeapAllocator<LinearAllocator> heap_readback;
			FrameHeaps():heap_upload_texture(HeapType::UPLOAD, D3D12_HEAP_FLAG_NONE),
				heap_upload_buffer(HeapType::UPLOAD, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS),
				heap_readback(HeapType::READBACK, D3D12_HEAP_FLAG_NONE)
			{

			}
		};
	
		std::vector<FrameHeaps> frames;
		FrameHeaps* current_frame;
		HeapType get_type();

	
		PlacedAllocator();

		CommonAllocator::Handle allocate_resource( CD3DX12_RESOURCE_DESC& desc, HeapType heap_type);
		Resource::ptr create_resource( CD3DX12_RESOURCE_DESC& desc, CommonAllocator::Handle handle, ResourceState state, vec4 clear_value);
		Resource::ptr create_resource( CD3DX12_RESOURCE_DESC& desc, HeapType heap_type, ResourceState state, vec4 clear_value);

		void begin_frame(int i)
		{
			if (frames.size() <= i)
				frames.resize(i + 1);

			current_frame = &frames[i];
		}

		void allocate_heaps()
		{

			heap_srv.allocate_for_allocator();
			heap_rtv.allocate_for_allocator();
			heap_uav.allocate_for_allocator();
			current_frame->heap_upload_buffer.allocate_for_allocator();
			current_frame->heap_upload_texture.allocate_for_allocator();
			current_frame->heap_readback.allocate_for_allocator();
		}

		void reset()
		{
			heap_srv.Reset();
			heap_rtv.Reset();
			heap_uav.Reset();

			current_frame->heap_upload_buffer.Reset();
			current_frame->heap_upload_texture.Reset();
			current_frame->heap_readback.Reset();
		}
	};




	class ResourceView
	{
	protected:
		Handle srv;
		Handle uav;
		Handle rtv;
		Handle dsv;
		Handle cb;
		Handle uav_clear;
	protected:
		ResourceViewDesc view_desc;

	public:
		Resource::ptr resource; //////////////
		ResourceView()
		{

		}

		auto get_desc()
		{
			return resource->get_desc();
		}

		ResourceView(Resource::ptr resource) :resource(resource)
		{

		}

		void init_desc()
		{
			auto& desc = resource->get_desc();

			view_desc.format = desc.Format;

			if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				view_desc.type = ResourceType::BUFFER;
				view_desc.Buffer.Offset = 0;
				view_desc.Buffer.Size = desc.Width;
				view_desc.Buffer.Stride = 0;
			}

			if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
			{
				view_desc.type = ResourceType::TEXTURE1D;
			}

			if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
			{

				
			
				{
					view_desc.type = ResourceType::TEXTURE2D;
					view_desc.Texture2D.ArraySize = desc.ArraySize();
					view_desc.Texture2D.MipSlice = 0;
					view_desc.Texture2D.FirstArraySlice = 0;
					view_desc.Texture2D.PlaneSlice = 0;
					view_desc.Texture2D.MipLevels = desc.MipLevels;
				}
				




			}

			if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
			{
				view_desc.type = ResourceType::TEXTURE3D;
			}

		
		}
		void init_views(FrameResources& frame);

		virtual void place_srv(Handle& h) { assert(false); }
		virtual void place_uav(Handle& h) { assert(false); }
		virtual void place_rtv(Handle& h) { assert(false); }
		virtual void place_dsv(Handle& h) { assert(false); }
		virtual void place_cb(Handle& h) { assert(false); }


		Handle get_srv() { return srv; }
		Handle get_uav() { return uav; }
		Handle get_uav_clear() { return uav_clear; }
		Handle get_rtv() { return rtv; }
		Handle get_dsv() { return dsv; }
		Handle get_cb() { return cb; }

		operator bool() const
		{
			return !!resource;
		}
	};


	class TextureView :public ResourceView
	{

	public:
		TextureView() = default;
		TextureView(Resource::ptr resource, FrameResources& frame, bool cube = false) :ResourceView(resource)
		{
			init_desc();

			auto& desc = resource->get_desc();

			if (cube&&desc.ArraySize() % 6 == 0)
			{

				view_desc.type = ResourceType::CUBE;
				view_desc.Texture2D.ArraySize = desc.ArraySize() / 6;
				view_desc.Texture2D.MipSlice = 0;
				view_desc.Texture2D.FirstArraySlice = 0;
				view_desc.Texture2D.PlaneSlice = 0;
				view_desc.Texture2D.MipLevels = desc.MipLevels;
			}


			init_views(frame);
		}

		TextureView(Resource::ptr resource, FrameResources& frame, ResourceViewDesc desc) :ResourceView(resource)
		{

			view_desc = desc;
			init_views(frame);
		}

		virtual void place_srv(Handle& h) override;
		virtual void place_uav(Handle& h)  override;
		virtual void place_rtv(Handle& h)  override;
		virtual void place_dsv(Handle& h)  override;
		virtual void place_cb(Handle& h) { assert(false); }


		ivec3 get_size()
		{
			auto desc = resource->get_desc();
			UINT scaler = 1 << view_desc.Texture2D.MipSlice;
			return { std::max(1ull,desc.Width / scaler),std::max(1u,desc.Height / scaler) ,std::max(1u,desc.Depth() / scaler) };
		}
		Viewport get_viewport()
		{

			auto desc = resource->get_desc();
			UINT scaler = 1 << view_desc.Texture2D.MipSlice;


			Viewport p;
			p.Width = std::max(1.0f, static_cast<float>(resource->get_desc().Width / scaler));
			p.Height = std::max(1.0f, static_cast<float>(resource->get_desc().Height / scaler));
			p.TopLeftX = 0;
			p.TopLeftY = 0;
			p.MinDepth = 0;
			p.MaxDepth = 1;

			return p;
		}


		sizer_long get_scissor()
		{
			auto desc = resource->get_desc();
			UINT scaler = 1 << view_desc.Texture2D.MipSlice;


			return { 0,0, std::max(1ull,desc.Width / scaler),std::max(1u,desc.Height / scaler) };
		}

		TextureView create_2d_slice(UINT slice, FrameResources & frame)
		{
			ResourceViewDesc desc = view_desc;
			
			desc.Texture2D.ArraySize = 1;
			desc.Texture2D.FirstArraySlice = slice;

			return TextureView(resource, frame, desc);
		}

	};


	class BufferView : public ResourceView
	{

	public:
		BufferView() = default;

		BufferView(Resource::ptr resource, FrameResources& frame) :ResourceView(resource)
		{
			init_desc();
			init_views(frame);
		}

		~BufferView()
		{

		}
		virtual void place_srv(Handle& h) override;
		virtual void place_uav(Handle& h) override;
		virtual void place_rtv(Handle& h) { assert(false); }
		virtual void place_dsv(Handle& h) { assert(false); }
		virtual void place_cb(Handle& h)override;
		template<class T>
		void write(UINT64 offset, T* data, UINT64 count)
		{
			memcpy(resource->buffer_data + offset, data, sizeof(T) * count);
		}

		
	};



template<class T>
class StructuredBufferView :public ResourceView
{
	
public:
	StructuredBufferView() = default;

	StructuredBufferView(Resource::ptr resource, FrameResources& frame, UINT offset = 0, UINT64 size = 0) :ResourceView(resource)
	{
		init_desc();
		view_desc.Buffer.Offset = offset;
		if(size) view_desc.Buffer.Size = size;

		init_views(frame);
	}

	~StructuredBufferView()
	{

	}
	virtual void place_srv(Handle & h) override {
		if (!resource) return;

		D3D12_SHADER_RESOURCE_VIEW_DESC  desc = {};
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = DXGI_FORMAT_UNKNOWN;
	
		desc.Buffer.StructureByteStride = sizeof(T);
		desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		
		if (desc.Buffer.StructureByteStride > 0) {
			desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / desc.Buffer.StructureByteStride);
			desc.Buffer.FirstElement =  view_desc.Buffer.Offset / desc.Buffer.StructureByteStride;

		}
		else {
			desc.Buffer.NumElements = 0;
			desc.Buffer.FirstElement = 0;

		}
		Device::get().create_srv(h, resource.get(), desc);
	

		
	}

	virtual void place_uav(Handle & h) { assert(false); }
	virtual void place_rtv(Handle & h) { assert(false); }
	virtual void place_dsv(Handle & h) { assert(false); }
	virtual void place_cb(Handle& h)override {
		if (!resource) return;

		D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
		desc.BufferLocation = resource->get_gpu_address();
		desc.SizeInBytes = view_desc.Buffer.Size;
		assert(desc.SizeInBytes < 65536);

		Device::get().create_cbv(h, resource.get(), desc);
	}
	void write(UINT64 offset, T * data, UINT64 count)
	{
		memcpy(resource->buffer_data + offset, data, sizeof(T) * count);
	}
};


template<class T, DXGI_FORMAT format>
class FormattedBufferView :public ResourceView
{

public:
	FormattedBufferView() = default;

	FormattedBufferView(Resource::ptr resource, FrameResources& frame, UINT offset = 0, UINT64 size = 0) :ResourceView(resource)
	{
		init_desc();
		view_desc.Buffer.Offset = offset;
		if (size) view_desc.Buffer.Size = size;

		init_views(frame);
	}

	~FormattedBufferView()
	{

	}
	virtual void place_srv(Handle& h) override {
		if (!resource) return;

		D3D12_SHADER_RESOURCE_VIEW_DESC  desc = {};
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = format;

		desc.Buffer.StructureByteStride =0;
		desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;


			desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / sizeof(T));
			desc.Buffer.FirstElement = view_desc.Buffer.Offset / sizeof(T);

		Device::get().create_srv(h, resource.get(), desc);

	
	}

	virtual void place_uav(Handle& h) {
	
		if (!resource) return;

		D3D12_UNORDERED_ACCESS_VIEW_DESC  desc = {};
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	
		desc.Format = format;

		desc.Buffer.StructureByteStride = 0;
		desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;


		desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / sizeof(T));
		desc.Buffer.FirstElement = view_desc.Buffer.Offset / sizeof(T);
		desc.Buffer.CounterOffsetInBytes = 0;

		Device::get().create_uav(h, resource.get(), desc);
		Device::get().create_uav(uav_clear, resource.get(), desc);


	}
	virtual void place_rtv(Handle& h) { assert(false); }
	virtual void place_dsv(Handle& h) { assert(false); }
	virtual void place_cb(Handle& h)override {
		if (!resource) return;

		D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
		desc.BufferLocation = resource->get_gpu_address();
		desc.SizeInBytes = view_desc.Buffer.Size;
		assert(desc.SizeInBytes < 65536);

		Device::get().create_cbv(h, resource.get(), desc);
	}
	void write(UINT64 offset, T* data, UINT64 count)
	{
		memcpy(resource->buffer_data + offset, data, sizeof(T) * count);
	}
};


}