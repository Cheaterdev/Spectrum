#pragma once
namespace DX12
{
	enum class HeapType : int
	{
		DEFAULT = D3D12_HEAP_TYPE_DEFAULT,
		UPLOAD = D3D12_HEAP_TYPE_UPLOAD,
		READBACK = D3D12_HEAP_TYPE_READBACK,
		CUSTOM = D3D12_HEAP_TYPE_CUSTOM,
		RESERVED = 10,
		PRESENT = 5
	};


	class ResourceHeap
	{
		std::shared_ptr<Resource> cpu_buffer;

	public:
		ComPtr<ID3D12Heap > heap;
		HeapType type;
		D3D12_HEAP_FLAGS flags;

		size_t heap_size = 0;
		using ptr = std::shared_ptr<ResourceHeap>;

		ResourceHeap(size_t size, HeapType type, D3D12_HEAP_FLAGS flags) :type(type), flags(flags)
		{
			init(size);
		}
		HeapType get_type() { return type; }
		virtual ~ResourceHeap() = default;
	protected:
		void init(size_t size);
	};

//	template<class AllocatorType>
	struct ResourceHeapPage : CommonAllocator, ResourceHeap
	{
		using ptr = std::shared_ptr<ResourceHeapPage>;
		ResourceHeapPage(size_t size, HeapType type, D3D12_HEAP_FLAGS flags) :ResourceHeap(size, type, flags), CommonAllocator(size)
		{

		}

	};
	class HeapAllocator;

	struct ResourceHandle
	{
		ResourceHandle() = default;
		ResourceHandle(const CommonAllocator::Handle& handle, HeapAllocator* owner, ResourceHeap::ptr heap):handle(handle), owner(owner), heap(heap)
		{
			offset = handle.get_offset();
		}

		ResourceHeap::ptr get_heap() const
		{
			return heap;
		}

		UINT64 get_offset() const
		{
			return offset;
		}

		void Free();

		bool operator<(const ResourceHandle& other)const
		{
			if (get_heap() != other.get_heap()) return get_heap() < other.get_heap();

			return get_offset() < other.get_offset();
		}

		bool operator!=(const ResourceHandle& other)const
		{
			if (get_heap() != other.get_heap()) return true;

			return get_offset() != other.get_offset();
		}

		bool operator==(const ResourceHandle& other)const
		{
			if (get_heap() != other.get_heap()) return false;

			return get_offset() == other.get_offset();
		}
		CommonAllocator::Handle handle;
	private:
	
		HeapAllocator* owner;

		ResourceHeap::ptr heap;
		size_t offset;

	};

	struct HeapIndex
	{
		D3D12_HEAP_FLAGS flags;
		HeapType type;

		GEN_DEF_COMP(HeapIndex);
	};

	class HeapAllocator
	{
		const HeapIndex creation_info;
		using heap_list = std::list<ResourceHeapPage::ptr>;
		heap_list all_heaps;
		static const size_t Alignment = 16 * 1024 * 1024;
		std::mutex m;
	public:
		HeapAllocator(HeapIndex index) :creation_info(index)
		{

		}
		ResourceHeapPage::ptr AllocateHeap(size_t size = 0)
		{

			size = std::max(Math::AlignUp(size, Alignment), Alignment);

			auto res = std::make_shared<ResourceHeapPage>(size, creation_info.type, creation_info.flags);
			all_heaps.emplace_back(res);
			return res;
		}

		ResourceHandle alloc(size_t size, size_t alignment)
		{
			std::lock_guard<std::mutex> g(m);

			for (auto& heap : all_heaps)
			{
				auto handle = heap->TryAllocate(size, alignment);

				if (handle)
				{
					return { *handle, this, heap };
				}
			}

			auto heap = AllocateHeap(size);
			auto handle = heap->TryAllocate(size, alignment);
			return { *handle,this, heap };

		}

		void free(CommonAllocator::Handle& handle)
		{
			std::lock_guard<std::mutex> g(m);

			auto heap = static_cast<ResourceHeapPage*>(handle.get_owner());
			handle.FreeAndClear();

		
			if (heap->is_empty())
			{
				auto h = std::find_if(all_heaps.begin(), all_heaps.end(), [&](const ResourceHeapPage::ptr& p) {return p.get() == heap; });
				all_heaps.erase(h);
			}
		}

	};

	struct ResourceAllocationInfo
	{
		size_t size;
		size_t alignment;
		D3D12_HEAP_FLAGS flags;
	};

	struct TileHeapPosition
	{

		ResourceHandle handle;
		ResourceHeap::ptr heap;
		UINT offset;
		UINT count = 1;
	};

	struct ResourceTile
	{
		ivec3 pos = ivec3(0, 0, 0);
		UINT subresource;

		ivec3 size = ivec3(1, 1, 1);
		TileHeapPosition heap_position;
	};

	struct update_tiling_info
	{
		std::map<ResourceHeap*, std::vector<ResourceTile>> tiles;
		Resource* resource = nullptr;

		Resource* source = nullptr;
		ivec3 pos;
		ivec3 source_pos;
		ivec3 size;
		uint source_subres;
		uint target_subres;

		void add_tile(ResourceTile tile)
		{
			tiles[tile.heap_position.heap.get()].push_back(tile);
		}
	};

	// for tiles now, only
	class ResourceHeapAllocator
	{

	

	using flags_map = std::map<HeapIndex, std::shared_ptr<HeapAllocator>>;

		std::mutex m;
		flags_map creators;
		
	public:

		ResourceHandle alloc(size_t size, size_t alignment, D3D12_HEAP_FLAGS flags, HeapType type)
		{
			std::lock_guard<std::mutex> g(m);

			HeapIndex index;

			index.flags = flags;
			index.type = type;

			auto& creator = creators[index];

			if (!creator)
			{
				creator = std::make_shared<HeapAllocator>(index);
			}
		//	auto& creator = *it;

			return creator->alloc(size, alignment);
		}

		TileHeapPosition create_tile(D3D12_HEAP_FLAGS flags, HeapType type, UINT count = 1)
		{
			static const size_t TileSize = 64 * 1024_t;

			auto handle = alloc(count*TileSize, TileSize, flags, type);

			TileHeapPosition result;

			result.offset = handle.get_offset() / (64 * 1024);
			result.heap = handle.get_heap();

			result.handle = handle;
			result.count = count;
			return result;
		}


	};

	class ResourceHeapPageManager :public ResourceHeapAllocator, public  Singleton<ResourceHeapPageManager>
	{
	public:
		ResourceHeapPageManager() = default;
	};
	/*
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
	};*/
}