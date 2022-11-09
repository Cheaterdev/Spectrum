export module HAL:HeapAllocators;
import Core;

import :Types;
import :Utils;
import :Heap;
import :API.Device;
using namespace HAL;


export namespace HAL
{
	struct ResourceHeapPage : public CommonAllocator, public Heap
	{

		using ptr = std::shared_ptr<ResourceHeapPage>;
		ResourceHeapPage(Device& device, size_t size, HeapType type, MemoryType memoryType) :Heap(device, HAL::HeapDesc{ size, type, memoryType, HeapFlags::NONE }), CommonAllocator(size)
		{



		}
		virtual ~ResourceHeapPage() = default;
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
		ResourceHandle(const CommonAllocator::Handle& handle, HeapAllocatorInterface* owner, Heap::ptr heap) :handle(handle), owner(owner), heap(heap)
		{
			offset = handle.get_offset();
		}
		ResourceHandle(size_t offset, Heap::ptr heap) : heap(heap), offset(offset)
		{
		}

		Heap::ptr get_heap() const
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

		Heap::ptr heap;
		size_t offset;

	};

	struct HeapIndex
	{
		MemoryType memory;
		HeapType type;

		GEN_DEF_COMP(HeapIndex);
	};
	class HeapFactory
	{

		using heap_list = std::list<ResourceHeapPage::ptr>;
		std::map<HeapIndex, heap_list> free_heaps;
		std::mutex m;
		Device& device;
	public:
		ResourceHeapPage::ptr AllocateHeap(HeapIndex index, size_t size = 0)
		{
			std::lock_guard<std::mutex> g(m);

			auto& list = free_heaps[index];
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

			auto res = std::make_shared<ResourceHeapPage>(device, size, index.type, index.memory);


			return res;

		}

		void Free(HeapIndex index, ResourceHeapPage::ptr page)
		{
			std::lock_guard<std::mutex> g(m);
			free_heaps[index].push_back(page);
		}
		HeapFactory(Device& device):device(device)
		{
			
		}

	};

	template<class LockPolicy = Thread::Free>
	class HeapAllocator : public HeapAllocatorInterface
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

			auto res = Device::get().get_heap_factory().AllocateHeap(creation_info, size);
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
			assert(handle);
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

				Device::get().get_heap_factory().Free(creation_info, ptr);
			}
		}

	};


	struct TileHeapPosition
	{

		ResourceHandle handle;
		Heap::ptr heap;
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
		ResourceHandle alloc(size_t size, size_t alignment, MemoryType memory, HeapType type)
		{
			typename LockPolicy::guard g(m);

			HeapIndex index;

			index.memory = memory;
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
			static const size_t TileSize = 64 * 1024;

			auto handle = alloc(count * TileSize, TileSize, HAL::MemoryType::COMMITED, type);

			TileHeapPosition result;

			result.offset = static_cast<UINT>(handle.get_offset() / (64 * 1024));
			result.heap = handle.get_heap();

			result.handle = handle;
			result.count = count;
			return result;
		}


	};

	class ResourceHeapPageManager :public ResourceHeapAllocator<Thread::Lockable>
	{
		Device& device;
	public:
		ResourceHeapPageManager(Device& device) :device(device) {}
		~ResourceHeapPageManager()
		{
			
		}
	};



	struct UploadInfo :public HAL::ResourceAddress
	{
		uint size;
	};

	template<class LockPolicy>
	class CPUGPUAllocator
	{
		// todo: get rid of handles - clear allocator by force
		HAL::ResourceHeapAllocator<LockPolicy> allocator;
		std::list<ResourceHandle> handles;
		typename LockPolicy::mutex m;
	public:
		static constexpr uint DEFAULT_ALIGN = 256;

		void reset()
		{
			typename LockPolicy::guard g(m);

			for (auto& h : handles)
				h.Free();
			handles.clear();
		}

		UploadInfo aquire_data(UINT64 uploadBufferSize, HeapType heap_type, unsigned int alignment = DEFAULT_ALIGN)
		{
			const auto AlignedSize = static_cast<UINT>(Math::roundUp(uploadBufferSize, alignment));


			auto handle = allocator.alloc(AlignedSize, alignment, HAL::MemoryType::COMMITED, heap_type);

			typename LockPolicy::guard g(m);

			handles.emplace_back(handle);

			return  { handle.get_heap()->as_buffer()->get_resource_address().offset(handle.get_offset()), uint(uploadBufferSize) };
		}
	};

	template<class LockPolicy>
	class Uploader :public CPUGPUAllocator<LockPolicy>
	{

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


	public:

		UploadInfo place_data(UINT64 uploadBufferSize, unsigned int alignment = CPUGPUAllocator<LockPolicy>::DEFAULT_ALIGN)
		{
			return CPUGPUAllocator<LockPolicy>::aquire_data(uploadBufferSize, HeapType::UPLOAD, alignment);
		}


		template<class ...Args>
		UploadInfo place_raw(Args... args)
		{
			size_t size = (0 + ... + size_of(args));
			auto info = place_data(size);
			size_t start = 0;
			{
				(write(info, start, std::forward<Args>(args)), ...);
			}
			return info;
		}

		void write(UploadInfo& info, size_t offset, void* data, size_t size)
		{
			if (size > 0) memcpy(info.get_cpu_data() + offset, data, size);
		}

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
		}
	};


	template<class LockPolicy>
	class Readbacker :public CPUGPUAllocator<LockPolicy>
	{
	public:
		UploadInfo read_data(UINT64 uploadBufferSize, unsigned int alignment = CPUGPUAllocator<LockPolicy>::DEFAULT_ALIGN)
		{
			return CPUGPUAllocator<LockPolicy>::aquire_data(uploadBufferSize, HeapType::READBACK, alignment);
		}
	};


}

