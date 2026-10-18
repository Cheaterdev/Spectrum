module HAL:Resource;

import vulkan;
import Core;

import :Utils;          // to_native(Format)
import :API.Resource;   // API::Resource, PlacementAddress, NativeImportHandle; pulls in :Types, :API.Device
import :API.Heap;       // API::Heap::cpu_address, get_address()
import :Heap;           // HAL::Heap::get_type()
import :Device;         // HAL::Device
import :HeapAllocators;
import :FrameManager;

namespace HAL
{
    namespace API
    {
        GPUAddressPtr Resource::get_address() { return address; }

        void* Resource::get_cpu_mapping() { return mapped_data; }

        void Resource::init(Device& device, const ResourceDesc& _desc,
                            const PlacementAddress& placement, TextureLayout initialLayout)
        {
            auto THIS = static_cast<HAL::Resource*>(this);
            THIS->m_device = static_cast<HAL::Device*>(&device);
            THIS->desc = _desc;

            // Resolve MipLevels=0 → full mip chain.
            // D3D12 handles this natively inside CreateCommittedResource;
            // Vulkan requires mipLevels >= 1 and we must compute the count
            // ourselves before init_subres or vmaCreateImage sees the descriptor.
            if (THIS->desc.is_texture())
            {
                auto& t = THIS->desc.as_texture();
                if (t.MipLevels == 0)
                {
                    uint max_dim = std::max({ t.Dimensions.x,
                                              t.is1D() ? 1u : t.Dimensions.y,
                                              t.is3D() ? t.Dimensions.z : 1u });
                    t.MipLevels = max_dim > 0u
                        ? static_cast<uint>(std::floor(std::log2(
                              static_cast<float>(max_dim)))) + 1u
                        : 1u;
                }
            }

            // heap_type priority:
            //   1. If placement.heap is set, derive from that heap's type.
            //   2. Otherwise, preserve what _init() already wrote.
            //      _init(HeapType::DEFAULT) calls init() with a null placement
            //      but has pre-set heap_type = DEFAULT; overwriting it to RESERVED
            //      was the root bug — it caused every regular non-placed resource
            //      to take the tiled/reserved early-return path with no VkBuffer/Image.
            if (placement.heap)
                THIS->heap_type = placement.heap->get_type();
            // else: keep heap_type as set by caller (_init always pre-sets it)

            // Mirror D3D12's initialLayout defaulting logic: when the caller
            // passes UNDEFINED (the default), derive the initial layout from
            // the resource's usage flags so the CPU state manager starts in
            // the correct layout and compile_transitions() generates valid
            // oldLayout values in Vulkan barriers.
            if (_desc.is_texture())
            {
                if (THIS->heap_type == HeapType::UPLOAD)
                {
                    initialLayout = TextureLayout::SHADER_RESOURCE | TextureLayout::COPY_SOURCE;
                }
                else if (THIS->heap_type == HeapType::READBACK)
                {
                    initialLayout = TextureLayout::COPY_DEST;
                }
                else if (initialLayout == TextureLayout::UNDEFINED)
                {
                    if (check(_desc.Flags & ResFlags::ShaderResource))
                        initialLayout = TextureLayout::SHADER_RESOURCE;
                    else if (check(_desc.Flags & ResFlags::UnorderedAccess))
                        initialLayout = TextureLayout::UNORDERED_ACCESS;
                    else if (check(_desc.Flags & ResFlags::DepthStencil))
                        initialLayout = TextureLayout::DEPTH_STENCIL_WRITE | TextureLayout::DEPTH_STENCIL_READ;
                    else if (check(_desc.Flags & ResFlags::RenderTarget))
                        initialLayout = TextureLayout::RENDER_TARGET;
                    else
                        initialLayout = TextureLayout::COPY_DEST;
                }

                // (See the D3D12 path: the DisableStateTracking override is gone
                // with the flag, and frame_graph_managed is set post-creation.)
            }


            // Reserved resources are sparse: created without memory, tiles bound later.
            sparse = THIS->heap_type == HeapType::RESERVED;

            // Placed resources — two cases:
            //
            // A) UPLOAD / READBACK heap: the heap owns a persistently-mapped VMA
            //    buffer.  Derive cpu/gpu addresses from it (no separate allocation).
            //
            // B) DEFAULT heap: Vulkan has no D3D12-style placed resource without
            //    VK_KHR_bind_memory2 / explicit memory management.  Fall through
            //    to create a standalone VMA allocation instead.  The heap's memory
            //    region is intentionally ignored — the HeapType drives VMA flags.
            if (placement.heap)
            {
                auto* api_heap = static_cast<API::Heap*>(placement.heap);
                if (api_heap->cpu_address)
                {
                    // Case A: CPU-visible heap (UPLOAD / READBACK).
                    // Share the heap's single VkBuffer; the slice is addressed via
                    // resource_offset (placement.offset) at copy/descriptor time.
                    // Without this, get_vk_buffer() is null for every placed staging
                    // buffer / CBV / vertex-index buffer, so copies and descriptor
                    // writes are silently skipped and the GPU reads zeroes.
                    mapped_data = api_heap->cpu_address + placement.offset;
                    address     = api_heap->get_address() + placement.offset;
                    vk_buffer   = api_heap->get_vk_buffer();
                    vk_buffer_offset = placement.offset;
                    return;
                }
                // Case B: DEFAULT heap — bound into the heap's memory below
                // (bind_placed), so placed resources alias like in D3D12.
            }

            // Binds a freshly created image/buffer into placement.heap at
            // placement.offset. False when it can't be placed exactly where the
            // allocator put it (memory type, alignment, or a requirement larger
            // than the size get_alloc_info reserved); the caller then allocates
            // standalone rather than overlap the next resource in the heap.
            auto bind_placed = [&](const VkMemoryRequirements& req, auto&& bind) -> bool
            {
                if (!placement.heap || sparse) return false;
                auto* heap = static_cast<API::Heap*>(placement.heap);
                if (heap->cpu_address || THIS->heap_type != HeapType::DEFAULT) return false;

                const uint32_t type = heap->get_memory_type();
                const VkDeviceSize reserved = device.get_alloc_info(_desc).size;
                const bool fits = type < 32 && (req.memoryTypeBits & (1u << type))
                               && placement.offset % req.alignment == 0
                               && req.size <= std::max<VkDeviceSize>(reserved, 1)
                               && placement.offset + req.size <= heap->get_heap_size();
                if (!fits)
                {
                    static bool warned = false;
                    if (!warned) { warned = true; Log::get() << Log::LEVEL_WARNING << "[Vulkan] placed resource '" << _desc.is_texture()
                        << "' doesn't fit its heap slot (size " << req.size << " vs " << reserved << "), allocating standalone" << Log::endl; }
                    return false;
                }
                auto [memory, base] = heap->get_sparse_memory();
                if (bind(memory, base + placement.offset) != VK_SUCCESS) return false;
                placed = true;
                return true;
            };

            VmaAllocator allocator = device.get_vma_allocator();
            if (!allocator) return;

            VmaAllocationCreateInfo vma_ci{};

            if (_desc.is_buffer())
            {
                auto& buf = _desc.as_buffer();

                VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
                bci.size  = buf.SizeInBytes;
                bci.usage = device.buffer_usage;

                if (THIS->heap_type == HeapType::UPLOAD || THIS->heap_type == HeapType::READBACK)
                {
                    vma_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                 | VMA_ALLOCATION_CREATE_MAPPED_BIT;
                    vma_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
                }
                else
                {
                    vma_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
                }

                if (sparse)
                {
                    // Whole tiles: the last tile's bind must not run past the buffer.
                    bci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT;
                    bci.size  = Math::roundUp(bci.size, Device::SPARSE_TILE_SIZE);
                    VkResult r = vkCreateBuffer(device.get_native_device(), &bci, nullptr, &vk_buffer);
                    ASSERT(r == VK_SUCCESS && "sparse vkCreateBuffer failed");

                    VkMemoryRequirements req{};
                    vkGetBufferMemoryRequirements(device.get_native_device(), vk_buffer, &req);
                    sparse_layout.size = req.size;
                }
                else
                {
                    const VkDevice vk_dev = device.get_native_device();
                    if (placement.heap && vkCreateBuffer(vk_dev, &bci, nullptr, &vk_buffer) == VK_SUCCESS)
                    {
                        VkMemoryRequirements req{};
                        vkGetBufferMemoryRequirements(vk_dev, vk_buffer, &req);
                        if (!bind_placed(req, [&](VkDeviceMemory m, VkDeviceSize o) { return vkBindBufferMemory(vk_dev, vk_buffer, m, o); }))
                        {
                            vkDestroyBuffer(vk_dev, vk_buffer, nullptr);
                            vk_buffer = VK_NULL_HANDLE;
                        }
                    }

                    if (!placed)
                    {
                        VmaAllocationInfo alloc_info{};
                        vmaCreateBuffer(allocator, &bci, &vma_ci, &vk_buffer, &vma_alloc, &alloc_info);

                        if (THIS->heap_type == HeapType::UPLOAD || THIS->heap_type == HeapType::READBACK)
                            mapped_data = alloc_info.pMappedData;
                    }
                }

                if (vk_buffer != VK_NULL_HANDLE)
                {
                    VkBufferDeviceAddressInfo dai{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
                    dai.buffer = vk_buffer;
                    address = vkGetBufferDeviceAddress(device.get_native_device(), &dai);
                }
            }
            else if (_desc.is_texture())
            {
                // Read from THIS->desc, not _desc: the MipLevels=0 → full-chain
                // resolution above mutated the stored copy only.  vkCreateImage
                // requires mipLevels >= 1, so using the raw parameter here made
                // every MipLevels=0 texture silently fail creation.
                auto& tex = THIS->desc.as_texture();

                VkImageCreateInfo ici{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
                ici.imageType   = tex.is3D() ? VK_IMAGE_TYPE_3D :
                                  tex.is1D() ? VK_IMAGE_TYPE_1D : VK_IMAGE_TYPE_2D;
                ici.format      = to_native(tex.Format);
                // A depth resource may declare a typeless/color format (R32_TYPELESS,
                // R32_FLOAT) because it is also sampled.  Vulkan requires the image to
                // carry a genuine depth format so it matches the pipeline's
                // depthAttachmentFormat (mismatch = draw-time crash on the depth pass).
                if (check(_desc.Flags & ResFlags::DepthStencil))
                {
                    switch (ici.format)
                    {
                    case VK_FORMAT_R32_SFLOAT: ici.format = VK_FORMAT_D32_SFLOAT; break;
                    case VK_FORMAT_R16_SFLOAT:
                    case VK_FORMAT_R16_UNORM:   ici.format = VK_FORMAT_D16_UNORM;  break;
                    default: break; // already a depth format (D16/D24S8/D32/D32S8)
                    }
                }
                ici.extent      = { tex.Dimensions.x,
                                    tex.is1D() ? 1u : tex.Dimensions.y,
                                    tex.is3D() ? tex.Dimensions.z : 1u };
                ici.mipLevels   = tex.MipLevels;
                ici.arrayLayers = tex.ArraySize;
                ici.samples     = VK_SAMPLE_COUNT_1_BIT;
                ici.tiling      = VK_IMAGE_TILING_OPTIMAL;
                ici.usage       = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

                // STORAGE only on request — mirrors D3D12's ALLOW_UNORDERED_ACCESS.
                // Forcing it on every image breaks formats without storage-image
                // support (e.g. B8G8R8A8_UNORM on NVIDIA): vkCreateImage fails and
                // the resource silently becomes a null no-op.
                if (check(_desc.Flags & ResFlags::UnorderedAccess))
                    ici.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
                if (check(_desc.Flags & ResFlags::RenderTarget))
                    ici.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                if (check(_desc.Flags & ResFlags::DepthStencil))
                    ici.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

                // D3D12 accepts a Cube SRV on any square 2D array of 6N layers;
                // Vulkan needs the flag at creation.
                cube_compatible = !sparse && ici.imageType == VK_IMAGE_TYPE_2D && ici.arrayLayers % 6 == 0
                               && ici.extent.width == ici.extent.height;
                if (cube_compatible)
                    ici.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

                // D3D12 reinterprets a UNORM/TYPELESS texture through an _SRGB SRV
                // (Texture::texture_2d_srgb); Vulkan needs the flag to view it so.
                if (tex.Format.to_srgb() != tex.Format)
                    ici.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

                vma_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

                VkResult img_res;
                if (sparse)
                {
                    ici.flags |= VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
                    img_res = vkCreateImage(device.get_native_device(), &ici, nullptr, &vk_image);
                    if (img_res == VK_SUCCESS)
                        init_sparse_layout(device, check(_desc.Flags & ResFlags::DepthStencil)
                            ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT);
                }
                else
                {
                    const VkDevice vk_dev = device.get_native_device();
                    img_res = VK_ERROR_UNKNOWN;
                    if (placement.heap && vkCreateImage(vk_dev, &ici, nullptr, &vk_image) == VK_SUCCESS)
                    {
                        VkMemoryRequirements req{};
                        vkGetImageMemoryRequirements(vk_dev, vk_image, &req);
                        if (bind_placed(req, [&](VkDeviceMemory m, VkDeviceSize o) { return vkBindImageMemory(vk_dev, vk_image, m, o); }))
                            img_res = VK_SUCCESS;
                        else
                        {
                            vkDestroyImage(vk_dev, vk_image, nullptr);
                            vk_image = VK_NULL_HANDLE;
                        }
                    }
                    if (!placed)
                        img_res = vmaCreateImage(allocator, &ici, &vma_ci, &vk_image, &vma_alloc, nullptr);
                }
                if (img_res != VK_SUCCESS)
                    Log::get() << "[Vulkan] vmaCreateImage failed (" << static_cast<int>(img_res)
                               << ") vk_format=" << static_cast<int>(ici.format)
                               << " usage=" << static_cast<uint>(ici.usage)
                               << " extent=" << ici.extent.width << "x" << ici.extent.height << "x" << ici.extent.depth
                               << " mips=" << ici.mipLevels << " layers=" << ici.arrayLayers
                               << " — resource will be a null no-op" << Log::endl;

                if (vk_image != VK_NULL_HANDLE)
                {
                    // Store pixel dimensions so get_imported_extent() works for all images.
                    imported_extent = { ici.extent.width, ici.extent.height };

                    // Create a full-resource image view.
                    bool is_depth = check(_desc.Flags & ResFlags::DepthStencil);
                    vk_image_format = ici.format;
                    vk_image_aspect = is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

                    VkImageViewCreateInfo ivci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
                    ivci.image    = vk_image;
                    ivci.viewType = tex.is3D()            ? VK_IMAGE_VIEW_TYPE_3D
                                  : tex.is1D()            ? VK_IMAGE_VIEW_TYPE_1D
                                  : tex.ArraySize > 1     ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                                                          : VK_IMAGE_VIEW_TYPE_2D;
                    ivci.format   = ici.format;
                    ivci.subresourceRange.aspectMask     = vk_image_aspect;
                    ivci.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
                    ivci.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
                    vkCreateImageView(device.get_native_device(), &ivci, nullptr, &vk_image_view);
                    vk_view_ci = ivci;

                    // The state manager records `initialLayout` as the layout this
                    // image rests in between command lists, but a fresh VkImage is
                    // actually UNDEFINED.  Queue the one-time transition that makes
                    // the assumption true (flushed before the next queue submit).
                    device.queue_initial_transition(vk_image, to_native(initialLayout),
                        is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT);
                }
            }

            if (sparse)
                THIS->tiled_manager.init_tilings();
        }

        void Resource::init_sparse_layout(Device& device, VkImageAspectFlags aspect)
        {
            uint32_t count = 0;
            vkGetImageSparseMemoryRequirements(device.get_native_device(), vk_image, &count, nullptr);
            std::vector<VkSparseImageMemoryRequirements> reqs(count);
            vkGetImageSparseMemoryRequirements(device.get_native_device(), vk_image, &count, reqs.data());

            for (auto& r : reqs)
            {
                // Metadata needs its own permanently bound mip tail; no current
                // device/format in use reports one, so it is not handled.
                ASSERT(!(r.formatProperties.aspectMask & VK_IMAGE_ASPECT_METADATA_BIT) && "sparse metadata aspect unsupported");

                if (!(r.formatProperties.aspectMask & aspect)) continue;

                sparse_layout.aspect          = aspect;
                sparse_layout.granularity     = r.formatProperties.imageGranularity;
                sparse_layout.mip_tail_first  = r.imageMipTailFirstLod;
                sparse_layout.mip_tail_size   = r.imageMipTailSize;
                sparse_layout.mip_tail_offset = r.imageMipTailOffset;
                sparse_layout.mip_tail_stride = r.imageMipTailStride;
                sparse_layout.single_mip_tail = r.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT;
                return;
            }
            ASSERT(!"sparse image has no requirements for its aspect");
        }

        void Resource::init(const NativeImportHandle& handle, TextureLayout layout, Device& device)
        {
            auto THIS = static_cast<HAL::Resource*>(this);
            THIS->m_device = static_cast<HAL::Device*>(&device);

            import_handle   = handle;
            vk_image        = handle.image;
            imported_extent = handle.extent;

            // Build a proper 2D TextureDesc so as_texture() works in common code.
            // Mirrors D3D12's Resource::init(NativeImportHandle) which calls extract().
            auto fmt = from_native(handle.format);
            THIS->desc = ResourceDesc::Tex2D(
                fmt,
                uint2(handle.extent.width, handle.extent.height),
                1,   // ArraySize
                1    // MipLevels
            );
            THIS->desc.Flags |= ResFlags::RenderTarget;
            if (layout == TextureLayout::PRESENT)
                THIS->desc.Flags |= ResFlags::Swapchain;

        }
    }

    void Resource::_init(Device& device, const ResourceDesc& desc, HeapType heap_type,
                         TextureLayout initialLayout, vec4 /*clear_value*/)
    {
        m_device = &device;
        this->heap_type = heap_type;
        alloc_info = device.get_alloc_info(desc);

        PlacementAddress address = {};
        init(device, desc, address, initialLayout);
    }

    Resource::Resource(Device& device, const ResourceDesc& desc, HeapType heap_type,
                       TextureLayout initialLayout, vec4 clear_value)
        : tiled_manager(this)
    {
        _init(device, desc, heap_type, initialLayout, clear_value);
    }

    Resource::Resource(Device& device, const ResourceDesc& desc, ResourceHandle handle, bool own)
        : tiled_manager(this)
    {
        m_device = &device;
        PlacementAddress address = { handle.get_heap().get(), handle.get_offset() };
        init(device, desc, address, TextureLayout::UNDEFINED);
        if (own)
            alloc_handle = handle;
    }

    Resource::Resource(Device& device, const ResourceDesc& desc, PlacementAddress address)
        : tiled_manager(this)
    {
        m_device = &device;
        init(device, desc, address, TextureLayout::UNDEFINED);
    }

    Resource::Resource(Device& device, const API::NativeImportHandle& handle, TextureLayout initialLayout)
        : tiled_manager(this)
    {
        m_device = &device;
        init(handle, initialLayout, device);
    }

    void Resource::set_name(std::string name)
    {
        if (!this->name.empty() && name.empty()) return;
        this->name = name;
        if (!m_device) return;
        auto& api_dev = static_cast<API::Device&>(*m_device);
        VkDevice vk_dev = api_dev.get_native_device();
        if (vk_dev == VK_NULL_HANDLE) return;

        auto fn = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
            vkGetDeviceProcAddr(vk_dev, "vkSetDebugUtilsObjectNameEXT"));
        if (!fn) return;

        VkDebugUtilsObjectNameInfoEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
        info.pObjectName = this->name.c_str();

        if (vk_image != VK_NULL_HANDLE)
        {
            info.objectType   = VK_OBJECT_TYPE_IMAGE;
            info.objectHandle = reinterpret_cast<uint64_t>(vk_image);
            fn(vk_dev, &info);
            if (vk_image_view != VK_NULL_HANDLE)
            {
                info.objectType   = VK_OBJECT_TYPE_IMAGE_VIEW;
                info.objectHandle = reinterpret_cast<uint64_t>(vk_image_view);
                fn(vk_dev, &info);
            }
        }
        else if (vk_buffer != VK_NULL_HANDLE)
        {
            info.objectType   = VK_OBJECT_TYPE_BUFFER;
            info.objectHandle = reinterpret_cast<uint64_t>(vk_buffer);
            fn(vk_dev, &info);
        }
    }

