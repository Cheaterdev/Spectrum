export module HAL:FrameManager;

import Core;
import :Types;
import :HeapAllocators;
import :DescriptorHeap;
import :Device;
import :QueryHeap;

export
namespace HAL {


	class GPUEntityStorageInterface
	{
	public:
		static constexpr uint DEFAULT_ALIGN = 256;

	public:
		virtual ResourceHandle alloc_memory(size_t size, size_t alignment, HeapIndex options) = 0;
		virtual QueryHandle alloc_query(uint size, QueryType options) = 0;
		virtual Handle  alloc_base_descriptor(uint size, DescriptorHeapIndex options) = 0;

		virtual void resolve_timers(std::function<void(const QueryType&, uint64, uint64, QueryHeap::ptr) > f) = 0;
		template <class Type = Handle>
		Type alloc_descriptor(uint size, DescriptorHeapIndex options)
		{
			return Type(alloc_base_descriptor(size, options));
		}

		template <>
		Handle alloc_descriptor<Handle>(uint size, DescriptorHeapIndex options)
		{
			return alloc_base_descriptor(size, options);
		}

		virtual ~GPUEntityStorageInterface() = default;



	private:

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


		UploadInfo aquire_data(UINT64 uploadBufferSize, HeapType heap_type, unsigned int alignment = DEFAULT_ALIGN, unsigned int offset = 0);
	public:

		UploadInfo place_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN);

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

		void write(UploadInfo& info, size_t offset, void* data, size_t size);

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

