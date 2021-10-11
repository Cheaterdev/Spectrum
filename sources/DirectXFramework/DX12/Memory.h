#pragma once
import StateContext;
import Allocators;
#include "Math/Math.h"
import Threading;
#include "DX12/Descriptors.h"
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
	
	struct TrackedObjectState
	{
		bool used = false;
		void reset()
		{
			used = false;
		}
	};

	template<class T>
	class Trackable: public ObjectState<TrackedObjectState>
	{
	public:
		mutable std::shared_ptr<T> tracked_info;
		Trackable()
		{
			tracked_info = std::make_shared<T>();
		}
		virtual ~Trackable() {};

	};

	template <class T>
	concept TrackableClass = std::is_base_of<ObjectState<TrackedObjectState>, T>::value;
	
	class TrackedObject
	{
	public:
		using ptr = std::shared_ptr<TrackedObject>;
		virtual ~TrackedObject() = default;
	};
	
	
	class TrackedHeap : public TrackedObject
	{
	public:
		ComPtr<ID3D12Heap > heap;
	};

	class ResourceHeap :std::enable_shared_from_this<ResourceHeap>, public Trackable<TrackedHeap>
	{

		D3D12_HEAP_DESC desc;
	public:
		std::shared_ptr<Resource> cpu_buffer;

		HeapType type;
		D3D12_HEAP_FLAGS flags;

		size_t heap_size = 0;
		using ptr = std::shared_ptr<ResourceHeap>;
		ComPtr<ID3D12Heap > get_native()
		{
			return tracked_info->heap;
		}
		
		
		ResourceHeap(size_t size, HeapType type, D3D12_HEAP_FLAGS flags) :type(type), flags(flags)
		{
			init(size);
		}
		HeapType get_type() { return type; }
		virtual ~ResourceHeap() = default;

		void init_cpu(ptr);

		std::span<std::byte> get_data();
	protected:
		void init(size_t size);
	};

	//	template<class AllocatorType>
	struct ResourceHeapPage : public CommonAllocator, public ResourceHeap
	{

		using ptr = std::shared_ptr<ResourceHeapPage>;
		ResourceHeapPage(size_t size, HeapType type, D3D12_HEAP_FLAGS flags) :ResourceHeap(size, type, flags), CommonAllocator(size)
		{



		}

	};

	class HeapAllocatorInterface
	{
	public:
		virtual ~HeapAllocatorInterface() = default;
		virtual void free(CommonAllocator::Handle& handle) = 0;
	};

	struct ResourceHandle
	{
		ResourceHandle() = default;
		ResourceHandle(const CommonAllocator::Handle& handle, HeapAllocatorInterface* owner, ResourceHeap::ptr heap) :handle(handle), owner(owner), heap(heap)
		{
			offset = handle.get_offset();
		}
		ResourceHandle(size_t offset, ResourceHeap::ptr heap) : heap(heap), offset(offset)
		{
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

		HeapAllocatorInterface* owner = nullptr;

		ResourceHeap::ptr heap;
		size_t offset;

	};

	struct HeapIndex
	{
		D3D12_HEAP_FLAGS flags;
		HeapType type;

		GEN_DEF_COMP(HeapIndex);
	};
	class HeapFactory: public Singleton<HeapFactory>
	{
		
		using heap_list = std::list<ResourceHeapPage::ptr>;
		std::map<HeapIndex, heap_list> free_heaps;
		std::mutex m;
	public:
		ResourceHeapPage::ptr AllocateHeap(HeapIndex index, size_t size = 0)
		{
			std::lock_guard<std::mutex> g(m);

			auto &list = free_heaps[index];
			if (!list.empty())
			{
				for (auto it = list.begin(); it != list.end(); ++it)
				{
					if ((*it)->get_size() >= size)
					{
						auto first = *it;
						list.erase(it);
						return first;
					}
				}

			}
			auto res = std::make_shared<ResourceHeapPage>(size, index.type, index.flags);

			res->init_cpu(res);

			return res;
			
		}

		void Free(HeapIndex index, ResourceHeapPage::ptr page)
		{
			std::lock_guard<std::mutex> g(m);
			free_heaps[index].push_back(page);
		}
		
	};

	template<class LockPolicy = Thread::Free>
	class HeapAllocator: public HeapAllocatorInterface
	{
		const HeapIndex creation_info;
		using heap_list = std::list<ResourceHeapPage::ptr>;
		heap_list all_heaps;
	

		static const size_t Alignment = 4 * 1024 * 1024;
		typename LockPolicy::mutex m;
		bool del_heaps;
	public:
		HeapAllocator(HeapIndex index, bool del_heaps = true) :creation_info(index), del_heaps(del_heaps)
		{

		}
		ResourceHeapPage::ptr AllocateHeap(size_t size = 0)
		{
			
			size = std::max(Math::AlignUp(size, Alignment), Alignment);

			auto res = HeapFactory::get().AllocateHeap(creation_info,size);
			all_heaps.emplace_back(res);
			return res;
		}

		ResourceHandle alloc(size_t size, size_t alignment)
		{
			typename LockPolicy::guard g(m);


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

		void free(CommonAllocator::Handle& handle) override
		{
			typename LockPolicy::guard g(m);

			auto heap = static_cast<ResourceHeapPage*>(handle.get_owner());
			handle.FreeAndClear();

			//uint t = heap.use_count();
			if (del_heaps && heap->is_empty())
			{
				auto h = std::find_if(all_heaps.begin(), all_heaps.end(), [&](const ResourceHeapPage::ptr& p) {return p.get() == heap; });
				auto ptr = *h;
			
				all_heaps.erase(h);

				HeapFactory::get().Free(creation_info, ptr);
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

	template<class LockPolicy = Thread::Free>
	class ResourceHeapAllocator
	{



		using flags_map = std::map<HeapIndex, std::shared_ptr<HeapAllocator<LockPolicy>>>;

		typename LockPolicy::mutex m;
		flags_map creators;
		bool del_heaps;
	public:
		ResourceHeapAllocator(bool del_heaps = true) :del_heaps(del_heaps)
		{

		}
		ResourceHandle alloc(size_t size, size_t alignment, D3D12_HEAP_FLAGS flags, HeapType type)
		{
			typename LockPolicy::guard g(m);
			
			HeapIndex index;

			index.flags = flags;
			index.type = type;

			auto& creator = creators[index];

			if (!creator)
			{
				creator = std::make_shared<HeapAllocator<LockPolicy>>(index, del_heaps);
			}
			//	auto& creator = *it;

			return creator->alloc(size, alignment);
		}

		TileHeapPosition create_tile(D3D12_HEAP_FLAGS flags, HeapType type, UINT count = 1)
		{
			static const size_t TileSize = 64 * 1024_t;

			auto handle = alloc(count * TileSize, TileSize, flags, type);

			TileHeapPosition result;

			result.offset = static_cast<UINT>(handle.get_offset() / (64 * 1024));
			result.heap = handle.get_heap();

			result.handle = handle;
			result.count = count;
			return result;
		}


	};

	class ResourceHeapPageManager :public ResourceHeapAllocator<Thread::Lockable>, public  Singleton<ResourceHeapPageManager>
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