    namespace API
    {
        // Guards every Resource's per_mip_views; lookups after the first are cheap.
        static std::mutex per_mip_views_mutex;

        VkImageView Resource::get_vk_uav_view(VkDevice vk_dev, uint32_t mip, uint32_t layer) const noexcept
        {
            if (vk_image == VK_NULL_HANDLE || vk_view_ci.image == VK_NULL_HANDLE)
                return VK_NULL_HANDLE;

            const uint32_t key = (layer << 16) | mip;
            std::lock_guard g(per_mip_views_mutex);
            auto it = per_mip_views.find(key);
            if (it != per_mip_views.end())
                return it->second;

            VkImageViewCreateInfo ivci = get_uav_view_ci(mip, layer);
            VkImageView v = VK_NULL_HANDLE;
            vkCreateImageView(vk_dev, &ivci, nullptr, &v);
            per_mip_views[key] = v;
            return v;
        }

        VkImageView Resource::get_vk_srv_view(VkDevice vk_dev, VkImageViewType type, uint32_t first_mip, uint32_t mip_count,
                                              uint32_t first_layer, uint32_t layer_count, VkFormat format) const noexcept
        {
            if (import_handle.image != VK_NULL_HANDLE)
                return import_handle.image_view;
            if (vk_image == VK_NULL_HANDLE || vk_view_ci.image == VK_NULL_HANDLE)
                return VK_NULL_HANDLE;
            if (format == VK_FORMAT_UNDEFINED)
                format = vk_view_ci.format;

            const std::array<uint32_t, 6> key = { uint32_t(type), first_mip, mip_count, first_layer, layer_count, uint32_t(format) };
            std::lock_guard g(per_mip_views_mutex);
            if (auto it = srv_views.find(key); it != srv_views.end())
                return it->second;

            if (!cube_compatible && (type == VK_IMAGE_VIEW_TYPE_CUBE || type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY))
                type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            // A single-layer view of a 3D image stays 3D (a 2D view would need 2D_ARRAY_COMPATIBLE).
            if (vk_view_ci.viewType == VK_IMAGE_VIEW_TYPE_3D)
            {
                type = VK_IMAGE_VIEW_TYPE_3D;
                first_layer = 0;
                layer_count = 1;
            }

            VkImageViewCreateInfo ivci = vk_view_ci;
            ivci.viewType = type;
            ivci.subresourceRange.baseMipLevel   = first_mip;
            ivci.subresourceRange.levelCount     = mip_count;
            ivci.subresourceRange.baseArrayLayer = first_layer;
            ivci.subresourceRange.layerCount     = layer_count;
            // A reinterpreting view (sRGB) inherits the image's usage by default,
            // which may include STORAGE the view format can't support.
            VkImageViewUsageCreateInfo usage{ VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO };
            usage.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
            if (format != vk_view_ci.format)
            {
                ivci.format = format;
                usage.pNext = ivci.pNext;
                ivci.pNext  = &usage;
            }
            VkImageView v = VK_NULL_HANDLE;
            vkCreateImageView(vk_dev, &ivci, nullptr, &v);
            srv_views[key] = v;
            return v;
        }

