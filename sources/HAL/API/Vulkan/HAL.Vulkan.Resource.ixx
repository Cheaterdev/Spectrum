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
            VkBuffer      vk_buffer    = VK_NULL_HANDLE;
            VkImage       vk_image     = VK_NULL_HANDLE;
            VkImageView   vk_image_view= VK_NULL_HANDLE; // owned (non-swapchain) images
            VmaAllocation vma_alloc    = VK_NULL_HANDLE;

            // Cached per-mip image views for UAV (storage image) descriptors.
            // Keyed by (array_layer << 16 | mip_level). VkImageViews with levelCount=1
            // are required for VK_DESCRIPTOR_TYPE_STORAGE_IMAGE descriptors.
            mutable std::unordered_map<uint32_t, VkImageView> per_mip_views;

            // Format/aspect stored at image creation for per-mip view creation.
            VkFormat           vk_image_format = VK_FORMAT_UNDEFINED;
            VkImageAspectFlags vk_image_aspect = 0;

            // Full-resource image-view create-info, saved at init().  VK_EXT_descriptor_heap
            // image writes (VkImageDescriptorInfoEXT) take a VkImageViewCreateInfo rather
            // than a VkImageView, so the parameters must be reproducible here.
            VkImageViewCreateInfo vk_view_ci = {};

            // Persistent CPU mapping (UPLOAD / READBACK heaps).
            void* mapped_data = nullptr;

            // Externally-owned image handle (e.g. swapchain backbuffer).
            NativeImportHandle import_handle;

            // Pixel dimensions — set for both imported and owned images.
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
            VkImage     get_vk_image()        const noexcept { return vk_image; }
            VkBuffer    get_vk_buffer()       const noexcept { return vk_buffer; }

            // Backend-uniform native handle (see HAL.D3D12.Resource.ixx).
            // Prefers an imported (e.g. swapchain) image over an owned one.
            void* get_native() const noexcept
            {
                VkImage image = import_handle.image != VK_NULL_HANDLE ? import_handle.image : vk_image;
                return reinterpret_cast<void*>(image);
            }
            // Image aspect (DEPTH for depth images, COLOR otherwise) — used for
            // transfer/barrier subresources so depth images aren't touched as COLOR.
            VkImageAspectFlags get_vk_aspect() const noexcept {
                return vk_image_aspect ? vk_image_aspect : VK_IMAGE_ASPECT_COLOR_BIT;
            }
            // Primary image view: swapchain view takes priority, owned view as fallback.
            VkImageView get_vk_image_view()   const noexcept {
                return import_handle.image_view != VK_NULL_HANDLE
                     ? import_handle.image_view : vk_image_view;
            }
            // Single-mip image view for UAV (STORAGE_IMAGE) descriptors.
            // Lazily created and cached; requires levelCount=1 per Vulkan spec.
            VkImageView get_vk_mip_view(VkDevice vk_dev, uint32_t mip, uint32_t layer = 0) const noexcept;

            // Full-resource image-view create-info (for VK_EXT_descriptor_heap SRV writes).
            const VkImageViewCreateInfo& get_view_ci() const noexcept { return vk_view_ci; }
            // Single-mip create-info for UAV (STORAGE_IMAGE) descriptor writes.
            // A 3D storage image must keep a 3D view (a 2D view of a 3D image is
            // invalid without 2D_ARRAY_COMPATIBLE); 2D/array images use a 2D view
            // of one mip/layer.
            VkImageViewCreateInfo get_uav_view_ci(uint32_t mip, uint32_t layer = 0) const noexcept
            {
                VkImageViewCreateInfo ci = vk_view_ci;
                if (ci.viewType != VK_IMAGE_VIEW_TYPE_3D)
                {
                    ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    ci.subresourceRange.baseArrayLayer = layer;
                    ci.subresourceRange.layerCount     = 1;
                }
                ci.subresourceRange.baseMipLevel = mip;
                ci.subresourceRange.levelCount   = 1;
                return ci;
            }
            VkExtent2D  get_imported_extent() const noexcept { return imported_extent; }
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
