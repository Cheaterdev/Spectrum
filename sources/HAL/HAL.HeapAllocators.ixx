export module HAL:HeapAllocators;
import Core;

import :Types;
import :Utils;
import :Heap;
import :API.Device;
using namespace HAL;


export namespace HAL
{
	struct HeapIndex
	{
		MemoryType memory;
		HeapType type;

		GEN_DEF_COMP(HeapIndex);
	};
	
	struct AllocatorContext
	{
		using HeapPageType = HAL::Heap;
		using HeapMemoryOptions = HeapIndex;

		static const size_t PageAlignment = 4 * 1024 * 1024;
	};
	using ResourceHandle = Allocators::HeapHandle<HAL::Heap>;


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


	class HeapFactory:public Allocators::HeapFactory<AllocatorContext, Thread::Lockable>
	{
		Device& device;
		virtual ptr_type make_heap(HeapIndex index, size_t size) override
		{
			HeapDesc desc = { size , index.type, index.memory , HeapFlags::NONE};
			return std::make_shared<HAL::Heap>(device,desc);
		}

	public:
		HeapFactory(Device& device) :device(device)
				{
					
				}

	};

	class ResourceHeapPageManager :public Allocators::HeapPageManager<AllocatorContext, Thread::Lockable>
	{
		Device& device;
	public:
		ResourceHeapPageManager(Device& _device) :Allocators::HeapPageManager<AllocatorContext, Thread::Lockable>(_device.get_heap_factory()), device(_device) {}
		~ResourceHeapPageManager()
		{
			
		}

		using Allocators::HeapPageManager<AllocatorContext, Thread::Lockable>::alloc;

		TileHeapPosition create_tile(HeapType type, UINT count = 1)
		{
			static const size_t TileSize = 64 * 1024;
			HeapIndex index = { HAL::MemoryType::COMMITED , type };

			auto handle = alloc(count * TileSize, TileSize, index);

			TileHeapPosition result;

			result.offset = static_cast<UINT>(handle.get_offset() / (64 * 1024));
			result.heap = handle.get_heap();

			result.handle = handle;
			result.count = count;
			return result;
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
		Allocators::HeapPageManager<AllocatorContext, Thread::Lockable> allocator;
		std::list<ResourceHandle> handles;
		typename LockPolicy::mutex m;
	public:
		static constexpr uint DEFAULT_ALIGN = 256;

		CPUGPUAllocator(Device&device):allocator(device.get_heap_factory())
		{
			
		}
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

			HeapIndex index = { HAL::MemoryType::COMMITED , heap_type };
			auto handle = allocator.alloc(AlignedSize, alignment, index);

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
		Uploader(Device& device) :CPUGPUAllocator<LockPolicy>(device)
		{

		}
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
		Readbacker(Device& device) :CPUGPUAllocator<LockPolicy>(device)
	{

	}
		UploadInfo read_data(UINT64 uploadBufferSize, unsigned int alignment = CPUGPUAllocator<LockPolicy>::DEFAULT_ALIGN)
		{
			return CPUGPUAllocator<LockPolicy>::aquire_data(uploadBufferSize, HeapType::READBACK, alignment);
		}
	};


}