        VkImageView Resource::get_vk_attachment_view(VkDevice vk_dev, uint32_t mip, uint32_t first_layer, uint32_t layer_count) const noexcept
        {
            if (import_handle.image != VK_NULL_HANDLE)
                return import_handle.image_view;
            if (vk_image == VK_NULL_HANDLE || vk_view_ci.image == VK_NULL_HANDLE)
                return VK_NULL_HANDLE;

            // Rendering to slices of a 3D image needs a 2D-array view, which the
            // image must have been created 2D_ARRAY_COMPATIBLE for; nothing does that.
            ASSERT(vk_view_ci.viewType != VK_IMAGE_VIEW_TYPE_3D && "render target on a 3D texture is unsupported");

            const uint64_t key = uint64_t(mip) | uint64_t(first_layer) << 8 | uint64_t(layer_count) << 32;
            std::lock_guard g(per_mip_views_mutex);
            auto it = attachment_views.find(key);
            if (it != attachment_views.end())
                return it->second;

            VkImageViewCreateInfo ivci = vk_view_ci;
            ivci.viewType = layer_count > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            ivci.subresourceRange.baseMipLevel   = mip;
            ivci.subresourceRange.levelCount     = 1;
            ivci.subresourceRange.baseArrayLayer = first_layer;
            ivci.subresourceRange.layerCount     = layer_count;
            VkImageView v = VK_NULL_HANDLE;
            vkCreateImageView(vk_dev, &ivci, nullptr, &v);
            attachment_views[key] = v;
            return v;
        }
    }

