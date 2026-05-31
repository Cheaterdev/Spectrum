export module HAL:API.Resource;
import vulkan;
import Core;

import :Types;
import :Sampler;
import :Utils;
import :API.Device;
import :Heap;

import :Format;

export namespace HAL
{
    struct PlacementAddress
    {
        Heap* heap;
        size_t offset;
    };

    namespace API
    {
        // NativeImportHandle: opaque wrapping of a backend-native resource for
        // importing externally-managed images (e.g. swapchain back-buffers).
        // D3D12 version wraps ComPtr<ID3D12Resource>;
        // Vulkan version wraps VkImage + view + format.
        struct NativeImportHandle
        {
            VkImage     image      = VK_NULL_HANDLE;
            VkImageView image_view = VK_NULL_HANDLE;
            VkFormat    format     = VK_FORMAT_UNDEFINED;
        };

        class Resource
        {
            uint64_t address = 0;
        public:
            using ptr = std::shared_ptr<Resource>;

            void init(Device& device, const ResourceDesc& desc,
                      const PlacementAddress& address,
                      TextureLayout initialLayout = TextureLayout::UNDEFINED);
            void init(const NativeImportHandle& handle,
                      TextureLayout layout,
                      Device& device);

            uint64_t get_address();

            // CPU mapping (for UPLOAD / READBACK heaps).
            void* get_cpu_mapping();

        public:
            // Vulkan resource storage
            VkBuffer      vk_buffer = VK_NULL_HANDLE;
            VkImage       vk_image  = VK_NULL_HANDLE;
            VmaAllocation vma_alloc = VK_NULL_HANDLE;

            // Cached mapped pointer (set during init for upload/readback heaps)
            void* mapped_data = nullptr;

            // Import handle cached for externally-owned images (swapchain)
            NativeImportHandle import_handle;
        };
    }
}

export
{
    namespace cereal
    {
        template<class Archive>
        void serialize(Archive& ar, HAL::API::Resource*& g) {}
    }
}
