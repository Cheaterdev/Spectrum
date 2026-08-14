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

		unsigned int offset = 0;
		GEN_DEF_COMP(HeapIndex);
	};
	
	struct ResourceContext
	{
		using HeapPageType = HAL::Heap;
		using HeapMemoryOptions = HeapIndex;

		static const size_t PageAlignment = 1_mb;

		static const size_t GarbageCount = 10;
	};


	using ResourceHandle = Allocators::PagedAllocation<HAL::Heap>;


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

		// The real D3D12 subresource (MipSlice + ArraySlice*MipLevels +
		// PlaneSlice*MipLevels*ArraySize) -- meaningful ONLY to the actual
		// D3D12 API call (D3D12_TILED_RESOURCE_COORDINATE::Subresource in
		// UpdateTileMappings). Never use this to index tiles[]/gpu_tiles[].
		UINT subresource;

		// Our own flat storage index into TiledResourceManager's
		// tiles[]/gpu_tiles[] (mip + array_slice*unpacked_mip_count, mip
		// varies fastest -- see TiledResourceManager::flat_index()).
		// Deliberately a SEPARATE field from `subresource` above: the two
		// coincide only when the resource has no packed mips (MipLevels ==
		// unpacked_mip_count) and PlaneSlice==0 -- conflating them (as this
		// code used to, indexing gpu_tiles[] by the real subresource) is
		// silently wrong whenever that's not true, and forced every caller of
		// the higher-level load_tiles()/zero_tiles()/is_mapped() API to
		// hand-compute a single combined index that meant "a mip" for some
		// resources and "an array slice times a mip count" for others.
		UINT storage_index;

		ivec3 size = ivec3(1, 1, 1);
		TileHeapPosition heap_position;
	};


	class HeapFactory :public Allocators::PageCache<ResourceContext, GlobalAllocationPolicy>
	{
		using Base = Allocators::PageCache<ResourceContext, GlobalAllocationPolicy>;

		Device& device;
		virtual ptr_type make_heap(HeapIndex index, size_t size) override;

		std::atomic<size_t> _upload_bytes   = 0;
		std::atomic<size_t> _readback_bytes = 0;

		void _add(HeapIndex index, size_t n) noexcept;
		void _sub(HeapIndex index, size_t n) noexcept;

	public:
		HeapFactory(Device& device);

		page_type AllocatePage(HeapIndex index, size_t size, Allocators::PageOwnerInterface<HAL::Heap>& owner) override;
		void      Free(HeapIndex index, page_type page) override;

		size_t get_upload_bytes()   const { return _upload_bytes.load(std::memory_order_relaxed); }
		size_t get_readback_bytes() const { return _readback_bytes.load(std::memory_order_relaxed); }
	};
	


	struct UploadInfo :public HAL::ResourceAddress
	{
		uint size;
	};


	template<class AllocationPolicy>
	class GPUMemoryAllocator:public Allocators::PagedAllocator<ResourceContext, AllocationPolicy>
	{

	public:
		using Allocators::PagedAllocator<ResourceContext, AllocationPolicy>::alloc;

		static constexpr uint DEFAULT_ALIGN = 256;

		GPUMemoryAllocator(Device&device):Allocators::PagedAllocator<ResourceContext, AllocationPolicy>(device.get_heap_factory())
		{
			
		}
	

	};


}

