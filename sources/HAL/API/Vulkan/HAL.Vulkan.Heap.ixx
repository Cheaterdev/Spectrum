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
            VmaAllocation  vma_allocation = VK_NULL_HANDLE;
            VkDeviceMemory vk_memory      = VK_NULL_HANDLE;
            VkBuffer       heap_vk_buffer = VK_NULL_HANDLE;

        public:
            virtual ~Heap();
            uint64_t get_address() const;

            // ---- Placement interface (public) --------------------------------
            // These are read by Resource::init(PlacementAddress) to derive the
            // CPU mapping and GPU address for placed resources (mirrors D3D12's
            // cpu_buffer->GetGPUVirtualAddress() / Map() pattern).
            std::byte* cpu_address = nullptr;
            uint64_t   gpu_address = 0;
        };
    }
}
