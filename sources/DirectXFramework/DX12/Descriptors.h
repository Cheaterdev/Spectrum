namespace DX12
{



	enum class HandleType : char
	{
		CBV,
		RTV,
		DSV,
		SRV,
		UAV
	};

enum class DescriptorHeapType : char
{
	CBV_SRV_UAV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	SAMPLER = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	RTV = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	DSV = D3D12_DESCRIPTOR_HEAP_TYPE_DSV
};
enum class DescriptorHeapFlags : char
{
	NONE = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	SHADER_VISIBLE = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE

};
class Resource;

enum class ResourceType : char
{

	BUFFER,
	TEXTURE1D,
	//	TEXTURE1DARRAY,
	TEXTURE2D,
	//TEXTURE2DARRAY,
	TEXTURE3D,
	CUBE
};



struct ResourceViewDesc
{
	ResourceType type;
	DXGI_FORMAT format;

	union
	{
		struct
		{

			UINT PlaneSlice;
			UINT MipSlice;
			UINT FirstArraySlice;
			UINT MipLevels;
			UINT ArraySize;
		} Texture2D;

		struct
		{
			UINT64 Size;
			UINT64 Offset;
			UINT64 Stride;
		} Buffer;

	};
};



struct ResourceInfo
{
	Resource* resource_ptr = nullptr;

	HandleType type;

	union
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv;

		struct
		{
			Resource* counter;
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
		} uav;
	
		D3D12_SHADER_RESOURCE_VIEW_DESC srv;
		D3D12_RENDER_TARGET_VIEW_DESC rtv;
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;

	};

	ResourceInfo() = default;

	ResourceInfo(Resource* resource_ptr, D3D12_SHADER_RESOURCE_VIEW_DESC srv):resource_ptr(resource_ptr)
	{
		type = HandleType::SRV;
		this->srv = srv;
	}

	ResourceInfo(Resource* resource_ptr, D3D12_DEPTH_STENCIL_VIEW_DESC dsv) :resource_ptr(resource_ptr)
	{
		type = HandleType::DSV;
		this->dsv = dsv;
	}

	ResourceInfo(Resource* resource_ptr, Resource* counter_ptr, D3D12_UNORDERED_ACCESS_VIEW_DESC uav) :resource_ptr(resource_ptr)
	{
		type = HandleType::UAV;
		this->uav.desc = uav;
		this->uav.counter = counter_ptr;
	}

	ResourceInfo(Resource* resource_ptr, D3D12_RENDER_TARGET_VIEW_DESC rtv) :resource_ptr(resource_ptr)
	{
		type = HandleType::RTV;
		this->rtv = rtv;
	}

	ResourceInfo(Resource* resource_ptr, D3D12_CONSTANT_BUFFER_VIEW_DESC cbv) :resource_ptr(resource_ptr)
	{
		type = HandleType::CBV;
		this->cbv = cbv;
	}

};

struct Handle
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpu;
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpu;

	ResourceInfo* resource_info = nullptr;

	bool is_valid() const
	{
		return cpu.ptr != 0 || gpu.ptr != 0;
	}

	Handle()
	{
		gpu.ptr = 0;
		cpu.ptr = 0;
	}

	void operator=(const std::function<void(Handle&)>& f)
	{
		f(*this);
	}

	bool operator!=(const Handle& r)
	{
		if (cpu.ptr != r.cpu.ptr) return true;

		if (gpu.ptr != r.gpu.ptr) return true;

		return false;
	}

	void place(const Handle& r, D3D12_DESCRIPTOR_HEAP_TYPE t = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) const;

	void clear(CommandList& list, float4 = { 0, 0, 0, 0 }) const;
};

using RTVHandle = Handle;
using DSVHandle = Handle;
using SRVHandle = Handle;
using SMPHandle = Handle;
using UAVHandle = Handle;

