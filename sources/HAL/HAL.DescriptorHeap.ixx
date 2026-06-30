export module HAL:DescriptorHeap;


import :API.DescriptorHeap;
import :Types;
import Core;
export
{

	namespace HAL
	{
			   		constexpr uint ALL_SUBRESOURCES = std::numeric_limits<uint>::max();

		struct ResourceInfo
		{
			std::variant<HAL::Views::Null, HAL::Views::DepthStencil, HAL::Views::UnorderedAccess, HAL::Views::ShaderResource, HAL::Views::RenderTarget, HAL::Views::ConstantBuffer> view = HAL::Views::Null();

			bool is_valid() const;
			ResourceInfo() = default;

			template<HAL::Views::ViewTemplate T>
			ResourceInfo(const T& v) { view = v; }

			template<HAL::Views::ViewTemplate T>
			ResourceInfo(T&& v) { view = std::move(v); }


		   Resource* get_resource() const;
			void for_each_subres(std::function<void(const std::shared_ptr<Resource>&, UINT)> f) const;
		};

		class DescriptorHeap : public SharedObject<DescriptorHeap>, public API::DescriptorHeap, public TypedObject<DescriptorHeap>
		{
			friend struct Handle;
			std::vector<ResourceInfo> resources;

			UINT descriptor_size;

		public:
			using ptr = std::shared_ptr<DescriptorHeap>;

			DescriptorHeap(Device& device, uint num, DescriptorHeapType type, DescriptorHeapFlags flags = DescriptorHeapFlags::None);

			virtual ~DescriptorHeap() {}

			ResourceInfo& get_resource_info(uint offset);
			uint get_size();
		};

		class Descriptor : public API::Descriptor
		{
			DescriptorHeap& heap;
			const uint offset;

			Descriptor(DescriptorHeap& heap, uint offset);

			friend class API::DescriptorHeap;
		public:
			void operator=(const Descriptor& r);

			void place(const Views::ShaderResource& view);
			void place(const Views::UnorderedAccess& view);
			void place(const Views::RenderTarget& view);
			void place(const Views::ConstantBuffer& view);
			void place(const Views::DepthStencil& view);
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

		using DescriptorHeapAllocation = Allocators::PagedAllocation<HAL::DescriptorHeap>;

		class DescriptorHeapStorage:public SharedObject<DescriptorHeapStorage>,public ObjectState<TrackedObjectState>, public TrackedObject
		{
			DescriptorHeapAllocation handle;
			uint count = 0;
		public:

			DescriptorHeapStorage() = default;

			DescriptorHeapStorage(const DescriptorHeapAllocation& handle);
			~DescriptorHeapStorage();

			HAL::DescriptorHeap::ptr get_heap() const;
			bool is_valid() const;
			uint get_offset() const;
			uint get_count() const;
			bool can_free() const;
			std::shared_ptr<DescriptorHeapStorage> get_tracked();


		};
		DescriptorHeapType get_heap_type(HandleType type);

		
		struct Handle
		{
		
			Handle() = default;
			Handle(std::shared_ptr<DescriptorHeapStorage> storage, UINT offset);
			ResourceInfo& get_resource_info() const;

			bool is_valid() const;

			operator bool() const;

			bool operator!=(const Handle& r);

			void place(const Handle& r) const;

			template<HAL::Views::ViewTemplate T>
			void operator=(const T& v);

			template<HAL::Views::ViewTemplate T>
			void operator=(T&& v);

			uint get_count() const;

			void operator=(const Handle& r);

			D3D12_CPU_DESCRIPTOR_HANDLE get_cpu() const;

			D3D12_GPU_DESCRIPTOR_HANDLE get_gpu() const;

			Handle operator[](uint i) const;

			uint get_offset() const;

			std::shared_ptr<DescriptorHeapStorage> get_storage() const;
		protected:
			std::shared_ptr<DescriptorHeapStorage> storage;
			UINT offset = std::numeric_limits<uint>::max();
		};

		namespace internal // TODO oops, exported
		{
			template<HandleType handle_type>
		struct TypedHandle :public Handle
		{
			static const HandleType TYPE = handle_type;

			TypedHandle() = default;
			TypedHandle(std::shared_ptr<DescriptorHeapStorage> storage, UINT offset) :Handle(storage, offset) {}
			TypedHandle(const Handle& h) :Handle(h) {}
		
			TypedHandle operator[](uint i) const
			{
				ASSERT(offset == 0);
				ASSERT(i < storage->get_count());

				return TypedHandle(storage, i);
			}

		};
		}

		namespace  Handles
		{
			using CBV = internal::TypedHandle<HandleType::CBV>;
			using SRV = internal::TypedHandle<HandleType::SRV>;
			using UAV = internal::TypedHandle<HandleType::UAV>;
			using RTV = internal::TypedHandle<HandleType::RTV>;
			using DSV = internal::TypedHandle<HandleType::DSV>;
			using Sampler = internal::TypedHandle<HandleType::SMP>;
		}

		template<typename T> concept HandleClass = std::is_base_of_v<HAL::Handle, T>;



		template<HAL::Views::ViewTemplate T>
		void Handle::operator=(const T& v)
		{
			PROFILE(L"HLSL View");
			auto& heap = *storage->get_heap();
			heap[get_offset()].place(v);
			get_resource_info() = ResourceInfo(v);
		}

		template<HAL::Views::ViewTemplate T>
		void Handle::operator=(T&& v)
		{
			PROFILE(L"HLSL View");
			auto& heap = *storage->get_heap();
			heap[get_offset()].place(v);                      // read before move
			get_resource_info() = ResourceInfo(std::move(v)); // move shared_ptr into variant
		}





		class DescriptorHeapFactory :public Allocators::PageCache<DescriptorHeapContext, GlobalAllocationPolicy>
		{
			ptr_type gpu_sampler;
			ptr_type gpu_cbv_srv_uav;

			Device& device;
			virtual ptr_type make_heap(DescriptorHeapIndex index, size_t size) override;

		public:
			DescriptorHeapFactory(Device& device);
			ptr_type get_sampler_heap();
			ptr_type get_cbv_srv_uav_heap();
		};
		template<class AllocationPolicy>
		class DescriptorHeapPageManager :public Allocators::PagedAllocator<DescriptorHeapContext, AllocationPolicy>
		{
			Device& device;
		public:
			DescriptorHeapPageManager(Device& _device) :Allocators::PagedAllocator<DescriptorHeapContext, AllocationPolicy>(_device.get_descriptor_heap_factory()), device(_device) {}


		};
	}

}