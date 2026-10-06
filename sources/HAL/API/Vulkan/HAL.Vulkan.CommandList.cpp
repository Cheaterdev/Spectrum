module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
module HAL:API.CommandList;

import stl.core;
import Core;
import :CommandAllocator;   // full definition needed: allocator.vk_command_pool
import :RootSignature;      // HAL::RootSignature (owns no Vulkan objects)
import :API.Device;         // API::Device::get_native_device()
import :API.DescriptorHeap; // API::DescriptorHeap::get_vk_set()
import :API.QueryHeap;      // API::QueryHeap::get_native()

namespace HAL::API
{
    // The mip and array slices an RTV/DSV names. Binding the resource's full view
    // instead renders into (and clears) the wrong subresources -- e.g. every VSM
    // atlas page instead of the one being drawn.
    struct Attachment
    {
        VkImage     image  = VK_NULL_HANDLE;
        VkImageView view   = VK_NULL_HANDLE;
        VkExtent2D  extent = {};
        uint32_t    mip = 0, first_layer = 0, layers = 1;
    };

    static Attachment resolve_attachment(VkDevice vk_dev, const Handle& h)
    {
        Attachment a;
        if (!h.is_valid()) return a;

        const auto& ri = h.get_resource_info();
        std::shared_ptr<HAL::Resource> res;
        auto slices = [&](uint mip, uint first, uint count) { a.mip = mip; a.first_layer = first; a.layers = std::max(1u, count); };

        if (auto* rtv = std::get_if<Views::RenderTarget>(&ri.view))
        {
            res = rtv->Resource;
            std::visit(overloaded{
                [&](const Views::RenderTarget::Texture2D& v)      { slices(v.MipSlice, 0, 1); },
                [&](const Views::RenderTarget::Texture2DArray& v) { slices(v.MipSlice, v.FirstArraySlice, v.ArraySize); },
                [&](const Views::RenderTarget::Texture1D& v)      { slices(v.MipSlice, 0, 1); },
                [&](const Views::RenderTarget::Texture1DArray& v) { slices(v.MipSlice, v.FirstArraySlice, v.ArraySize); },
                [&](const auto&) { ASSERT(!"unsupported render target view on Vulkan"); },
            }, rtv->View);
        }
        else if (auto* dsv = std::get_if<Views::DepthStencil>(&ri.view))
        {
            res = dsv->Resource;
            std::visit(overloaded{
                [&](const Views::DepthStencil::Texture2D& v)      { slices(v.MipSlice, 0, 1); },
                [&](const Views::DepthStencil::Texture2DArray& v) { slices(v.MipSlice, v.FirstArraySlice, v.ArraySize); },
                [&](const Views::DepthStencil::Texture1D& v)      { slices(v.MipSlice, 0, 1); },
                [&](const Views::DepthStencil::Texture1DArray& v) { slices(v.MipSlice, v.FirstArraySlice, v.ArraySize); },
                [&](const auto&) { ASSERT(!"unsupported depth-stencil view on Vulkan"); },
            }, dsv->View);
        }
        if (!res) return a;

        auto& api = static_cast<API::Resource&>(*res);
        a.image = api.get_import_handle().image != VK_NULL_HANDLE ? api.get_import_handle().image : api.get_vk_image();
        a.view  = api.get_vk_attachment_view(vk_dev, a.mip, a.first_layer, a.layers);

        if (res->get_desc().is_texture())
        {
            const auto& td = res->get_desc().as_texture();
            a.extent = { std::max(1u, td.Dimensions.x >> a.mip), std::max(1u, td.Dimensions.y >> a.mip) };
        }
        else
            a.extent = api.get_imported_extent();
        return a;
    }

    static VkExtent3D mip_extent(const TextureDesc& t, uint mip)
    {
        return { std::max(1u, t.Dimensions.x >> mip),
                 t.is1D() ? 1u : std::max(1u, t.Dimensions.y >> mip),
                 t.is3D() ? std::max(1u, t.Dimensions.z >> mip) : 1u };
    }

    void CommandList::create(CommandListType t, Device& dev)
    {
        type     = t;
        m_device = &dev;
        // VkCommandBuffer is allocated lazily in begin() from the allocator's pool.
    }