    Resource::~Resource()
    {
        alloc_handle.Free();

        if (vma_alloc || sparse || placed)
        {
            auto& api_dev = static_cast<API::Device&>(*m_device);
            VkDevice vk_dev = api_dev.get_native_device();

            // Cancel any pending UNDEFINED→initial transition queued at creation
            // time — if the image is destroyed before Queue::execute() flushes the
            // batch, the barrier would reference a freed handle and crash the driver.
            api_dev.cancel_pending_init_transition(vk_image);

            // Destroy per-mip UAV views before destroying the image.
            for (auto& [key, view] : per_mip_views)
                if (view != VK_NULL_HANDLE)
                    vkDestroyImageView(vk_dev, view, nullptr);
            per_mip_views.clear();
            for (auto& [key, view] : attachment_views)
                if (view != VK_NULL_HANDLE)
                    vkDestroyImageView(vk_dev, view, nullptr);
            attachment_views.clear();
            for (auto& [key, view] : srv_views)
                if (view != VK_NULL_HANDLE)
                    vkDestroyImageView(vk_dev, view, nullptr);
            srv_views.clear();

            // Destroy the owned image view before destroying the image itself.
            if (vk_image_view != VK_NULL_HANDLE && !import_handle.image)
            {
                vkDestroyImageView(vk_dev, vk_image_view, nullptr);
                vk_image_view = VK_NULL_HANDLE;
            }

            // Acceleration-structure objects over this buffer must go first.
            if (vk_buffer != VK_NULL_HANDLE)
                api_dev.forget_acceleration_structures(vk_buffer);

            if (placed)
            {
                if (vk_buffer != VK_NULL_HANDLE) vkDestroyBuffer(vk_dev, vk_buffer, nullptr);
                else if (vk_image != VK_NULL_HANDLE) vkDestroyImage(vk_dev, vk_image, nullptr);
            }
            else if (vk_buffer != VK_NULL_HANDLE)
            {
                vmaDestroyBuffer(api_dev.get_vma_allocator(), vk_buffer, vma_alloc);
            }
            else if (vk_image != VK_NULL_HANDLE && !import_handle.image)
                vmaDestroyImage(api_dev.get_vma_allocator(), vk_image, vma_alloc);
            vma_alloc = VK_NULL_HANDLE;
        }
    }

}
