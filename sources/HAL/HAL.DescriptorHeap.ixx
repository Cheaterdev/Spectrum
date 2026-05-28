export module HAL:DescriptorHeap;
import <HAL.h>;

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

			bool is_valid() const;
			ResourceInfo() = default;

			template<HAL::Views::ViewTemplate T>
			ResourceInfo(const T& v)
			{
				view = v;
			}

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

			virtual ~DescriptorHeap()
			{

			}
			ResourceInfo& get_resource_info(uint offset);
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

			DescriptorHeapStorage(const DescriptorHeapHandle& handle);
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
		

		using CBVHandle = internal::TypedHandle<HandleType::CBV>;
		using SRVHandle = internal::TypedHandle<HandleType::SRV>;
		using UAVHandle = internal::TypedHandle<HandleType::UAV>;
		using RTVHandle = internal::TypedHandle<HandleType::RTV>;
		using DSVHandle = internal::TypedHandle<HandleType::DSV>;
		using SamplerHandle = internal::TypedHandle<HandleType::SMP>;

		template<typename T> concept HandleClass = std::is_base_of_v<HAL::Handle, T>;



		template<HAL::Views::ViewTemplate T>
		void Handle::operator=(const T& v)
		{
			auto &heap =* storage->get_heap();

			heap[get_offset()].place(v);
			get_resource_info() = ResourceInfo(v);

		}





		class DescriptorHeapFactory :public Allocators::HeapFactory<DescriptorHeapContext, GlobalAllocationPolicy>
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
		class DescriptorHeapPageManager :public Allocators::HeapPageManager<DescriptorHeapContext, AllocationPolicy>
		{
			Device& device;
		public:
			DescriptorHeapPageManager(Device& _device) :Allocators::HeapPageManager<DescriptorHeapContext, AllocationPolicy>(_device.get_descriptor_heap_factory()), device(_device) {}


		};
	}

}