    void CommandList::begin(HAL::CommandAllocator& allocator)
    {
        auto& api_dev   = static_cast<API::Device&>(*m_device);
        auto& api_alloc = static_cast<API::CommandAllocator&>(allocator);

        // Lock the pool mutex for the entire recording session (held until end()).
        _pool_mutex = &api_alloc.pool_mutex;
        _pool_mutex->lock();

        // Like a D3D12 allocator, the pool's buffers are reused only after its
        // reset(), so the buffer we get here is not in flight.
        vk_cmd      = api_alloc.acquire(api_dev.get_native_device());
        vk_cmd_pool = api_alloc.vk_command_pool;

        VkCommandBufferBeginInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(vk_cmd, &info);

        // The barrier compiler (compile_transitions) is written against D3D12's
        // ExecuteCommandLists scope: at a group's first touch it skips a barrier
        // whose layout already matches, or emits SyncBefore=NONE, because on D3D12
        // everything submitted earlier on the queue has retired and is visible by
        // then. Vulkan gives no such guarantee between submissions -- earlier
        // batches may still be running and their writes are not made visible --
        // so recreate that boundary here. Without it: bloom flicker, GBuffer data
        // in freshly aliased targets, garbage after resize.
        {
            VkMemoryBarrier2 scope{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
            scope.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            scope.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
            scope.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            scope.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
            VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
            dep.memoryBarrierCount = 1;
            dep.pMemoryBarriers    = &scope;
            vkCmdPipelineBarrier2(vk_cmd, &dep);
        }

        // Fresh recording: clear any deferred PRESENT barriers from the previous frame.
        deferred_present_barriers.clear();

        // Descriptor set bindings don't carry over into a new command buffer.
        heaps_dirty = resource_set != VK_NULL_HANDLE;

        // This list object is pooled: without the reset, a root slot this recording
        // never sets would keep a descriptor index from whichever pass used the
        // object last, and push_full_constants() re-sends that stale index after
        // every command-buffer split -- a shader then reads or writes through a
        // descriptor that may now name a freed resource.
        // Same for the bound pipelines: a fresh command buffer has none, and
        // reapply_draw_state() would otherwise re-bind the last recording's.
        current_graphics_pipeline = VK_NULL_HANDLE;
        current_compute_pipeline  = VK_NULL_HANDLE;

        // COPY lists run no shaders, and their transfer-only pool rejects push constants.
        push_constants.fill(0);
        if (type != CommandListType::COPY)
            push_full_constants();
    }

    void CommandList::end()
    {
        if (vk_cmd != VK_NULL_HANDLE)
        {
            end_rendering_if_active();

            // Flush any deferred PRESENT_SRC_KHR barriers now — after all draws and
            // after the render pass is closed.  See the comment in the ixx for why
            // these are deferred rather than emitted at the FrameGraph's normal
            // non_tracked_resources transition point.
            if (!deferred_present_barriers.empty())
            {
                VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
                dep.imageMemoryBarrierCount = static_cast<uint32_t>(deferred_present_barriers.size());
                dep.pImageMemoryBarriers    = deferred_present_barriers.data();
                vkCmdPipelineBarrier2(vk_cmd, &dep);
                deferred_present_barriers.clear();
            }

            vkEndCommandBuffer(vk_cmd);
        }
        // Release the pool mutex — recording is complete.
        if (_pool_mutex) { _pool_mutex->unlock(); _pool_mutex = nullptr; }
    }

    // ---- Dynamic rendering helpers ------------------------------------------

    void CommandList::end_rendering_if_active()
    {
        if (in_render_pass)
        {
            vkCmdEndRendering(vk_cmd);
            in_render_pass = false;
        }
    }

    void CommandList::begin_rendering(VkAttachmentLoadOp  color_load,
                                       VkClearValue        color_clear,
                                       VkAttachmentLoadOp  depth_load,
                                       VkClearValue        depth_clear)
    {
        end_rendering_if_active();
        if (current_color_count == 0 && current_depth_view == VK_NULL_HANDLE)
            return;

        std::array<VkRenderingAttachmentInfo, MAX_COLOR_ATTACHMENTS> color_atts{};
        for (uint32_t i = 0; i < current_color_count; i++)
        {
            auto& a = color_atts[i];
            a.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            a.imageView   = current_color_views[i];
            a.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            a.loadOp      = color_load;
            a.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
            a.clearValue  = color_clear;
        }

        VkRenderingAttachmentInfo depth_att{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        depth_att.imageView   = current_depth_view;
        depth_att.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_att.loadOp      = depth_load;
        depth_att.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
        depth_att.clearValue  = depth_clear;

        VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        ri.renderArea             = { {0, 0}, current_extent };
        ri.layerCount             = current_layers;
        ri.colorAttachmentCount   = current_color_count;
        ri.pColorAttachments      = current_color_count ? color_atts.data() : nullptr;
        ri.pDepthAttachment       = current_depth_view != VK_NULL_HANDLE ? &depth_att : nullptr;

        vkCmdBeginRendering(vk_cmd, &ri);
        in_render_pass = true;
    }

    // ---- Barriers (synchronization2) ----------------------------------------

    // D3D12 describes a UAV clear with UNORDERED_ACCESS access; Vulkan clears
    // are transfer writes, and shader access bits are invalid on a clear-only stage.
    static void fix_clear_access(VkPipelineStageFlags2 stage, VkAccessFlags2& access, BarrierSync sync)
    {
        if (!check(sync & BarrierSync::CLEAR_UNORDERED_ACCESS_VIEW)) return;
        constexpr VkPipelineStageFlags2 clear_stages = VK_PIPELINE_STAGE_2_CLEAR_BIT | VK_PIPELINE_STAGE_2_COPY_BIT;
        if ((stage & ~clear_stages) == 0)
            access &= VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
        access |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
    }

    void CommandList::transitions(const HAL::Barriers& barriers)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        // Image/buffer barriers are illegal inside a dynamic rendering instance.
        // End the current render pass so the barrier lands outside it.
        end_rendering_if_active();

        std::vector<VkImageMemoryBarrier2> image_barriers;
        std::vector<VkBufferMemoryBarrier2> buffer_barriers;

        for (auto& b : barriers.get_barriers())
        {
            const auto* res = b.resource;
            if (!res) continue;

            if (res->get_type() == ResourceType::Texture)
            {
                auto& api_res = static_cast<const API::Resource&>(*res);
                if (api_res.get_vk_image() == VK_NULL_HANDLE) continue;

                VkImageMemoryBarrier2 ib{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
                ib.srcStageMask        = to_native_stage(b.before.operation);
                ib.srcAccessMask       = to_native_access(b.before.access);
                ib.dstStageMask        = to_native_stage(b.after.operation);
                ib.dstAccessMask       = to_native_access(b.after.access);
                fix_clear_access(ib.srcStageMask, ib.srcAccessMask, b.before.operation);
                fix_clear_access(ib.dstStageMask, ib.dstAccessMask, b.after.operation);
                // A resource entered from UNDEFINED may alias heap memory another
                // resource was still using. D3D12 orders that through the heap;
                // Vulkan needs the dependency spelled out, or the new contents race
                // the old resource's in-flight writes.
                // The DISCARD barrier of a freshly aliased resource often names a
                // before-state inherited from last frame (a compute read), so its
                // stages don't cover the previous occupant's draws. Wait on
                // everything and drop the contents, as D3D12's aliasing does.
                const bool discard = check(b.flags & BarrierFlags::DISCARD);
                if (discard || (to_native(b.before.layout) == VK_IMAGE_LAYOUT_UNDEFINED && ib.srcStageMask == VK_PIPELINE_STAGE_2_NONE))
                {
                    ib.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    ib.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
                }
                ib.oldLayout           = discard ? VK_IMAGE_LAYOUT_UNDEFINED : to_native(b.before.layout);
                ib.newLayout           = to_native(b.after.layout);
                if (DebugSync::widen_barriers.load(std::memory_order_relaxed))
                {
                    ib.srcStageMask  = ib.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    ib.srcAccessMask = ib.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
                }
                // D3D12 allows transitioning TO "undefined" (discard).
                // Vulkan forbids VK_IMAGE_LAYOUT_UNDEFINED as newLayout — skip it.
                if (ib.newLayout == VK_IMAGE_LAYOUT_UNDEFINED) continue;
                ib.image               = api_res.get_vk_image();
                bool is_depth = check(res->get_desc().Flags & ResFlags::DepthStencil);
                ib.subresourceRange = {
                    static_cast<VkImageAspectFlags>(
                        is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
                    0, VK_REMAINING_MIP_LEVELS,
                    0, VK_REMAINING_ARRAY_LAYERS
                };
                // Per-subresource barriers must stay per-subresource: a whole-image
                // UNDEFINED -> X for one mip's first use discards the other mips.
                if (!b.range.is_all())
                {
                    ib.subresourceRange.baseMipLevel   = b.range.first_mip;
                    ib.subresourceRange.levelCount     = b.range.num_mips;
                    ib.subresourceRange.baseArrayLayer = b.range.first_slice;
                    ib.subresourceRange.layerCount     = b.range.num_slices;
                    if (res->get_desc().as_texture().is3D())
                    {
                        ib.subresourceRange.baseArrayLayer = 0;
                        ib.subresourceRange.layerCount     = 1;
                    }
                }

                // Defer PRESENT_SRC_KHR transitions to end() so they always fire
                // after ALL draw calls.  The FrameGraph non_tracked_resources loop
                // places this barrier right after set_rtv() (before draws), which
                // would leave the swapchain in PRESENT_SRC_KHR when the draws run.
                // D3D12 is immune (PRESENT==COMMON; implicit promotion back to RT).
                // Vulkan is not → black output without this deferral.
                if (ib.newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                    deferred_present_barriers.push_back(ib);
                else
                    image_barriers.push_back(ib);
            }
            else
            {
                auto& api_res = static_cast<const API::Resource&>(*res);
                if (api_res.get_vk_buffer() == VK_NULL_HANDLE) continue;

                VkBufferMemoryBarrier2 bb{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
                bb.srcStageMask  = to_native_stage(b.before.operation);
                bb.srcAccessMask = to_native_access(b.before.access);
                bb.dstStageMask  = to_native_stage(b.after.operation);
                bb.dstAccessMask = to_native_access(b.after.access);
                fix_clear_access(bb.srcStageMask, bb.srcAccessMask, b.before.operation);
                fix_clear_access(bb.dstStageMask, bb.dstAccessMask, b.after.operation);
                if (DebugSync::widen_barriers.load(std::memory_order_relaxed))
                {
                    bb.srcStageMask  = bb.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    bb.srcAccessMask = bb.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
                }
                bb.buffer        = api_res.get_vk_buffer();
                bb.offset        = 0;
                bb.size          = VK_WHOLE_SIZE;
                buffer_barriers.push_back(bb);
            }
        }

        if (image_barriers.empty() && buffer_barriers.empty()) return;

        // A DISCARD starts a resource's life in heap memory another resource
        // (aliased) was just using. An image barrier's memory dependency covers
        // only its own image, so the previous occupant's in-flight writes to the
        // same memory are not flushed by it and can land on top of the new
        // contents later. A global memory barrier covers them.
        bool any_discard = false;
        for (auto& b : barriers.get_barriers())
            any_discard |= check(b.flags & BarrierFlags::DISCARD);
        VkMemoryBarrier2 alias_flush{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
        alias_flush.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        alias_flush.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        alias_flush.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        alias_flush.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;

        VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        if (any_discard)
        {
            dep.memoryBarrierCount = 1;
            dep.pMemoryBarriers    = &alias_flush;
        }
        dep.imageMemoryBarrierCount  = static_cast<uint32_t>(image_barriers.size());
        dep.pImageMemoryBarriers     = image_barriers.data();
        dep.bufferMemoryBarrierCount = static_cast<uint32_t>(buffer_barriers.size());
        dep.pBufferMemoryBarriers    = buffer_barriers.data();
        vkCmdPipelineBarrier2(vk_cmd, &dep);
    }

    // ---- Render target management -------------------------------------------

    void CommandList::set_rtv(int count, Handles::RTV rt, Handles::DSV dsv)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        end_rendering_if_active();

        const VkDevice vk_dev = static_cast<API::Device&>(*m_device).get_native_device();

        current_color_count = 0;
        current_depth_view  = VK_NULL_HANDLE;
        current_extent      = {};
        current_layers      = 1;

        // Every attachment must be bound: the pipeline's colorAttachmentCount has
        // to match the render pass's, where D3D12 would tolerate a short list.
        ASSERT(count <= int(MAX_COLOR_ATTACHMENTS));
        for (int i = 0; rt.is_valid() && i < count; i++)
        {
            Attachment a = resolve_attachment(vk_dev, rt[i]);
            ASSERT(a.view != VK_NULL_HANDLE && "render target has no image view");
            current_color_views[current_color_count++] = a.view;
            if (i == 0)
            {
                current_extent = a.extent;
                current_layers = a.layers;
            }
        }

        if (dsv.is_valid())
        {
            Attachment d = resolve_attachment(vk_dev, dsv);
            current_depth_view = d.view;
            if (!current_color_count)
            {
                current_extent = d.extent;
                current_layers = d.layers;
            }
        }
    }

    // ---- Clear operations ---------------------------------------------------

    void CommandList::clear_rtv(const Handles::RTV& h, vec4 color)
    {
        if (vk_cmd == VK_NULL_HANDLE || !h.is_valid()) return;

        // No layout barrier here: the caller's state tracking has already moved
        // this subresource to RENDER_TARGET, as D3D12 requires for a clear.
        const Attachment a = resolve_attachment(static_cast<API::Device&>(*m_device).get_native_device(), h);
        if (a.view == VK_NULL_HANDLE) return;

        end_rendering_if_active();

        VkClearValue cv{};
        cv.color.float32[0] = color.x;
        cv.color.float32[1] = color.y;
        cv.color.float32[2] = color.z;
        cv.color.float32[3] = color.w;

        VkRenderingAttachmentInfo att{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        att.imageView   = a.view;
        att.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        att.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
        att.clearValue  = cv;

        VkRenderingInfo rinfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        rinfo.renderArea           = { {0, 0}, a.extent };
        rinfo.layerCount           = a.layers;
        rinfo.colorAttachmentCount = 1;
        rinfo.pColorAttachments    = &att;

        vkCmdBeginRendering(vk_cmd, &rinfo);
        vkCmdEndRendering(vk_cmd);
    }

    void CommandList::clear_depth(const Handles::DSV& dsv, float depth)
    {
        if (vk_cmd == VK_NULL_HANDLE || !dsv.is_valid()) return;

        // No layout barrier: see clear_rtv. clear_dsv clears one VSM page slice
        // at a time, so touching the whole image would wipe every other page.
        const Attachment a = resolve_attachment(static_cast<API::Device&>(*m_device).get_native_device(), dsv);
        if (a.view == VK_NULL_HANDLE) return;

        end_rendering_if_active();

        VkClearValue cv;
        cv.depthStencil = { depth, 0 };

        VkRenderingAttachmentInfo att{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        att.imageView   = a.view;
        att.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        att.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
        att.clearValue  = cv;

        VkRenderingInfo rinfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        rinfo.renderArea      = { {0, 0}, a.extent };
        rinfo.layerCount      = a.layers;
        rinfo.pDepthAttachment = &att;

        vkCmdBeginRendering(vk_cmd, &rinfo);
        vkCmdEndRendering(vk_cmd);
    }

    void CommandList::clear_depth_stencil(const Handles::DSV& dsv, bool d, bool s, float fd, UINT8 fs)
    {
        if (d) clear_depth(dsv, fd);
    }

    void CommandList::clear_depth_rects(const Handles::DSV& dsv, float depth, std::vector<sizer_long> rects)
    {
        if (vk_cmd == VK_NULL_HANDLE || !dsv.is_valid()) return;

        // No layout barrier: see clear_rtv. An UNDEFINED transition here would
        // also discard everything outside the rects.
        const Attachment a = resolve_attachment(static_cast<API::Device&>(*m_device).get_native_device(), dsv);
        if (a.view == VK_NULL_HANDLE) return;

        end_rendering_if_active();

        VkClearValue cv;
        cv.depthStencil = { depth, 0 };

        VkRenderingAttachmentInfo att{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        att.imageView   = a.view;
        att.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        att.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
        att.clearValue  = cv;

        // One begin/end per rect -- dynamic rendering's renderArea IS the
        // clear scope for LOAD_OP_CLEAR, so restricting it per-rect gives a
        // scoped clear without needing a separate vkCmdClearAttachments path.
        for (auto& rect : rects)
        {
            VkRenderingInfo rinfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
            rinfo.renderArea = {
                { (int32_t)rect.left, (int32_t)rect.top },
                { (uint32_t)(rect.right - rect.left), (uint32_t)(rect.bottom - rect.top) } };
            rinfo.layerCount       = a.layers;
            rinfo.pDepthAttachment = &att;

            vkCmdBeginRendering(vk_cmd, &rinfo);
            vkCmdEndRendering(vk_cmd);
        }
    }
    void CommandList::clear_stencil(const Handles::DSV&, UINT8) { ASSERT(0); }

    void CommandList::global_barrier()
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        // A barrier inside dynamic rendering is only legal as a by-region
        // self-dependency; close the pass (the next draw reopens it with LOAD).
        end_rendering_if_active();

        // Vulkan equivalent of a D3D12 global barrier: one memory barrier over
        // ALL_COMMANDS with every access bit, no image layout involved.
        VkMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
        barrier.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;

        VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dep.memoryBarrierCount = 1;
        dep.pMemoryBarriers    = &barrier;

        vkCmdPipelineBarrier2(vk_cmd, &dep);
    }
    void CommandList::clear_uav(const Handles::UAV& h, vec4 color)
    {
        if (vk_cmd == VK_NULL_HANDLE || !h.is_valid()) return;

        auto& ri  = h.get_resource_info();
        auto* uav = std::get_if<Views::UnorderedAccess>(&ri.view);
        if (!uav || !uav->Resource) return;

        auto& api = static_cast<API::Resource&>(*uav->Resource);
        VkImage img = api.get_vk_image();
        end_rendering_if_active();
        if (img == VK_NULL_HANDLE)
        {
            clear_storage_buffer(api, *uav, { std::bit_cast<uint32_t>(color.x), std::bit_cast<uint32_t>(color.y),
                                              std::bit_cast<uint32_t>(color.z), std::bit_cast<uint32_t>(color.w) });
            return;
        }

        VkClearColorValue cv{};
        cv.float32[0] = color.x;
        cv.float32[1] = color.y;
        cv.float32[2] = color.z;
        cv.float32[3] = color.w;
        clear_storage_image(img, *uav, cv);
    }

    // ClearUnorderedAccessView on a typed/raw buffer view. vkCmdFillBuffer
    // repeats one 32-bit word, which is exact when every channel holds the same
    // value (zero included) -- what buffer clears use in practice.
    void CommandList::clear_storage_buffer(const API::Resource& res, const Views::UnorderedAccess& uav, uint4 bits)
    {
        if (res.get_vk_buffer() == VK_NULL_HANDLE) return;

        VkDeviceSize offset = res.get_vk_buffer_offset();
        VkDeviceSize size   = VK_WHOLE_SIZE;
        VkDeviceSize elem   = 4;
        if (auto* b = std::get_if<Views::UnorderedAccess::Buffer>(&uav.View))
        {
            elem = b->StructureByteStride ? b->StructureByteStride
                 : (uav.Format != Format::UNKNOWN ? uav.Format.size() / 8 : 4u);   // size() is in bits
            offset += VkDeviceSize(b->FirstElement) * elem;
            if (b->NumElements)
                size = VkDeviceSize(b->NumElements) * elem;
        }

        const uint channels = uint(std::max<VkDeviceSize>(1, elem / 4));
        uint32_t value = bits.x;
        bool exact = elem % 4 == 0;
        if (elem == 2) { value = (bits.x & 0xFFFFu) * 0x00010001u; exact = true; }
        if (elem == 1) { value = (bits.x & 0xFFu) * 0x01010101u;   exact = true; }
        if (size != VK_WHOLE_SIZE) size &= ~VkDeviceSize(3);   // vkCmdFillBuffer: multiple of 4
        for (uint c = 1; c < channels && c < 4; c++)
            exact &= bits[c] == bits.x;
        if (!exact)
        {
            static bool warned = false;
            if (!warned) { warned = true; Log::get() << Log::LEVEL_WARNING << "[Vulkan] buffer UAV clear with mixed channels or sub-32-bit elements: filling with channel 0" << Log::endl; }
        }
        if (size == VK_WHOLE_SIZE && res.get_vk_buffer_offset() != 0)
            size = static_cast<const HAL::Resource&>(res).get_desc().as_buffer().SizeInBytes;

        VkBufferMemoryBarrier2 b{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
        b.buffer = res.get_vk_buffer();
        b.offset = offset;
        b.size   = size;
        VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dep.bufferMemoryBarrierCount = 1;
        dep.pBufferMemoryBarriers    = &b;

        b.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        b.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        b.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
        b.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier2(vk_cmd, &dep);

        vkCmdFillBuffer(vk_cmd, res.get_vk_buffer(), offset, size, value);

        b.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
        b.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        b.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        b.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        vkCmdPipelineBarrier2(vk_cmd, &dep);
    }

    // Like ClearUnorderedAccessView: only the view's subresource. The HAL
    // transitions the image to UNORDERED_ACCESS (GENERAL) for a compute-stage
    // access, but vkCmdClearColorImage is a transfer-stage write, so it needs
    // its own ordering against the shader work on either side.
    void CommandList::clear_storage_image(VkImage img, const Views::UnorderedAccess& uav, const VkClearColorValue& cv)
    {
        VkImageSubresourceRange range{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        std::visit(overloaded{
            [&](const Views::UnorderedAccess::Texture2D& t)      { range.baseMipLevel = t.MipSlice; },
            [&](const Views::UnorderedAccess::Texture2DArray& t) { range.baseMipLevel = t.MipSlice; range.baseArrayLayer = t.FirstArraySlice; range.layerCount = t.ArraySize; },
            [&](const Views::UnorderedAccess::Texture3D& t)      { range.baseMipLevel = t.MipSlice; },
            [&](const Views::UnorderedAccess::Texture1DArray& t) { range.baseMipLevel = t.MipSlice; range.baseArrayLayer = t.FirstArraySlice; range.layerCount = t.ArraySize; },
            [&](auto&&) {}
        }, uav.View);
        if (range.layerCount == 0 || range.layerCount == uint32_t(-1))
            range.layerCount = VK_REMAINING_ARRAY_LAYERS;

        VkImageMemoryBarrier2 b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        b.oldLayout        = VK_IMAGE_LAYOUT_GENERAL;
        b.newLayout        = VK_IMAGE_LAYOUT_GENERAL;
        b.image            = img;
        b.subresourceRange = range;
        VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dep.imageMemoryBarrierCount = 1;
        dep.pImageMemoryBarriers    = &b;

        b.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        b.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        b.dstStageMask  = VK_PIPELINE_STAGE_2_CLEAR_BIT;
        b.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier2(vk_cmd, &dep);

        vkCmdClearColorImage(vk_cmd, img, VK_IMAGE_LAYOUT_GENERAL, &cv, 1, &range);

        b.srcStageMask  = VK_PIPELINE_STAGE_2_CLEAR_BIT;
        b.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        b.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        b.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        vkCmdPipelineBarrier2(vk_cmd, &dep);
    }

    void CommandList::clear_uav_uint(const Handles::UAV& h, uint4 color)
    {
        if (vk_cmd == VK_NULL_HANDLE || !h.is_valid()) return;

        auto& ri  = h.get_resource_info();
        auto* uav = std::get_if<Views::UnorderedAccess>(&ri.view);
        if (!uav || !uav->Resource) return;

        auto& api = static_cast<API::Resource&>(*uav->Resource);
        VkImage img = api.get_vk_image();
        end_rendering_if_active();
        if (img == VK_NULL_HANDLE)
        {
            clear_storage_buffer(api, *uav, color);
            return;
        }

        VkClearColorValue cv{};
        cv.uint32[0] = color.x;
        cv.uint32[1] = color.y;
        cv.uint32[2] = color.z;
        cv.uint32[3] = color.w;
        clear_storage_image(img, *uav, cv);
    }

    // ---- Lazy render-pass start (for draw calls) ----------------------------

    void CommandList::ensure_rendering_active()
    {
        if (in_render_pass) return;
        if (current_color_count == 0 && current_depth_view == VK_NULL_HANDLE)
            return;
        VkClearValue noop{};
        begin_rendering(VK_ATTACHMENT_LOAD_OP_LOAD, noop, VK_ATTACHMENT_LOAD_OP_LOAD, noop);
    }

    // Re-bind pipeline + viewport/scissor immediately before a draw.  The task-based
    // recorder can place set_pipeline / set_viewports / set_scissors in a different
    // command buffer than the draw, and Vulkan dynamic state + pipeline bindings do not
    // carry across command buffers.  Re-applying here guarantees the draw has them.
    void CommandList::reapply_draw_state()
    {
        if (current_graphics_pipeline != VK_NULL_HANDLE)
            vkCmdBindPipeline(vk_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, current_graphics_pipeline);

        if (!current_viewports.empty())
            vkCmdSetViewport(vk_cmd, 0, static_cast<uint32_t>(current_viewports.size()),
                             current_viewports.data());
        else if (current_extent.width && current_extent.height)
        {
            // Fallback: full-target viewport (negative height = D3D12 Y orientation).
            VkViewport vp{ 0.0f, (float)current_extent.height,
                           (float)current_extent.width, -(float)current_extent.height, 0.0f, 1.0f };
            vkCmdSetViewport(vk_cmd, 0, 1, &vp);
        }

        if (!current_scissors.empty())
            vkCmdSetScissor(vk_cmd, 0, static_cast<uint32_t>(current_scissors.size()), current_scissors.data());
        else if (current_extent.width && current_extent.height)
        {
            VkRect2D sc{ {0,0}, current_extent };
            vkCmdSetScissor(vk_cmd, 0, 1, &sc);
        }

        // Re-set topology — dynamic topology state doesn't survive a CB split.
        // Mesh pipelines have no topology dynamic state, so skip it for them.
        if (!current_is_mesh)
            vkCmdSetPrimitiveTopology(vk_cmd, current_topology);

        // Re-push the staged root-constant block (bindless descriptor indices).
        push_full_constants();
    }

    // Push the entire staged root-constant block.  A draw after a command-buffer
    // split would otherwise read undefined values for every slot not re-set since.
    void CommandList::push_full_constants()
    {
        static_assert(sizeof(push_constants) == API::Device::get_push_constant_size(),
                      "push-constant staging must match the pipeline layout's range");
        if (vk_cmd == VK_NULL_HANDLE) return;
        auto& dev = static_cast<API::Device&>(*m_device);
        vkCmdPushConstants(vk_cmd, dev.get_pipeline_layout(), VK_SHADER_STAGE_ALL, 0,
                           static_cast<uint32_t>(sizeof(push_constants)), push_constants.data());
    }

    void CommandList::flush_heaps()
    {
        if (!heaps_dirty || vk_cmd == VK_NULL_HANDLE || resource_set == VK_NULL_HANDLE) return;
        heaps_dirty = false;

        auto& dev = static_cast<API::Device&>(*m_device);
        const VkDescriptorSet sets[] = { resource_set, sampler_set };
        const uint32_t set_count = sampler_set != VK_NULL_HANDLE ? 2u : 1u;

        // A bind point the queue family doesn't support is invalid to bind to.
        auto bind = [&](VkPipelineBindPoint point) {
            vkCmdBindDescriptorSets(vk_cmd, point, dev.get_pipeline_layout(), 0, set_count, sets, 0, nullptr);
        };
        switch (type)
        {
        case CommandListType::DIRECT:
            bind(VK_PIPELINE_BIND_POINT_GRAPHICS);
            bind(VK_PIPELINE_BIND_POINT_COMPUTE);
            break;
        case CommandListType::COMPUTE:
        case CommandListType::COMPUTE2:
        case CommandListType::COMPUTE3:
            bind(VK_PIPELINE_BIND_POINT_COMPUTE);
            break;
        default:
            break;
        }
    }

    // ---- Pipeline binding ---------------------------------------------------

    void CommandList::set_pipeline(std::shared_ptr<TrackedPipeline> pipeline)
    {
        if (!pipeline || vk_cmd == VK_NULL_HANDLE) return;
        // Not built yet (a shader still compiling): forget the previous pipeline so
        // the draws/dispatches meant for this one are skipped, not run with it.
        if (pipeline->vk_pipeline == VK_NULL_HANDLE)
        {
            (pipeline->is_compute ? current_compute_pipeline : current_graphics_pipeline) = VK_NULL_HANDLE;
            return;
        }

        VkPipelineBindPoint bind_point = pipeline->is_compute
            ? VK_PIPELINE_BIND_POINT_COMPUTE
            : VK_PIPELINE_BIND_POINT_GRAPHICS;

        if (!pipeline->is_compute)
        {
            current_graphics_pipeline = pipeline->vk_pipeline;
            current_is_mesh           = pipeline->is_mesh;
            // Keep dynamic topology in sync with the PSO's declared topology so
            // the spec requirement (same class) is always satisfied without
            // requiring explicit set_topology calls at every draw site.
      //      current_topology = pipeline->vk_topology;
        }

        else
            current_compute_pipeline = pipeline->vk_pipeline;

        vkCmdBindPipeline(vk_cmd, bind_point, pipeline->vk_pipeline);
        flush_heaps();
    }

    // Every root signature maps onto the one device-global pipeline layout (the
    // SIG system uses DefaultLayout everywhere), so there is nothing to bind here.
    void CommandList::set_graphics_signature(const HAL::RootSignature::ptr&) {}
    void CommandList::set_compute_signature(const HAL::RootSignature::ptr&)  {}

    // ---- Descriptor heap binding -------------------------------------------

    void CommandList::set_descriptor_heaps(DescriptorHeap* cbv, DescriptorHeap* sampler)
    {
        auto* api_cbv     = static_cast<API::DescriptorHeap*>(cbv);
        auto* api_sampler = static_cast<API::DescriptorHeap*>(sampler);

        resource_set = api_cbv     ? api_cbv->get_vk_set()     : VK_NULL_HANDLE;
        sampler_set  = api_sampler ? api_sampler->get_vk_set() : VK_NULL_HANDLE;
        heaps_dirty  = true;
    }

    // ---- Draw / dispatch ---------------------------------------------------

    void CommandList::draw(UINT vertex_count, UINT vertex_offset,
                            UINT instance_count, UINT instance_offset)
    {
        if (vk_cmd == VK_NULL_HANDLE || current_graphics_pipeline == VK_NULL_HANDLE) return;
        ensure_rendering_active();
        if (!in_render_pass) return; // no RTV set — skip rather than crash validation
        reapply_draw_state();
        flush_heaps();
        vkCmdDraw(vk_cmd, vertex_count, instance_count, vertex_offset, instance_offset);
    }

    void CommandList::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset,
                                    UINT instance_count, UINT instance_offset)
    {
        if (vk_cmd == VK_NULL_HANDLE || current_graphics_pipeline == VK_NULL_HANDLE) return;
        ensure_rendering_active();
        if (!in_render_pass) return;
        reapply_draw_state();
        if (current_index_buffer != VK_NULL_HANDLE)
            vkCmdBindIndexBuffer(vk_cmd, current_index_buffer, current_index_offset, current_index_type);
        flush_heaps();
        vkCmdDrawIndexed(vk_cmd, index_count, instance_count,
                         index_offset, static_cast<int32_t>(vertex_offset), instance_offset);
    }

    void CommandList::dispatch(ivec3 v)
    {
        if (vk_cmd == VK_NULL_HANDLE || current_compute_pipeline == VK_NULL_HANDLE) return;
        // Compute doesn't use a render pass — end any active one first.
        end_rendering_if_active();
        // Same reason as reapply_draw_state: the pipeline may have been bound in
        // an earlier command buffer of a split recording.
        vkCmdBindPipeline(vk_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, current_compute_pipeline);
        flush_heaps();
        push_full_constants();  // compute shaders read bindless indices from push data
        vkCmdDispatch(vk_cmd, static_cast<uint32_t>(v.x),
                               static_cast<uint32_t>(v.y),
                               static_cast<uint32_t>(v.z));
    }

    void CommandList::dispatch_mesh(ivec3 v)
    {
        if (vk_cmd == VK_NULL_HANDLE || current_graphics_pipeline == VK_NULL_HANDLE) return;
        // Mesh dispatch is a rasterization command — needs an active render pass and
        // the same pipeline/viewport/scissor/heaps as a draw (but no topology).
        ensure_rendering_active();
        if (!in_render_pass) return;
        reapply_draw_state();
        flush_heaps();
        // vkCmdDrawMeshTasksEXT is an extension entry point — not exported by
        // vulkan-1.dll — so resolve it once via vkGetDeviceProcAddr.
        auto* dev = static_cast<API::Device*>(m_device);
        VkDevice vk_dev = dev ? dev->get_native_device() : VK_NULL_HANDLE;
        static PFN_vkCmdDrawMeshTasksEXT p_mesh = nullptr;
        if (!p_mesh && vk_dev)
            p_mesh = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(
                vkGetDeviceProcAddr(vk_dev, "vkCmdDrawMeshTasksEXT"));
        if (p_mesh)
            p_mesh(vk_cmd, static_cast<uint32_t>(v.x),
                           static_cast<uint32_t>(v.y),
                           static_cast<uint32_t>(v.z));
    }

    // ---- Index buffer ------------------------------------------------------

    void CommandList::set_index_buffer(HAL::Views::IndexBuffer index)
    {
        if (vk_cmd == VK_NULL_HANDLE || !index.Resource) return;
        auto& api_res = static_cast<API::Resource&>(*index.Resource);
        if (api_res.get_vk_buffer() == VK_NULL_HANDLE) return;

        VkIndexType idx_type = (index.Format == HAL::Format::R32_UINT)
            ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;

        current_index_buffer = api_res.get_vk_buffer();
        current_index_offset = api_res.get_vk_buffer_offset() + index.OffsetInBytes;
        current_index_type   = idx_type;
        vkCmdBindIndexBuffer(vk_cmd, current_index_buffer, current_index_offset, idx_type);
    }

    // ---- Viewport / scissor ------------------------------------------------

    void CommandList::set_viewports(std::vector<Viewport> viewports)
    {
        if (vk_cmd == VK_NULL_HANDLE || viewports.empty()) return;

        std::vector<VkViewport> vk_vps;
        vk_vps.reserve(viewports.size());
        for (auto& vp : viewports)
        {
            // Vulkan Y-flip: use negative height trick to match D3D12 NDC
            VkViewport v{};
            v.x        = vp.pos.x;
            v.y        = vp.pos.y + vp.size.y;  // start at bottom
            v.width    = vp.size.x;
            v.height   = -vp.size.y;             // negative flips Y
            v.minDepth = vp.depths.x;
            v.maxDepth = vp.depths.y;
            vk_vps.push_back(v);
        }
        current_viewports = vk_vps;   // remember for per-draw re-apply (CB-split safe)
        vkCmdSetViewport(vk_cmd, 0, static_cast<uint32_t>(vk_vps.size()), vk_vps.data());
    }

    void CommandList::set_scissors(sizer_long rect)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        VkRect2D scissor{};
        scissor.offset = { static_cast<int32_t>(rect.left),
                           static_cast<int32_t>(rect.top) };
        scissor.extent = { static_cast<uint32_t>(rect.right  - rect.left),
                           static_cast<uint32_t>(rect.bottom - rect.top) };
        current_scissors.assign(1, scissor);   // remember for per-draw re-apply (CB-split safe)
        vkCmdSetScissor(vk_cmd, 0, 1, &scissor);
    }

    void CommandList::set_scissors(std::vector<sizer_long> rects)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        std::vector<VkRect2D> vk_scissors;
        vk_scissors.reserve(rects.size());
        for (auto& rect : rects)
        {
            VkRect2D scissor{};
            scissor.offset = { static_cast<int32_t>(rect.left),
                               static_cast<int32_t>(rect.top) };
            scissor.extent = { static_cast<uint32_t>(rect.right  - rect.left),
                               static_cast<uint32_t>(rect.bottom - rect.top) };
            vk_scissors.push_back(scissor);
        }
        current_scissors = vk_scissors;   // remember for per-draw re-apply (CB-split safe)
        vkCmdSetScissor(vk_cmd, 0, static_cast<uint32_t>(vk_scissors.size()), vk_scissors.data());
    }

    void CommandList::set_stencil_ref(UINT ref)
    {
        if (vk_cmd != VK_NULL_HANDLE)
            vkCmdSetStencilReference(vk_cmd, VK_STENCIL_FACE_FRONT_AND_BACK, ref);
    }

    // ---- Push constants ---------------------------------------------------

    void CommandList::graphics_set_constant(UINT slot, UINT offset, UINT value)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        const uint32_t idx = slot + offset;
        // Stage so reapply_draw_state() can re-push the whole block before each draw —
        // push data does not survive a command-buffer split; without re-pushing, the
        // shader reads 0 for every bindless descriptor index → all bindless reads fail.
        ASSERT(idx < push_constants.size());
        if (idx >= push_constants.size()) return;
        push_constants[idx] = value;

        auto& dev = static_cast<API::Device&>(*m_device);
        vkCmdPushConstants(vk_cmd, dev.get_pipeline_layout(), VK_SHADER_STAGE_ALL,
                           idx * sizeof(uint32_t), sizeof(uint32_t), &value);
    }

    // Push constants belong to the pipeline layout, not a bind point, so the
    // graphics and compute variants share one staging block.
    void CommandList::compute_set_constant(UINT slot, UINT offset, UINT value)
    {
        graphics_set_constant(slot, offset, value);
    }

    // Phase 5: push descriptors for inline CBV binding
    void CommandList::graphics_set_const_buffer(UINT, const ResourceAddress&) { ASSERT(0); }
    void CommandList::compute_set_const_buffer(UINT, const ResourceAddress&)  { ASSERT(0); }

    // ---- Copy operations ---------------------------------------------------

    void CommandList::copy_resource(HAL::Resource* dest, HAL::Resource* source)
    {
        if (!dest || !source || vk_cmd == VK_NULL_HANDLE) return;
        auto& dst_api = static_cast<API::Resource&>(*dest);
        auto& src_api = static_cast<API::Resource&>(*source);

        if (dst_api.get_vk_buffer() != VK_NULL_HANDLE &&
            src_api.get_vk_buffer() != VK_NULL_HANDLE)
        {
            VkBufferCopy region{};
            region.srcOffset = src_api.get_vk_buffer_offset();
            region.dstOffset = dst_api.get_vk_buffer_offset();
            region.size      = dest->get_desc().as_buffer().SizeInBytes;
            vkCmdCopyBuffer(vk_cmd, src_api.get_vk_buffer(),
                            dst_api.get_vk_buffer(), 1, &region);
            return;
        }

        // CopyResource on textures: every mip of every layer.
        if (dst_api.get_vk_image() != VK_NULL_HANDLE && src_api.get_vk_image() != VK_NULL_HANDLE
            && dest->get_desc().is_texture())
        {
            const auto& td = dest->get_desc().as_texture();
            const uint mips   = std::max(1u, (uint)td.MipLevels);
            const uint layers = td.is3D() ? 1u : std::max(1u, (uint)td.ArraySize);
            std::vector<VkImageCopy> regions(mips);
            for (uint m = 0; m < mips; m++)
            {
                auto& r = regions[m];
                r.srcSubresource = { src_api.get_vk_aspect(), m, 0, layers };
                r.dstSubresource = { dst_api.get_vk_aspect(), m, 0, layers };
                r.extent = mip_extent(td, m);
            }
            vkCmdCopyImage(vk_cmd,
                src_api.get_vk_image(), VK_IMAGE_LAYOUT_GENERAL,
                dst_api.get_vk_image(), VK_IMAGE_LAYOUT_GENERAL,
                mips, regions.data());
        }
    }

    void CommandList::copy_buffer(HAL::Resource* dest, uint64 dest_offset,
                                   HAL::Resource* source, uint64 src_offset, uint64 size)
    {
        if (!dest || !source || vk_cmd == VK_NULL_HANDLE) return;
        auto& dst_api = static_cast<API::Resource&>(*dest);
        auto& src_api = static_cast<API::Resource&>(*source);
        if (dst_api.get_vk_buffer() == VK_NULL_HANDLE ||
            src_api.get_vk_buffer() == VK_NULL_HANDLE) return;

        VkBufferCopy region{};
        region.srcOffset = src_api.get_vk_buffer_offset() + src_offset;
        region.dstOffset = dst_api.get_vk_buffer_offset() + dest_offset;
        region.size      = size;
        vkCmdCopyBuffer(vk_cmd, src_api.get_vk_buffer(),
                        dst_api.get_vk_buffer(), 1, &region);
    }

    // Build a VkBufferImageCopy from the HAL texture_layout.  The staging buffer
    // rows are 256-byte aligned (see Device::get_texture_layout), so bufferRowLength
    // must be expressed in texels = row_stride / bytes-per-texel (NOT 0, which would
    // assume tight packing and shear the image for non-aligned widths).
    static VkBufferImageCopy make_buffer_image_copy(const HAL::Resource& resource,
                                                    ivec3 offset, ivec3 box, UINT sub_resource,
                                                    const ResourceAddress& address,
                                                    const texture_layout& layout)
    {
        if (box.y == 0) box.y = 1;
        if (box.z == 0) box.z = 1;

        auto  sinfo = layout.format.surface_info({ (uint)box.x, (uint)box.y });
        uint  bpt   = box.x ? (uint)(sinfo.rowBytes / box.x) : 4u; // bytes per texel
        if (bpt == 0) bpt = 4u;

        auto& tdesc = resource.get_desc().as_texture();
        uint  mips  = tdesc.MipLevels ? tdesc.MipLevels : 1u;

        VkImageAspectFlags aspect = static_cast<const API::Resource&>(resource).get_vk_aspect();

        VkBufferImageCopy region{};
        region.bufferOffset      = static_cast<const API::Resource&>(*address.resource).get_vk_buffer_offset() + address.resource_offset;
        region.bufferRowLength   = (layout.row_stride / bpt);
        region.bufferImageHeight = 0; // tight vertically: rows = imageExtent.height
        region.imageSubresource  = { aspect,
                                     sub_resource % mips, sub_resource / mips, 1 };
        region.imageOffset       = { offset.x, offset.y, offset.z };
        region.imageExtent       = { (uint32_t)box.x, (uint32_t)box.y, (uint32_t)box.z };
        return region;
    }

    void CommandList::update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box,
                                      UINT sub_resource, ResourceAddress address,
                                      texture_layout layout)
    {
        if (!resource || vk_cmd == VK_NULL_HANDLE || !address.resource) return;
        auto& dst = static_cast<API::Resource&>(*resource);
        auto& staging = static_cast<API::Resource&>(*address.resource);
        if (dst.get_vk_image() == VK_NULL_HANDLE || staging.get_vk_buffer() == VK_NULL_HANDLE)
            return;

        // The HAL transitions `resource` to COPY_DEST (→ TRANSFER_DST_OPTIMAL) before this call.
        VkBufferImageCopy region = make_buffer_image_copy(*resource, offset, box, sub_resource, address, layout);
        vkCmdCopyBufferToImage(vk_cmd, staging.get_vk_buffer(), dst.get_vk_image(),
            VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    }

    void CommandList::read_texture(const HAL::Resource* resource, ivec3 offset, ivec3 box,
                                    UINT sub_resource, ResourceAddress target,
                                    texture_layout layout)
    {
        if (!resource || vk_cmd == VK_NULL_HANDLE || !target.resource) return;
        auto& src = static_cast<const API::Resource&>(*resource);
        auto& staging = static_cast<API::Resource&>(*target.resource);
        if (src.get_vk_image() == VK_NULL_HANDLE || staging.get_vk_buffer() == VK_NULL_HANDLE)
            return;

        // The HAL transitions `resource` to COPY_SOURCE (→ TRANSFER_SRC_OPTIMAL) before this call.
        VkBufferImageCopy region = make_buffer_image_copy(*resource, offset, box, sub_resource, target, layout);
        vkCmdCopyImageToBuffer(vk_cmd, src.get_vk_image(),
            VK_IMAGE_LAYOUT_GENERAL, staging.get_vk_buffer(), 1, &region);
    }

    void CommandList::copy_texture(const Resource::ptr& dest, int dest_sub,
                                    const Resource::ptr& source, int src_sub)
    {
        if (!dest || !source || vk_cmd == VK_NULL_HANDLE) return;
        auto& dst = static_cast<API::Resource&>(*dest);
        auto& src = static_cast<API::Resource&>(*source);
        if (dst.get_vk_image() == VK_NULL_HANDLE || src.get_vk_image() == VK_NULL_HANDLE) return;

        // Subresource to subresource, sized by the source subresource like
        // CopyTextureRegion with no box. Flat index = mip + layer * mips.
        const auto& sd = static_cast<const HAL::Resource&>(*source).get_desc().as_texture();
        const auto& dd = static_cast<const HAL::Resource&>(*dest).get_desc().as_texture();
        const uint src_mips = std::max(1u, (uint)sd.MipLevels);
        const uint dst_mips = std::max(1u, (uint)dd.MipLevels);
        const uint src_mip = uint(src_sub) % src_mips, src_layer = uint(src_sub) / src_mips;
        const uint dst_mip = uint(dest_sub) % dst_mips, dst_layer = uint(dest_sub) / dst_mips;

        VkImageCopy region{};
        region.srcSubresource = { src.get_vk_aspect(), src_mip, sd.is3D() ? 0u : src_layer, 1 };
        region.dstSubresource = { dst.get_vk_aspect(), dst_mip, dd.is3D() ? 0u : dst_layer, 1 };
        region.extent         = mip_extent(sd, src_mip);
        vkCmdCopyImage(vk_cmd,
            src.get_vk_image(), VK_IMAGE_LAYOUT_GENERAL,
            dst.get_vk_image(), VK_IMAGE_LAYOUT_GENERAL,
            1, &region);
    }

    void CommandList::copy_texture(const Resource::ptr& dest, ivec3 dest_pos,
                                    const Resource::ptr& source, ivec3 src_pos, ivec3 size)
    {
        if (!dest || !source || vk_cmd == VK_NULL_HANDLE) return;
        auto& dst = static_cast<API::Resource&>(*dest);
        auto& src = static_cast<API::Resource&>(*source);
        if (dst.get_vk_image() == VK_NULL_HANDLE || src.get_vk_image() == VK_NULL_HANDLE) return;

        VkImageCopy region{};
        region.srcSubresource = { src.get_vk_aspect(), 0, 0, 1 };
        region.dstSubresource = { dst.get_vk_aspect(), 0, 0, 1 };
        region.srcOffset      = { src_pos.x, src_pos.y, src_pos.z };
        region.dstOffset      = { dest_pos.x, dest_pos.y, dest_pos.z };
        region.extent         = { static_cast<uint32_t>(size.x),
                                   static_cast<uint32_t>(size.y),
                                   static_cast<uint32_t>(size.z) };
        vkCmdCopyImage(vk_cmd,
            src.get_vk_image(), VK_IMAGE_LAYOUT_GENERAL,
            dst.get_vk_image(), VK_IMAGE_LAYOUT_GENERAL,
            1, &region);
    }

    // ---- Set topology -------------------------------------------------------
    // Vulkan 1.3 promotes VK_EXT_extended_dynamic_state to core, so
    // VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY is always available.  Mirrors D3D12's
    // IASetPrimitiveTopology: the PSO has a topology TYPE (TRIANGLE/LINE), while
    // this call sets the actual LIST vs STRIP mode used by the draw.
    void CommandList::set_topology(HAL::PrimitiveTopologyType t, HAL::PrimitiveTopologyFeed feed,
                                    bool, uint)
    {
        using T = HAL::PrimitiveTopologyType;
        using F = HAL::PrimitiveTopologyFeed;

        VkPrimitiveTopology vk_topo;
        if (t == T::POINT)
            vk_topo = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        else if (t == T::LINE)
            vk_topo = (feed == F::STRIP) ? VK_PRIMITIVE_TOPOLOGY_LINE_STRIP
                                         : VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        else if (t == T::PATCH)
            vk_topo = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        else
            vk_topo = (feed == F::STRIP) ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
                                         : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        current_topology = vk_topo;
        if (vk_cmd != VK_NULL_HANDLE)
            vkCmdSetPrimitiveTopology(vk_cmd, vk_topo);
    }

    // ---- Debug labels (VK_EXT_debug_utils) ----------------------------------

    void CommandList::start_event(std::wstring_view name)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        // Narrow wstring → UTF-8 for Vulkan
        std::string label(name.begin(), name.end());
        auto* dev = static_cast<API::Device*>(m_device);
        if (!dev) return;
        auto fn = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
            vkGetDeviceProcAddr(dev->get_native_device(), "vkCmdBeginDebugUtilsLabelEXT"));
        if (!fn) return;
        VkDebugUtilsLabelEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
        info.pLabelName = label.c_str();
        fn(vk_cmd, &info);
    }

    void CommandList::end_event()
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        auto* dev = static_cast<API::Device*>(m_device);
        if (!dev) return;
        auto fn = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
            vkGetDeviceProcAddr(dev->get_native_device(), "vkCmdEndDebugUtilsLabelEXT"));
        if (fn) fn(vk_cmd);
    }

    // ---- Indirect -----------------------------------------------------------
    // The engine's argument structs (DrawIndexedArguments / DispatchArguments /
    // DispatchMeshArguments) are laid out identically to Vulkan's indirect command
    // structs and carry no per-command root constants, so each maps straight to a
    // vkCmd*Indirect call.  A counter buffer (when present) selects the *Count
    // variant, matching D3D12 ExecuteIndirect's max(count, maxCommands) semantics.
    void CommandList::execute_indirect(const IndirectCommand& command, UINT max_commands,
                                        Resource* command_buffer, UINT64 command_offset,
                                        Resource* counter_buffer, UINT64 counter_offset)
    {
        if (vk_cmd == VK_NULL_HANDLE || !command_buffer) return;
        auto& args_res = static_cast<API::Resource&>(*command_buffer);
        VkBuffer args = args_res.get_vk_buffer();
        if (args == VK_NULL_HANDLE) return;

        const uint32_t stride = command.stride;
        command_offset += command.args_offset + args_res.get_vk_buffer_offset();
        VkBuffer count_buf = VK_NULL_HANDLE;
        if (counter_buffer)
        {
            auto& count_res = static_cast<API::Resource&>(*counter_buffer);
            count_buf = count_res.get_vk_buffer();
            counter_offset += count_res.get_vk_buffer_offset();
        }

        switch (command.kind)
        {
        case IndirectKind::DrawIndexed:
        {
            if (current_graphics_pipeline == VK_NULL_HANDLE) return;
            ensure_rendering_active();
            if (!in_render_pass) return;
            reapply_draw_state();
            if (current_index_buffer != VK_NULL_HANDLE)
                vkCmdBindIndexBuffer(vk_cmd, current_index_buffer, current_index_offset, current_index_type);
            flush_heaps();
            if (count_buf != VK_NULL_HANDLE)
                vkCmdDrawIndexedIndirectCount(vk_cmd, args, command_offset,
                                              count_buf, counter_offset, max_commands, stride);
            else
                vkCmdDrawIndexedIndirect(vk_cmd, args, command_offset, max_commands, stride);
            break;
        }
        case IndirectKind::DispatchMesh:
        {
            if (current_graphics_pipeline == VK_NULL_HANDLE) return;
            ensure_rendering_active();
            if (!in_render_pass) return;
            reapply_draw_state();
            flush_heaps();
            // Mesh-shader indirect commands are EXT — not exported by vulkan-1.dll,
            // so resolve them once via vkGetDeviceProcAddr.
            auto* dev = static_cast<API::Device*>(m_device);
            VkDevice vk_dev = dev ? dev->get_native_device() : VK_NULL_HANDLE;
            static PFN_vkCmdDrawMeshTasksIndirectEXT      p_mesh = nullptr;
            static PFN_vkCmdDrawMeshTasksIndirectCountEXT p_mesh_count = nullptr;
            if (!p_mesh && vk_dev)
                p_mesh = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(
                    vkGetDeviceProcAddr(vk_dev, "vkCmdDrawMeshTasksIndirectEXT"));
            if (!p_mesh_count && vk_dev)
                p_mesh_count = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(
                    vkGetDeviceProcAddr(vk_dev, "vkCmdDrawMeshTasksIndirectCountEXT"));
            if (count_buf != VK_NULL_HANDLE && p_mesh_count)
                p_mesh_count(vk_cmd, args, command_offset, count_buf, counter_offset, max_commands, stride);
            else if (p_mesh)
                p_mesh(vk_cmd, args, command_offset, max_commands, stride);
            break;
        }
        case IndirectKind::Dispatch:
        {
            if (current_compute_pipeline == VK_NULL_HANDLE) return;
            end_rendering_if_active();
            vkCmdBindPipeline(vk_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, current_compute_pipeline);
            flush_heaps();
            push_full_constants();  // compute shaders read bindless indices from push data
            // Vulkan has no indirect-count dispatch; issue up to max_commands
            // individual indirect dispatches from consecutive buffer entries.
            for (uint32_t i = 0; i < max_commands; ++i)
                vkCmdDispatchIndirect(vk_cmd, args, command_offset + static_cast<VkDeviceSize>(i) * stride);
            break;
        }
        default:
            break;
        }
    }

    // ---- Misc ---------------------------------------------------------------
    void CommandList::set_name(std::wstring_view name)
    {
        debug_name.assign(name.begin(), name.end());
        if (vk_cmd == VK_NULL_HANDLE) return;
        auto* dev = static_cast<API::Device*>(m_device);
        if (!dev) return;
        auto fn = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
            vkGetDeviceProcAddr(dev->get_native_device(), "vkSetDebugUtilsObjectNameEXT"));
        if (!fn) return;
        std::string label(name.begin(), name.end());
        VkDebugUtilsObjectNameInfoEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
        info.objectType   = VK_OBJECT_TYPE_COMMAND_BUFFER;
        info.objectHandle = reinterpret_cast<uint64_t>(vk_cmd);
        info.pObjectName  = label.c_str();
        fn(dev->get_native_device(), &info);
    }
    void CommandList::discard(const HAL::Resource*)
    {
        static bool warned = false;
        if (!warned) { warned = true; Log::get() << Log::LEVEL_WARNING << "[Vulkan] CommandList::discard not implemented" << Log::endl; }
    }
    void CommandList::insert_time(const QueryHandle& handle, uint index)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        auto* heap = handle.get_heap().get();
        if (!heap) return;
        auto& api_heap = static_cast<API::QueryHeap&>(*heap);
        if (api_heap.get_native() == VK_NULL_HANDLE) return;
        uint32_t slot = static_cast<uint32_t>(handle.get_offset() + index);
        // In Vulkan, queries must be reset between uses (unlike D3D12 where this
        // is implicit).  Emit a per-slot reset command immediately before the write
        // so the query is always in the "unavailable" state when written.
        vkCmdResetQueryPool(vk_cmd, api_heap.get_native(), slot, 1);
        vkCmdWriteTimestamp2(vk_cmd,
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            api_heap.get_native(), slot);
    }

    void CommandList::resolve_times(const QueryHeap* heap, uint32_t count,
                                     ResourceAddress dest)
    {
        if (!heap || vk_cmd == VK_NULL_HANDLE || !dest.resource) return;

        auto& api_heap = static_cast<const API::QueryHeap&>(*heap);
        if (api_heap.get_native() == VK_NULL_HANDLE) return;

        auto& dst_res = static_cast<API::Resource&>(*dest.resource);
        if (dst_res.get_vk_buffer() == VK_NULL_HANDLE) return;

        // No WAIT_BIT: a slot reset but not written this frame never becomes
        // available, and waiting on it stalls the queue forever. D3D12's
        // ResolveQueryData just returns stale data for such slots; so does this.
        vkCmdCopyQueryPoolResults(vk_cmd,
            api_heap.get_native(),
            0, count,
            dst_res.get_vk_buffer(),
            dst_res.get_vk_buffer_offset() + dest.resource_offset,
            sizeof(uint64_t),
            VK_QUERY_RESULT_64_BIT);
    }
}
