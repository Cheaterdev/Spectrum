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

		// One view bound into a table slot, plus how the barrier system should
		// scope it. Separate from ResourceInfo because the scope belongs to the
		// BINDING, not the view: the same descriptor can be bound by one table
		// that wants whole-resource barriers and another that does not.
		struct BoundResource
		{
			ResourceInfo* info = nullptr;

			// [Barrier = ALL] on the .sig member. Transition the WHOLE resource
			// instead of the mip/array range this view names.
			//
			// For a view narrowed to one mip of a big array (a Hi-Z pyramid
			// level over N slices), the range costs one barrier per slice per
			// bind while the neighbouring levels are being written anyway. One
			// whole-resource entry is both cheaper and what the surrounding
			// code actually wants -- which is why several call sites used to
			// hand-write a bare add_resource_usage() next to the bind.
			bool whole_resource = false;
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

			void place(const Views::ShaderResource& view, bool skip_gpu_write = false);
			void place(const Views::UnorderedAccess& view, bool skip_gpu_write = false);
			void place(const Views::RenderTarget& view, bool skip_gpu_write = false);
			void place(const Views::ConstantBuffer& view, bool skip_gpu_write = false);
			void place(const Views::DepthStencil& view, bool skip_gpu_write = false);
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
			bool deferred_gpu_write = false;
		public:

			DescriptorHeapStorage() = default;

			DescriptorHeapStorage(const DescriptorHeapAllocation& handle, bool deferred_gpu_write = false);
			~DescriptorHeapStorage();

			HAL::DescriptorHeap::ptr get_heap() const;
			bool is_valid() const;
			uint get_offset() const;
			uint get_count() const;
			bool can_free() const;
			bool is_deferred() const { return deferred_gpu_write; }
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

			// True once .create()'s underlying operator=(view_desc) has
			// actually written a descriptor into this slot (see the
			// `written` member's own comment below). Used at the point a
			// NAMED view field (texture2D, texture2DArray, rwTexture2D, ...)
			// gets pulled out of one struct and bound into another (the
			// HasTexture2D-etc.-constrained operator= overloads in
			// HAL.HLSL.ixx) -- that is specifically "about to be sampled by
			// a shader", unlike a generic whole-object copy of a multi-view
			// container (e.g. StructuredBufferView's move-assignment
			// memberwise-copying every sub-handle, most of which are
			// structurally inapplicable -- e.g. a raygen SBT record's
			// never-created TextureCube slot -- and never read by anything).
			bool is_written() const { return written; }
		protected:
			std::shared_ptr<DescriptorHeapStorage> storage;
			UINT offset = std::numeric_limits<uint>::max();

			// Set only by operator=(const T&)/operator=(T&&) below -- the
			// actual "write the descriptor into the heap slot" step (.create()
			// on Texture2D/Texture2DArray/RWTexture2D/etc. bottoms out here).
			// A Texture2DView (and friends) always hands out a Handle for
			// every possible view kind regardless of whether the owning
			// resource's ResFlags actually support it (see
			// Texture2DView::init(), HAL.ResourceViews.cpp) -- only the
			// flag-gated .create() call actually writes content into that
			// heap slot. Without this flag, a resource created with e.g.
			// DepthStencil but not ShaderResource silently hands shaders a
			// perfectly valid-looking, bindable, but NEVER-WRITTEN descriptor
			// index -- no D3D12 validation error, just whatever unrelated
			// resource's content last occupied that heap slot (root-caused a
			// real bug: VSM_Atlas sampled GUI text). See
			// operator=(const Handle&)'s ASSERT below, which is where this
			// gets caught -- at the point a view is actually bound for
			// shader use.
			bool written = false;
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
			heap[get_offset()].place(v, storage->is_deferred());
			get_resource_info() = ResourceInfo(v);
			written = true;
		}

		template<HAL::Views::ViewTemplate T>
		void Handle::operator=(T&& v)
		{
			PROFILE(L"HLSL View");
			auto& heap = *storage->get_heap();
			heap[get_offset()].place(v, storage->is_deferred());  // read before move
			get_resource_info() = ResourceInfo(std::move(v)); // move shared_ptr into variant
			written = true;
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

		// ---- null descriptors -------------------------------------------------
		//
		// Engine-lifetime descriptors that name no resource. D3D12 reads them as
		// zero, which is what [Auto = Texture_Null] binds into a table member
		// nothing assigned -- instead of leaving the slot at descriptor index 0,
		// which is a REAL descriptor belonging to whatever happens to sit at the
		// start of the heap.
		//
		// Cached per (view dimension, format): the descriptor has to agree with
		// the type the shader declares, and a Texture2D<float> reading a
		// Texture3D null descriptor is the same class of mismatch this exists to
		// prevent.
		//
		// init() must run once at device setup, before any table compiles. It
		// takes the Device because the descriptors are allocated from static GPU
		// data -- they must outlive every frame, so a per-frame linear allocator
		// would be wrong.
		void init_null_descriptors(Device& device);

		// Never pushes the descriptor's ResourceInfo into a bound-resource list:
		// there is no resource, so there is nothing to transition. See
		// Slot_Compiler::compile_auto.
		const Handle& get_null_descriptor(const Views::ShaderResource& proto);
	}

}