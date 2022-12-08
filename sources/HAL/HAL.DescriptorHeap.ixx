export module HAL:DescriptorHeap;


import :API.DescriptorHeap;
import :Types;
import Core;
export
{

	namespace HAL
	{

		struct ResourceInfo
		{
			std::variant<HAL::Views::Null, HAL::Views::DepthStencil, HAL::Views::UnorderedAccess, HAL::Views::ShaderResource, HAL::Views::RenderTarget, HAL::Views::ConstantBuffer> view = HAL::Views::Null();

			bool is_valid() const
			{
				return view.index() != 0;
			}
			ResourceInfo() = default;

			template<HAL::Views::ViewTemplate T>
			ResourceInfo(const T& v)
			{
				view = v;
			}

			void for_each_subres(std::function<void(Resource*, UINT)> f) const;
		};

		class DescriptorHeap : public SharedObject<DescriptorHeap>, public API::DescriptorHeap
		{
			friend struct Handle;
			std::vector<ResourceInfo> resources;

			UINT descriptor_size;
			
		public:
			using ptr = std::shared_ptr<DescriptorHeap>;

			DescriptorHeap(Device& device, UINT num, DescriptorHeapType type, DescriptorHeapFlags flags = DescriptorHeapFlags::None);

			virtual ~DescriptorHeap()
			{

			}
ResourceInfo* get_resource_info(uint offset)
			{
				return &resources[offset];
			}
			uint get_size();

		};


		struct DescriptorHeapIndex
		{
			DescriptorHeapType type;
			DescriptorHeapFlags flags;

			GEN_DEF_COMP(DescriptorHeapIndex);
		};


		struct DescriptorHeapContext
		{
			using HeapPageType = HAL::DescriptorHeap;
			using HeapMemoryOptions = DescriptorHeapIndex;

			static const size_t PageAlignment = 256;
		};

		using DescriptorHeapHandle = Allocators::HeapHandle<HAL::DescriptorHeap>;

		class DescriptorHeapStorage:public SharedObject<DescriptorHeapStorage>,public ObjectState<TrackedObjectState>, public TrackedObject
		{
			DescriptorHeapHandle handle;
			uint count = 0;
		public:

			DescriptorHeapStorage() = default;

			DescriptorHeapStorage(const  DescriptorHeapHandle &handle):handle(handle)
			{
				
			}
			~DescriptorHeapStorage()
			{
				handle.Free();
			}

			HAL::DescriptorHeap::ptr get_heap() const
			{
				return handle.get_heap();
			}

			bool is_valid() const
			{
				return handle;
			}
			uint get_offset() const
			{
				return handle.get_offset();
			}
			uint get_count() const
			{
				return handle.get_size();
			}
			
			bool can_free() const
			{
				return handle.CanFree();
			}
				std::shared_ptr<DescriptorHeapStorage> get_tracked()
			{
				return get_ptr<DescriptorHeapStorage>();
			}


		};
		DescriptorHeapType get_heap_type(HandleType type);

		
		struct Handle
		{
		
			Handle() = default;
			Handle(std::shared_ptr<DescriptorHeapStorage> storage,UINT offset):storage(storage),offset(offset){}
			ResourceInfo* get_resource_info() const;

			bool is_valid() const
			{
				return storage && (offset != UINT_MAX);
			}

			operator bool() const;


			bool operator!=(const Handle& r)
			{
				if (offset != r.offset) return true;
				return false;
			}

			void place(const Handle& r) const;

			template<HAL::Views::ViewTemplate T>
			void operator=(const T& v);

			uint get_count() const
			{
				if (!storage) return 0;
				assert(offset == 0);
				return storage->get_count();
			}

			void operator=(const Handle& r)
			{
				//				assert(r.heap);
				storage = r.storage;
				offset = r.offset;
			}
			D3D12_CPU_DESCRIPTOR_HANDLE get_cpu() const;

			D3D12_GPU_DESCRIPTOR_HANDLE get_gpu()const;
			Handle operator[](uint i) const
			{
				assert(offset == 0);
				assert(i<storage->get_count());

				return Handle( storage ,i);
			}
			inline uint get_offset()const
			{
				return storage->get_offset() + offset;
			}

			std::shared_ptr<DescriptorHeapStorage> get_storage()const
			{
				return storage;
			}
		private:
			std::shared_ptr<DescriptorHeapStorage> storage;
			UINT offset = UINT_MAX;
		};

	
		


		D3D12_CPU_DESCRIPTOR_HANDLE Handle::get_cpu()const
		{
			auto& heap = *storage->get_heap();

			return heap[get_offset()].get_cpu();
		}

		D3D12_GPU_DESCRIPTOR_HANDLE Handle::get_gpu()const
		{
			auto& heap = *storage->get_heap();

			return heap[get_offset()].get_gpu();
		}


		template<HAL::Views::ViewTemplate T>
		void Handle::operator=(const T& v)
		{
			auto &heap =* storage->get_heap();

			heap[get_offset()].place(v);
			*get_resource_info() = ResourceInfo(v);

		}





		class DescriptorHeapFactory :public Allocators::HeapFactory<DescriptorHeapContext, GlobalAllocationPolicy>
		{
			ptr_type gpu_sampler;
			ptr_type gpu_cbv_srv_uav;

			Device& device;
			virtual ptr_type make_heap(DescriptorHeapIndex index, size_t size) override
			{
				if(check(index.flags& DescriptorHeapFlags::ShaderVisible))
				{
					if(index.type== DescriptorHeapType::CBV_SRV_UAV)
						return gpu_cbv_srv_uav;
					else
						return gpu_sampler;
				}
				assert(index.type != DescriptorHeapType::CBV_SRV_UAV);
				return std::make_shared<HAL::DescriptorHeap>(device, size, index.type, index.flags);
			}

		public:
			DescriptorHeapFactory(Device& device) :device(device)
			{
				gpu_sampler = std::make_shared<HAL::DescriptorHeap>(device, 2048, DescriptorHeapType::SAMPLER, DescriptorHeapFlags::ShaderVisible);
				gpu_cbv_srv_uav = std::make_shared<HAL::DescriptorHeap>(device, 65536 * 8, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::ShaderVisible);

			}
			ptr_type get_sampler_heap()
			{
				return gpu_sampler;
			}

			ptr_type get_cbv_srv_uav_heap()
			{
				return gpu_cbv_srv_uav;
			}
		};
		template<class AllocationPolicy>
		class DescriptorHeapPageManager :public Allocators::HeapPageManager<DescriptorHeapContext, AllocationPolicy>
		{
			Device& device;
		public:
			DescriptorHeapPageManager(Device& _device) :Allocators::HeapPageManager<DescriptorHeapContext, AllocationPolicy>(_device.get_descriptor_heap_factory()), device(_device) {}


		};
	}

}