class DescriptorHeap;
class CommandList;
struct HandleTable
{
	Handle operator[](UINT i) const
	{
		assert(i < info->count);
		Handle res = get_base();
		res.cpu.Offset(i, info->descriptor_size);
		res.gpu.Offset(i, info->descriptor_size);
		res.resource_info += i;
		return res;
	}

	Handle& get_base() const
	{
		return info->base;
	}

	UINT get_count() const
	{
		return info ? info->count : 0;
	}

	bool valid() const
	{
		return !!info;
	}

	void place(const HandleTable& r, D3D12_DESCRIPTOR_HEAP_TYPE t = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

private:
	friend class DescriptorHeap;

	struct helper
	{
		Handle base;
		UINT descriptor_size;
		UINT count;
	};


	std::shared_ptr<helper> info;
};

struct HandleTableLight : public Handle
{
	Handle operator[](UINT i) const
	{
		assert(i < count);
		Handle res = *this;
		res.cpu.Offset(i, descriptor_size);
		res.gpu.Offset(i, descriptor_size);
		res.resource_info = resource_info + i;
		return res;
	}


	inline UINT get_count() const
	{
		return count;
	}

	bool valid() const
	{
		return descriptor_size > 0;
	}
	 

private:
	friend class DescriptorHeap;

	UINT descriptor_size = 0;
	UINT count = 0;

};
class DescriptorHeap : public std::enable_shared_from_this<DescriptorHeap>
{
	//      std::vector<Handle> handles;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	std::vector<ResourceInfo> resources;

	UINT descriptor_size;
	UINT current_offset = 0;
	UINT max_count;
	std::mutex m;
	std::map<UINT, std::deque<UINT>> frees;
	HandleTable make_table(UINT count, UINT offset)
	{
		HandleTable res;
		std::weak_ptr<DescriptorHeap> ptr = shared_from_this();
		res.info = std::shared_ptr<HandleTable::helper>(new HandleTable::helper, [ptr, count, offset](HandleTable::helper* e)
			{
				auto t = ptr.lock();

				if (t)
					t->frees[count].push_back(offset);

				delete e;
			});
		res.info->base.cpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), offset, descriptor_size);
		res.info->base.gpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), offset, descriptor_size);
		res.info->base.resource_info = resources.data()+ offset;
		res.info->descriptor_size = descriptor_size;
		res.info->count = count;
		return res;
	}
	template<class T>
	void place(HandleTable& table, UINT i, T smth)
	{
		smth(table[i]);
	}

	template<class T, class ...Args>
	void place(HandleTable& table, UINT i, T smth, Args...args)
	{
		place(table, i, smth);
		place(table, i + 1, args...);
	}