		UploadInfo read_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN, unsigned int offset = 0);

		TileHeapPosition create_tile(HeapType type, UINT count = 1);
	};

	template<class T>
	concept IsGPUEntityStorageInterface = std::is_base_of_v<GPUEntityStorageInterface, T>;

	// Flat list instead of a map: there are only ever 1-2 distinct shader-visible heaps
	// (the singleton CBV_SRV_UAV and Sampler heaps), so linear scan beats tree lookups.
	using DescriptorRangeList = std::vector<std::pair<HAL::DescriptorHeap*, std::vector<std::pair<uint64, uint64>>>>;

	template<class MemoryAllocationPolicy, class AllocationPolicy = MemoryAllocationPolicy>
	struct GPUEntityStorage :
		public GPUEntityStorageInterface,
		public GPUMemoryAllocator<MemoryAllocationPolicy>,
		public DescriptorHeapPageManager<AllocationPolicy>,
		public QueryHeapPageManager<AllocationPolicy>
	{
		static constexpr bool resetable = std::is_same_v<typename AllocationPolicy::AllocatorType, LinearAllocator> || std::is_same_v<typename MemoryAllocationPolicy::AllocatorType, LinearAllocator>;

		GPUEntityStorage(Device& device) :
			GPUMemoryAllocator<AllocationPolicy>(device),
			QueryHeapPageManager<AllocationPolicy>(device),
			DescriptorHeapPageManager<AllocationPolicy>(device)
		{

		}
		GPUMemoryAllocator<MemoryAllocationPolicy>::HandleType alloc_memory(size_t size, size_t alignment, GPUMemoryAllocator<MemoryAllocationPolicy>::HeapMemoryOptions options) override
		{
			return GPUMemoryAllocator<MemoryAllocationPolicy>::alloc(size, alignment, options);
		}

		QueryHeapPageManager<AllocationPolicy>::HandleType  alloc_query(uint size, QueryHeapPageManager<AllocationPolicy>::HeapMemoryOptions options) override
		{
			return QueryHeapPageManager<AllocationPolicy>::alloc(size, 1, options);
		}

		// Set once this storage allocates a descriptor from a shader-visible heap. Lets
		// collect_descriptor_ranges skip the whole page-walk for storages that never
		// touched one (transitions-only, memory-only, RTV/DSV-only command lists, etc).
		bool has_deferred_descriptors = false;

		Handle  alloc_base_descriptor(uint size, DescriptorHeapPageManager<AllocationPolicy>::HeapMemoryOptions options) override
		{
			auto h = DescriptorHeapPageManager<AllocationPolicy>::alloc(size, 1, options);

			// LinearAllocator-backed (per-frame) storages defer their GPU-visible heap
			// write to a single batched CopyDescriptors done at collect_descriptor_ranges().
			constexpr bool deferred_gpu_write = std::is_same_v<typename AllocationPolicy::AllocatorType, LinearAllocator>;

			if constexpr (deferred_gpu_write)
				if (check(options.flags & DescriptorHeapFlags::ShaderVisible))
					has_deferred_descriptors = true;

			return Handle{ std::make_shared<DescriptorHeapStorage>(h, deferred_gpu_write), 0 };
		}

		// Accumulates every shader-visible page's [start, current-watermark) range into
		// the caller's flat range list, keyed by physical heap. Pure collection - no D3D12
		// call here, so many storages can contribute into one list and be flushed in one
		// CopyDescriptors call per heap (see FrameResources::commit_descriptors_to_gpu).
		// reserve_hint: upper bound on how many storages will contribute, used to size a
		// heap's range vector once, on its first range.
		void collect_descriptor_ranges(DescriptorRangeList& ranges, size_t reserve_hint = 0)
		{
			if (!has_deferred_descriptors) return;

			DescriptorHeapPageManager<AllocationPolicy>::for_each(
				[&](const DescriptorHeapPageManager<AllocationPolicy>::HeapMemoryOptions& options, uint64 from, uint64 to, HAL::DescriptorHeap::ptr heap)
				{
					if (!check(options.flags & DescriptorHeapFlags::ShaderVisible)) return;
					if (to <= from) return;

					HAL::DescriptorHeap* h = heap.get();
					auto it = std::find_if(ranges.begin(), ranges.end(), [h](auto& entry) { return entry.first == h; });
					if (it == ranges.end())
					{
						ranges.emplace_back(h, std::vector<std::pair<uint64, uint64>>{});
						it = ranges.end() - 1;
						if (reserve_hint) it->second.reserve(reserve_hint);
					}
					it->second.push_back({ from, to });
				});
		}


		void resolve_timers(std::function<void(const QueryType&, uint64, uint64, QueryHeap::ptr) > f) override
		{

			QueryHeapPageManager<AllocationPolicy>::for_each(f);

		}


		void reset() requires (resetable)
		{

			DescriptorHeapPageManager<AllocationPolicy>::for_each([](const DescriptorHeapPageManager<AllocationPolicy>::HeapMemoryOptions& type, uint64 from, uint64 to, HAL::DescriptorHeap::ptr heap)
				{

					for (uint i = static_cast<uint>(from); i < static_cast<uint>(to); i++)
					{
						heap->get_resource_info(i) = HAL::Views::Null();
					}

				});

			GPUMemoryAllocator<MemoryAllocationPolicy>::reset();
			QueryHeapPageManager<AllocationPolicy>::reset();
			DescriptorHeapPageManager<AllocationPolicy>::reset();
		}



		~GPUEntityStorage()
		{
			if constexpr (resetable) reset();
		}
	};



	class GPUEntityStorageProxy :public GPUEntityStorageInterface
	{
	protected:
		std::shared_ptr<GPUEntityStorageInterface> proxy;

	public:

		void set_proxy(std::shared_ptr<GPUEntityStorageInterface> proxy);

		ResourceHandle alloc_memory(size_t size, size_t alignment, HeapIndex options) override;
		QueryHandle alloc_query(uint size, QueryType options) override;
		Handle alloc_base_descriptor(uint size, DescriptorHeapIndex options) override;

		template <class Type = Handle>
		Type alloc_descriptor(uint size, DescriptorHeapIndex options)
		{
			return Type(alloc_base_descriptor(size, options));
		}

		template <>
		Handle alloc_descriptor<Handle>(uint size, DescriptorHeapIndex options)
		{
			return alloc_base_descriptor(size, options);
		}

		void resolve_timers(std::function<void(const QueryType&, uint64, uint64, QueryHeap::ptr) > f) override;
	};


	class StaticCompiledGPUData : public GPUEntityStorage<GlobalAllocationPolicy>
	{
		Device& device;
	public:
		using GPUEntityStorage<GlobalAllocationPolicy>::place_raw;

		StaticCompiledGPUData(Device& device);
	};

	class FrameResources :public SharedObject<FrameResources>, public GPUEntityStorage<LocalAllocationPolicy>
	{
		friend class FrameResourceManager;

		Device& device;
		std::uint64_t frame_number = 0;
		std::mutex m;

		Pool<std::shared_ptr<GPUEntityStorage<LocalAllocationPolicy>>> gpu_resources;

		enum_array<CommandListType, Pool<std::shared_ptr<CommandAllocator>>> command_allocators;

	public:
		using ptr = std::shared_ptr<FrameResources>;

		FrameResources(Device& device);
		~FrameResources();

		std::uint64_t get_frame();

		// exclusive: true when this FrameResources was obtained (via begin_frame()) just for
		// this one command list, which then owns committing its descriptors in end().
		// false (default) when this FrameResources is shared across multiple command lists
		// by a longer-lived owner (e.g. FrameGraph's current_frame) that commits once itself.
		std::shared_ptr<CommandList> start_list(LiteralWStr name = L"", CommandListType type = CommandListType::DIRECT, bool exclusive = false);

		void free_storage(std::shared_ptr<GPUEntityStorageInterface> e);
		std::shared_ptr<GPUEntityStorageInterface> get_storage();

		// Flushes this FrameResources' own descriptor pages plus every per-commandlist
		// storage handed out via get_storage(). Called once by whichever owner obtained
		// this FrameResources from FrameResourceManager::begin_frame(): either a single
		// CommandList (in end(), if it created its own frame_resources) or the FrameGraph
		// (once, after all passes for the frame have been compiled).
		void commit_descriptors_to_gpu();

		void free_ca(std::shared_ptr<CommandAllocator> e);
		std::shared_ptr<CommandAllocator> get_ca(CommandListType type);
	};

	class FrameResourceManager
	{
		std::atomic_size_t frame_number = 0;
		Device& device;
	public:
		FrameResourceManager(Device& device);
		FrameResources::ptr begin_frame();
	};

}
