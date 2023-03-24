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
		static constexpr uint DEFAULT_ALIGN = 256;

	public:
		virtual ResourceHandle alloc_memory(size_t size, size_t alignment, HeapIndex options) = 0;
		virtual QueryHandle alloc_query(uint size, QueryType options) = 0;
		virtual Handle  alloc_base_descriptor(uint size, DescriptorHeapIndex options) = 0;

		virtual void resolve_timers(std::function<void(const QueryType& , uint64 , uint64 , QueryHeap::ptr) > f) = 0;
		template <class Type = Handle>
		Type alloc_descriptor(uint size, DescriptorHeapIndex options)
		{
			return Type(alloc_base_descriptor(size, options));
		}

		template <>
		Handle alloc_descriptor<Handle>(uint size,DescriptorHeapIndex options)
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


		UploadInfo aquire_data(UINT64 uploadBufferSize, HeapType heap_type, unsigned int alignment = DEFAULT_ALIGN);
	public:

		UploadInfo place_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN)
		{
			return aquire_data(uploadBufferSize, HeapType::UPLOAD, alignment);
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
	public:
		UploadInfo read_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN)
		{
			return aquire_data(uploadBufferSize, HeapType::READBACK, alignment);
		}

	public:
	
		TileHeapPosition create_tile(HeapType type, UINT count = 1);
		/*{
			static const size_t TileSize = 64 * 1024;
			HeapIndex index = { HAL::MemoryType::COMMITED , type };

			auto handle = alloc(count * TileSize, TileSize, index);

			TileHeapPosition result;

			result.offset = static_cast<UINT>(handle.get_offset() / (64 * 1024));
			result.heap = handle.get_heap();

			result.handle = handle;
			result.count = count;
			return result;
		}*/
	};

		template<class MemoryAllocationPolicy,class AllocationPolicy = MemoryAllocationPolicy>
		struct GPUEntityStorage:
		public GPUEntityStorageInterface,
			public GPUMemoryAllocator<MemoryAllocationPolicy>,
			public DescriptorHeapPageManager<AllocationPolicy>,
			public QueryHeapPageManager<AllocationPolicy>
		{
			static constexpr bool resetable = std::is_same_v<typename AllocationPolicy::AllocatorType,LinearAllocator> || std::is_same_v<typename MemoryAllocationPolicy::AllocatorType,LinearAllocator>;

			GPUEntityStorage(Device& device):
				GPUMemoryAllocator<AllocationPolicy>(device),
				QueryHeapPageManager<AllocationPolicy>(device),
				DescriptorHeapPageManager<AllocationPolicy>(device)
			{
				
			}
			GPUMemoryAllocator<MemoryAllocationPolicy>::HandleType alloc_memory(size_t size, size_t alignment, GPUMemoryAllocator<MemoryAllocationPolicy>::HeapMemoryOptions options) override
			{
				return GPUMemoryAllocator<MemoryAllocationPolicy>::alloc(size, alignment, options);
			}

			QueryHeapPageManager<AllocationPolicy>::HandleType  alloc_query(uint size,  QueryHeapPageManager<AllocationPolicy>::HeapMemoryOptions options) override
			{
				return QueryHeapPageManager<AllocationPolicy>::alloc(size,1,options);
			}

			Handle  alloc_base_descriptor(uint size, DescriptorHeapPageManager<AllocationPolicy>::HeapMemoryOptions options) override
			{
				auto h = DescriptorHeapPageManager<AllocationPolicy>::alloc(size, 1, options);
				
				return Handle{std::make_shared<DescriptorHeapStorage>(h),0};
			}


			void resolve_timers(std::function<void(const QueryType& , uint64 , uint64 , QueryHeap::ptr) > f) override
					{
						
				QueryHeapPageManager<AllocationPolicy>::for_each(f);

					}


			void reset() requires (resetable)
			{

				DescriptorHeapPageManager<AllocationPolicy>::for_each([](const DescriptorHeapPageManager<AllocationPolicy>::HeapMemoryOptions& type, uint64 from , uint64 to, HAL::DescriptorHeap::ptr heap)
				{
		
					for(uint i=static_cast<uint>(from);i<static_cast<uint>(to);i++)
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

			void set_proxy(std::shared_ptr<GPUEntityStorageInterface> proxy)
			{
				this->proxy = proxy;
			}

			ResourceHandle alloc_memory(size_t size, size_t alignment, HeapIndex options) override
			{
				assert(proxy);
				return proxy->alloc_memory(size, alignment, options);
			}
			QueryHandle alloc_query(uint size, QueryType options)override
			{
				assert(proxy);
				return proxy->alloc_query(size, options);
			}
			Handle  alloc_base_descriptor(uint size, DescriptorHeapIndex options)override
			{
				assert(proxy);
				return proxy->alloc_base_descriptor(size, options);
			}

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


					void resolve_timers(std::function<void(const QueryType& , uint64 , uint64 , QueryHeap::ptr) > f) override
					{
							assert(proxy);
				 proxy->resolve_timers(f);

					}
		};


		class StaticCompiledGPUData : public GPUEntityStorage<GlobalAllocationPolicy>
		{
			Device& device;
		public:
			using GPUEntityStorage<GlobalAllocationPolicy>::place_raw;

			StaticCompiledGPUData(Device& device) :device(device), GPUEntityStorage<GlobalAllocationPolicy>(device) {}
		};

		class FrameResources :public SharedObject<FrameResources>, public GPUEntityStorage<LocalAllocationPolicy>
		{
			friend class FrameResourceManager;

			std::uint64_t frame_number = 0;
			std::mutex m;

			Pool<std::shared_ptr<GPUEntityStorage<LocalAllocationPolicy>>> gpu_resources;



			enum_array<CommandListType, Pool<std::shared_ptr<CommandAllocator>>> command_allocators;

		public:
			using ptr = std::shared_ptr<FrameResources>;
			FrameResources(Device& device) : GPUEntityStorage<LocalAllocationPolicy>(device)
			{

				gpu_resources.create_func = [&device]() {
					return std::make_shared<GPUEntityStorage<LocalAllocationPolicy>>(device);

				};

				for (auto type : magic_enum::enum_values<CommandListType>())
			{
				command_allocators[type].create_func = [type](){
		return Device::get().get_ca(type);
		
		};
			}
			}
			~FrameResources()
			{
				reset();

				for (auto& e : gpu_resources.table)
				{
					e->reset();
				}
					for (auto type : magic_enum::enum_values<CommandListType>())
					for (auto& e : command_allocators[type].table)
				{
				Device::get().free_ca(e);
				}
			}

			std::uint64_t get_frame()
			{
				return frame_number;
			}

			std::shared_ptr<CommandList> start_list(std::string name = "", CommandListType type = CommandListType::DIRECT);

			void free_storage(std::shared_ptr<GPUEntityStorageInterface> e)
			{
				gpu_resources.put(std::static_pointer_cast<GPUEntityStorage<LocalAllocationPolicy>>(e));
			}

			std::shared_ptr<GPUEntityStorageInterface> get_storage() {
				return gpu_resources.get();
			}


			void free_ca(std::shared_ptr<CommandAllocator> e);

			std::shared_ptr<CommandAllocator> get_ca(CommandListType type) {
				return command_allocators[type].get();
			}


		};

		class FrameResourceManager
		{
			std::atomic_size_t frame_number = 0;
			Device& device;
		public:
			FrameResourceManager(Device& device) :device(device){}
			FrameResources::ptr begin_frame();
		};

}