export module HAL:API.Heap;
import vulkan;
import Core;
import :Types;
import :Sampler;
import :Utils;
import :API.Device;

export namespace HAL
{
    namespace API
    {
        class Heap
        {
        protected:
            // VMA allocation backing this heap — backend-internal, not part of
            // the common HAL contract.  Only HAL::Heap (derived class) touches these.
            VmaAllocation  vma_allocation    = VK_NULL_HANDLE;
            VkDeviceMemory vk_memory         = VK_NULL_HANDLE;
            VkBuffer       heap_vk_buffer    = VK_NULL_HANDLE;
            VmaAllocator   vma_allocator_ref = VK_NULL_HANDLE;

            // DEFAULT heap memory, allocated on first use: by a placed resource
            // bound into it (Resource::init) or a sparse tile mapped to it.
            std::mutex     sparse_mutex;
            Device*        sparse_device = nullptr;
            VkDeviceSize   sparse_size   = 0;
            VmaAllocation  sparse_alloc  = VK_NULL_HANDLE;
            VkDeviceMemory sparse_memory = VK_NULL_HANDLE;
            VkDeviceSize   sparse_offset = 0;
            uint32_t       sparse_memory_type = ~0u;

        public:
            virtual ~Heap();
            uint64_t get_address() const;

            // Device memory (and offset into it) backing this DEFAULT heap: sparse
            // tiles and placed resources both bind into it, as in D3D12.
            std::pair<VkDeviceMemory, VkDeviceSize> get_sparse_memory();
            uint32_t     get_memory_type() { get_sparse_memory(); return sparse_memory_type; }
            VkDeviceSize get_heap_size() const { return sparse_size; }

            // The single VkBuffer backing a CPU-visible (UPLOAD/READBACK) heap.
            // Placed resources share it and address their slice via resource_offset.
            VkBuffer get_vk_buffer() const { return heap_vk_buffer; }

            // ---- Placement interface (public) --------------------------------
            // These are read by Resource::init(PlacementAddress) to derive the
            // CPU mapping and GPU address for placed resources (mirrors D3D12's
            // cpu_buffer->GetGPUVirtualAddress() / Map() pattern).
            std::byte* cpu_address = nullptr;
            uint64_t   gpu_address = 0;
        };
    }
}