public:
	using ptr = std::shared_ptr<DescriptorHeap>;

	ComPtr<ID3D12DescriptorHeap> get_native()
	{
		return m_rtvHeap;
	}


	DescriptorHeap(UINT num, DescriptorHeapType type, DescriptorHeapFlags flags = DescriptorHeapFlags::NONE);

	virtual ~DescriptorHeap()
	{

	}
	void reset()
	{
		current_offset = 0;
	}

	size_t size()
	{
		return max_count;
	}
	template<class T>
	HandleTable create_table(T _count,
		typename std::enable_if<std::is_scalar<T>::value>::type* = 0)
	{

		UINT count = static_cast<UINT>(_count);
		std::lock_guard<std::mutex> g(m);

		if (frees[count].size())
		{
			auto&& list = frees[count];
			UINT offset = list.front();
			list.pop_front();
			return make_table(count, offset);
		}

		if (current_offset >= max_count)
			Log::get() << Log::LEVEL_ERROR << "DescriptorHeap limit reached!" << Log::endl;

		HandleTable res = make_table(count, current_offset);
		current_offset += count;
		return res;
	}


	template<class ...Args>
	HandleTable create_table(Args... t)
	{
		const UINT count = sizeof...(t);
		std::lock_guard<std::mutex> g(m);

		if (frees[count].size())
		{
			auto&& list = frees[count];
			size_t offset = list.front();
			list.pop_front();
			return make_table(count, offset);
		}

		if (current_offset >= max_count)
			Log::get() << Log::LEVEL_ERROR << "DescriptorHeap limit reached!" << Log::endl;

		HandleTable res = make_table(count, current_offset);
		current_offset += count;
		place(res, 0, t...);
		return res;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE get_handle(UINT i);
	CD3DX12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(UINT i);


	Handle handle(UINT offset)
	{
		Handle res;
		res.cpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), offset, descriptor_size);
		res.gpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), offset, descriptor_size);
		res.resource_info = resources.data() + offset;
		return res;
	}

	HandleTable get_table_view(UINT offset, UINT count)
	{
		assert((offset + count) <= max_count && "Not enought handles");
		HandleTable res;
		res.info = std::shared_ptr<HandleTable::helper>(new HandleTable::helper);

		res.info->base.cpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), offset, descriptor_size);
		res.info->base.gpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), offset, descriptor_size);
		res.info->base.resource_info = resources.data() + offset;
		res.info->descriptor_size = descriptor_size;
		res.info->count = count;
		return res;
	}

	HandleTableLight get_light_table_view(UINT offset, UINT count)
	{
		assert((offset + count) <= max_count && "Not enought handles");
		HandleTableLight res;

		res.cpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), offset, descriptor_size);
		res.gpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), offset, descriptor_size);
		res.descriptor_size = descriptor_size;
		res.resource_info = resources.data() + offset;
		res.count = count;
		return res;
	}

};

class DescriptorHeapPaged;
class DescriptorPage
{

//	std::mutex m;
	friend class DescriptorHeapPaged;
	UINT heap_offset;
	UINT count;
	UINT offset;
	DescriptorHeapPaged* heap;

	CommonAllocator::Handle alloc_handle;
public:
	DescriptorPage(DescriptorHeapPaged* heap, CommonAllocator::Handle alloc_handle,  UINT heap_offset, UINT count) :heap(heap), heap_offset(heap_offset), count(count), alloc_handle(alloc_handle)
	{
		offset = 0;
	}

	~DescriptorPage()
	{
	}
	bool has_free_size(UINT need_count = 1)
	{
		return offset + need_count < count;
	}


	Handle place();
	HandleTableLight place(UINT count);
	void free();
};

class DescriptorHeapPaged :public DescriptorHeap
{
	CommonAllocator allocator;

	UINT offset = 0;

public:
	using ptr = std::shared_ptr<DescriptorHeapPaged>;

	DescriptorHeapPaged(UINT num, DescriptorHeapType type, DescriptorHeapFlags flags = DescriptorHeapFlags::NONE) : DescriptorHeap(num, type, flags), allocator(num/32)
	{

	}

	~DescriptorHeapPaged()
	{
	}
	std::mutex m;

	DescriptorPage* create_page(UINT count = 1)
	{

		UINT pages_count = (count + 32 - 1) / 32;

		std::lock_guard<std::mutex> g(m);
		auto handle = allocator.Allocate(pages_count, 1);

		return new DescriptorPage(this, handle, (UINT)(handle.get_offset()*32), pages_count*32);
	}

	void free_page(DescriptorPage* page)
	{

		std::lock_guard<std::mutex> g(m);
		allocator.Free(page->alloc_handle);
		delete page;
	}

	size_t used_size()
	{
		return allocator.get_max_usage();
	}

};

class DescriptorHeapManager : public Singleton<DescriptorHeapManager>
{
	friend class Singleton<DescriptorHeapManager>;
	DescriptorHeap::ptr heap_cb_sr_ua_static;
	DescriptorHeap::ptr heap_rt;
	DescriptorHeap::ptr heap_ds;



	DescriptorHeapManager();

public:

