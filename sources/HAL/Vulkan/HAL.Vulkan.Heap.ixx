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
            uint64_t     gpu_address = 0;
            std::byte*   cpu_address = nullptr;
        public:
            virtual ~Heap();

            uint64_t get_address() const;

        public:
            // Vulkan: memory is managed by VMA; the "heap" abstraction maps
            // to a VmaAllocation covering a large block of device memory.
            VmaAllocation vma_allocation = VK_NULL_HANDLE;
            VkDeviceMemory vk_memory     = VK_NULL_HANDLE;
        };
    }
}
