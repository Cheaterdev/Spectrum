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
            VkExtent2D  extent     = {};   // pixel dimensions of the image
        };

        class Resource
        {
            uint64_t address = 0;

            // CommandList needs vk_buffer/vk_image for barriers and clears.
            friend class CommandList;
            // Heap::Heap sets mapped_data / address on its backing buffer.
            friend class Heap;

        protected:
            // Vulkan resource handles — backend-internal.
            VkBuffer      vk_buffer = VK_NULL_HANDLE;
            VkImage       vk_image  = VK_NULL_HANDLE;
            VmaAllocation vma_alloc = VK_NULL_HANDLE;

            // Persistent CPU mapping (UPLOAD / READBACK heaps).
            void* mapped_data = nullptr;

            // Externally-owned image handle (e.g. swapchain backbuffer).
            NativeImportHandle import_handle;

            // Pixel dimensions for imported images.
            VkExtent2D imported_extent = {};

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

            // ---- Backend accessors (Vulkan handles for sibling modules) ------
            // Cross-partition friends are unreliable in MSVC; expose via getters.
            VkImage    get_vk_image()    const noexcept { return vk_image; }
            VkBuffer   get_vk_buffer()   const noexcept { return vk_buffer; }
            VkExtent2D get_imported_extent() const noexcept { return imported_extent; }
            const NativeImportHandle& get_import_handle() const noexcept { return import_handle; }
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