	~DescriptorHeapManager()
	{
		heap_cb_sr_ua_static->reset();
	}
	DescriptorHeapPaged::ptr gpu_srv;
	DescriptorHeapPaged::ptr gpu_smp;


	DescriptorHeapPaged::ptr cpu_srv;
	DescriptorHeapPaged::ptr cpu_rtv;
	DescriptorHeapPaged::ptr cpu_dsv;
	DescriptorHeapPaged::ptr cpu_smp;

	DescriptorHeap::ptr& get_csu_static()
	{
		return heap_cb_sr_ua_static;
	}
	DescriptorHeap::ptr& get_rt()
	{
		return heap_rt;
	}

	DescriptorHeapPaged::ptr& get_samplers()
	{
		return cpu_smp;
	}
};


struct Lockable
{
	using guard = std::lock_guard<std::mutex>;
	using mutex = std::mutex;
};

struct thread_tester
{
	std::atomic<std::thread::id>& id;
	thread_tester(std::atomic<std::thread::id>& id) :id(id)
	{
		auto prev = id.exchange(std::this_thread::get_id());

		assert(prev== std::thread::id());
	}

	~thread_tester()
	{
		auto prev = id.exchange(std::thread::id());
		assert(prev == std::this_thread::get_id());
	}
};
struct  Free
{
	using guard = thread_tester;
	using mutex = std::atomic<std::thread::id>;
};

template<DescriptorHeapType type, class LockPolicy = Free, DescriptorHeapFlags flags = DescriptorHeapFlags::NONE>
class DynamicDescriptor
{
	friend class CommandList;
	friend class GraphicsContext;
	friend class ComputeContext;
		template<class LockPolicy >
		friend class GPUCompiledManager;
	std::list<DescriptorPage*> pages;

	typename LockPolicy::mutex m;
	void create_heap(UINT count)
	{

		
		if constexpr (flags == DescriptorHeapFlags::SHADER_VISIBLE)
		{

			if constexpr (type == DescriptorHeapType::CBV_SRV_UAV)
				pages.push_back(DescriptorHeapManager::get().gpu_srv->create_page(count));

			if constexpr (type == DescriptorHeapType::SAMPLER)
				pages.push_back(DescriptorHeapManager::get().gpu_smp->create_page(count));

		}
		else
		{
			if constexpr (type == DescriptorHeapType::CBV_SRV_UAV)
				pages.push_back(DescriptorHeapManager::get().cpu_srv->create_page(count));


			if constexpr (type == DescriptorHeapType::RTV)
				pages.push_back(DescriptorHeapManager::get().cpu_rtv->create_page(count));

			if constexpr (type == DescriptorHeapType::SAMPLER)
				pages.push_back(DescriptorHeapManager::get().cpu_smp->create_page(count));

			if constexpr (type == DescriptorHeapType::DSV)
				pages.push_back(DescriptorHeapManager::get().cpu_dsv->create_page(count));

		}

	//	assert(pages.size() < 100);
	}



	void reset()
	{
		typename LockPolicy::guard g(m);

		for (auto& p : pages)
			p->free();

		pages.clear();
	}


	HandleTableLight prepare(UINT count)
	{
		typename LockPolicy::guard g(m);

		if (pages.empty() || !pages.back()->has_free_size(count))
		{
			create_heap(count);
		}

		return pages.back()->place(count);
	}

public:
	~DynamicDescriptor()
	{
		reset();
	}

	Handle place(Handle e)
	{
		auto table = place();
		table.place(e);
		return table;
	}

	HandleTableLight place(std::initializer_list<Handle> list)
	{
		auto table = prepare((UINT)list.size());

		int i = 0;
		for (auto& e : list)
			table[i++].place(e);

		return table;
	}


	HandleTableLight place(UINT count)
	{
		return prepare(count);
	}

	Handle place()
	{
		return prepare(1)[0];
	}
};

}
