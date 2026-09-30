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

        // Tile geometry of a sparse (HeapType::RESERVED) resource.
        struct SparseLayout
        {
            VkImageAspectFlags aspect          = 0;
            VkExtent3D         granularity     = { 1, 1, 1 };   // texels per 64 KB tile
            uint32_t           mip_tail_first  = 0;             // == mip count when there is no tail
            VkDeviceSize       mip_tail_size   = 0;
            VkDeviceSize       mip_tail_offset = 0;             // opaque offset of layer 0's tail
            VkDeviceSize       mip_tail_stride = 0;             // per-layer tail stride
            bool               single_mip_tail = false;         // one tail shared by all layers
            VkDeviceSize       size            = 0;             // buffers: bindable byte range
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
            VkDeviceSize  vk_buffer_offset = 0;
            VkImage       vk_image     = VK_NULL_HANDLE;
            VkImageView   vk_image_view= VK_NULL_HANDLE; // owned (non-swapchain) images
            VmaAllocation vma_alloc    = VK_NULL_HANDLE;

            // Cached single-mip image views for UAV (storage image) descriptors.
            // Keyed by (array_layer << 16 | mip_level).
            mutable std::unordered_map<uint32_t, VkImageView> per_mip_views;

            // Cached render-target / depth-stencil views of one mip and a layer
            // range. Keyed by mip | first_layer << 8 | layer_count << 32.
            mutable std::unordered_map<uint64_t, VkImageView> attachment_views;

            // Cached sampled-image (SRV) views: view type + mip range + layer range.
            mutable std::map<std::array<uint32_t, 5>, VkImageView> srv_views;
            bool cube_compatible = false;

            // Format/aspect stored at image creation for per-mip view creation.
            VkFormat           vk_image_format = VK_FORMAT_UNDEFINED;
            VkImageAspectFlags vk_image_aspect = 0;

            // Full-resource image-view create-info, saved at init(); per-mip UAV
            // views are derived from it.
            VkImageViewCreateInfo vk_view_ci = {};

            // Persistent CPU mapping (UPLOAD / READBACK heaps).
            void* mapped_data = nullptr;

            // Externally-owned image handle (e.g. swapchain backbuffer).
            NativeImportHandle import_handle;

            // Pixel dimensions — set for both imported and owned images.
            VkExtent2D imported_extent = {};

            // Sparse resources own their handle but have no VMA allocation: tile
            // memory comes from heaps, bound by Queue::update_tile_mappings.
            bool         sparse = false;
            SparseLayout sparse_layout;
            // Bound into a DEFAULT heap's memory: the heap owns it, destroy frees only the handle.
            bool         placed = false;
            void init_sparse_layout(Device& device, VkImageAspectFlags aspect);

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
            // Byte offset of this resource inside get_vk_buffer(): non-zero for a
            // buffer placed in an UPLOAD/READBACK heap, which shares the heap's
            // VkBuffer. Every offset handed to Vulkan must add it.
            VkDeviceSize get_vk_buffer_offset() const noexcept { return vk_buffer_offset; }
            bool                is_sparse()          const noexcept { return sparse; }
            const SparseLayout& get_sparse_layout()  const noexcept { return sparse_layout; }

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
            // View for a UAV (STORAGE_IMAGE) descriptor, created from get_uav_view_ci()
            // on first use and cached. Thread-safe: descriptors are placed from many threads.
            VkImageView get_vk_uav_view(VkDevice vk_dev, uint32_t mip, uint32_t layer = 0) const noexcept;

            // View for a render-target / depth-stencil attachment: one mip, a layer
            // range. An imported (swapchain) image returns its own view.
            VkImageView get_vk_attachment_view(VkDevice vk_dev, uint32_t mip, uint32_t first_layer, uint32_t layer_count) const noexcept;

            // View for an SRV (SAMPLED_IMAGE) descriptor: the view's own type, mips
            // and layers -- the full image view would read mip 0 for a per-mip SRV.
            // Counts may be VK_REMAINING_MIP_LEVELS / VK_REMAINING_ARRAY_LAYERS.
            VkImageView get_vk_srv_view(VkDevice vk_dev, VkImageViewType type, uint32_t first_mip, uint32_t mip_count,
                                        uint32_t first_layer, uint32_t layer_count) const noexcept;

            // Single-mip create-info for UAV (STORAGE_IMAGE) descriptors.
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
