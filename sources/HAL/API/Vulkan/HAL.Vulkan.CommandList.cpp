module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
module HAL:API.CommandList;

import stl.core;
import Core;
import :CommandAllocator;   // full definition needed: allocator.vk_command_pool
import :RootSignature;      // API::RootSignature::get_vk_pipeline_layout()
import :API.Device;         // API::Device::get_native_device()
import :API.DescriptorHeap; // API::DescriptorHeap::get_vk_set()
import :API.QueryHeap;      // API::QueryHeap::get_native()

namespace HAL::API
{
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

        if (vk_cmd != VK_NULL_HANDLE && vk_cmd_pool == api_alloc.vk_command_pool)
        {
            // Same pool as before — we hold its mutex so reset is safe.
            vkResetCommandBuffer(vk_cmd, 0);
        }
        else
        {
            // First use, or the compile() rotation gave us a different allocator.
            // Allocate a fresh buffer from this pool.  The old vk_cmd (if any)
            // remains allocated from its original pool and will be freed when
            // CommandAllocator::reset() is called on that pool — no explicit free
            // needed here because VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
            // lets the pool reclaim its buffers on pool reset.
            VkCommandBufferAllocateInfo alloc{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            alloc.commandPool        = api_alloc.vk_command_pool;
            alloc.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc.commandBufferCount = 1;
            vkAllocateCommandBuffers(api_dev.get_native_device(), &alloc, &vk_cmd);
            vk_cmd_pool = api_alloc.vk_command_pool;
        }

        VkCommandBufferBeginInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(vk_cmd, &info);
    }

    void CommandList::end()
    {
        if (vk_cmd != VK_NULL_HANDLE)
        {
            end_rendering_if_active();
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
        if (current_color_view == VK_NULL_HANDLE && current_depth_view == VK_NULL_HANDLE)
            return;

        VkRenderingAttachmentInfo color_att{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color_att.imageView   = current_color_view;
        color_att.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_att.loadOp      = color_load;
        color_att.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
        color_att.clearValue  = color_clear;

        VkRenderingAttachmentInfo depth_att{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        depth_att.imageView   = current_depth_view;
        depth_att.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_att.loadOp      = depth_load;
        depth_att.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
        depth_att.clearValue  = depth_clear;

        VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        ri.renderArea             = { {0, 0}, current_extent };
        ri.layerCount             = 1;
        ri.colorAttachmentCount   = current_color_view != VK_NULL_HANDLE ? 1 : 0;
        ri.pColorAttachments      = current_color_view != VK_NULL_HANDLE ? &color_att : nullptr;
        ri.pDepthAttachment       = current_depth_view != VK_NULL_HANDLE ? &depth_att : nullptr;

        vkCmdBeginRendering(vk_cmd, &ri);
        in_render_pass = true;
    }

    // ---- Barriers (synchronization2) ----------------------------------------

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
                ib.oldLayout           = to_native(b.before.layout);
                ib.newLayout           = to_native(b.after.layout);
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
                bb.buffer        = api_res.get_vk_buffer();
                bb.offset        = 0;
                bb.size          = VK_WHOLE_SIZE;
                buffer_barriers.push_back(bb);
            }
        }

        if (image_barriers.empty() && buffer_barriers.empty()) return;

        VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dep.imageMemoryBarrierCount  = static_cast<uint32_t>(image_barriers.size());
        dep.pImageMemoryBarriers     = image_barriers.data();
        dep.bufferMemoryBarrierCount = static_cast<uint32_t>(buffer_barriers.size());
        dep.pBufferMemoryBarriers    = buffer_barriers.data();
        vkCmdPipelineBarrier2(vk_cmd, &dep);
    }

    // ---- Render target management -------------------------------------------

    void CommandList::set_rtv(int /*count*/, RTVHandle rt, DSVHandle dsv)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        end_rendering_if_active();

        current_color_view = VK_NULL_HANDLE;
        current_depth_view = VK_NULL_HANDLE;
        current_extent     = {};

        auto extract_view = [](const Handle& h, bool depth) -> std::pair<VkImageView, VkExtent2D>
        {
            if (!h.is_valid()) return { VK_NULL_HANDLE, {} };
            auto& ri = h.get_resource_info();

            std::shared_ptr<HAL::Resource> res;
            if (!depth)
            {
                auto* rtv = std::get_if<Views::RenderTarget>(&ri.view);
                if (rtv) res = rtv->Resource;
            }
            else
            {
                auto* dsv = std::get_if<Views::DepthStencil>(&ri.view);
                if (dsv) res = dsv->Resource;
            }

            if (!res) return { VK_NULL_HANDLE, {} };
            auto& api = static_cast<API::Resource&>(*res);

            // get_vk_image_view() returns the swapchain view if present,
            // otherwise the owned VkImageView created by Resource::init().
            return { api.get_vk_image_view(), api.get_imported_extent() };
        };

        auto [cv, ce] = extract_view(rt, false);
        auto [dv, de] = extract_view(dsv, true);

        current_color_view = cv;
        current_depth_view = dv;
        current_extent     = (cv != VK_NULL_HANDLE) ? ce : de;
    }

    // ---- Clear operations ---------------------------------------------------

    void CommandList::clear_rtv(const RTVHandle& h, vec4 color)
    {
        if (vk_cmd == VK_NULL_HANDLE || !h.is_valid()) return;

        auto& ri  = h.get_resource_info();
        auto* rtv = std::get_if<Views::RenderTarget>(&ri.view);
        if (!rtv || !rtv->Resource) return;

        auto& api = static_cast<API::Resource&>(*rtv->Resource);
        VkImageView view = api.get_import_handle().image_view;
        if (view == VK_NULL_HANDLE) return;

        end_rendering_if_active();

        VkClearValue cv{};
        cv.color.float32[0] = color.x;
        cv.color.float32[1] = color.y;
        cv.color.float32[2] = color.z;
        cv.color.float32[3] = color.w;

        VkRenderingAttachmentInfo att{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        att.imageView   = view;
        att.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        att.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
        att.clearValue  = cv;

        VkExtent2D ext = api.get_imported_extent();
        // Fallback: if no imported extent (regular texture), use desc dimensions
        if (ext.width == 0 && rtv->Resource->get_desc().is_texture())
        {
            auto& td = rtv->Resource->get_desc().as_texture();
            ext = { td.Dimensions.x, td.Dimensions.y };
        }

        VkRenderingInfo rinfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        rinfo.renderArea           = { {0, 0}, ext };
        rinfo.layerCount           = 1;
        rinfo.colorAttachmentCount = 1;
        rinfo.pColorAttachments    = &att;

        vkCmdBeginRendering(vk_cmd, &rinfo);
        vkCmdEndRendering(vk_cmd);
    }

    void CommandList::clear_depth(const DSVHandle& dsv, float depth)
    {
        if (vk_cmd == VK_NULL_HANDLE || !dsv.is_valid()) return;

        auto& ri  = dsv.get_resource_info();
        auto* dv  = std::get_if<Views::DepthStencil>(&ri.view);
        if (!dv || !dv->Resource) return;

        auto& api = static_cast<API::Resource&>(*dv->Resource);
        VkImageView view = api.get_import_handle().image_view;
        if (view == VK_NULL_HANDLE) return;

        end_rendering_if_active();

        VkClearValue cv;
        cv.depthStencil = { depth, 0 };

        VkRenderingAttachmentInfo att{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        att.imageView   = view;
        att.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        att.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
        att.clearValue  = cv;

        VkExtent2D ext = api.get_imported_extent();

        VkRenderingInfo rinfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        rinfo.renderArea      = { {0, 0}, ext };
        rinfo.layerCount      = 1;
        rinfo.pDepthAttachment = &att;

        vkCmdBeginRendering(vk_cmd, &rinfo);
        vkCmdEndRendering(vk_cmd);
    }

    void CommandList::clear_depth_stencil(const DSVHandle& dsv, bool d, bool s, float fd, UINT8 fs)
    {
        if (d) clear_depth(dsv, fd);
    }
    void CommandList::clear_stencil(const DSVHandle& dsv, UINT8) {}
    void CommandList::clear_uav(const UAVHandle&, vec4) {}

    // ---- Lazy render-pass start (for draw calls) ----------------------------

    void CommandList::ensure_rendering_active()
    {
        if (in_render_pass) return;
        if (current_color_view == VK_NULL_HANDLE && current_depth_view == VK_NULL_HANDLE) return;
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

        if (has_scissor)
            vkCmdSetScissor(vk_cmd, 0, 1, &current_scissor);
        else if (current_extent.width && current_extent.height)
        {
            VkRect2D sc{ {0,0}, current_extent };
            vkCmdSetScissor(vk_cmd, 0, 1, &sc);
        }

        // Re-push the staged push-constant block (carries the bindless descriptor
        // indices the shader reads as _hal_push.sN).  128 bytes = the range declared
        // by the root signature's VkPushConstantRange.
        if (current_pipeline_layout != VK_NULL_HANDLE)
            vkCmdPushConstants(vk_cmd, current_pipeline_layout, VK_SHADER_STAGE_ALL,
                               0, 128, push_constants.data());
    }

    void CommandList::flush_descriptor_sets()
    {
        if (!descriptor_sets_dirty || current_pipeline_layout == VK_NULL_HANDLE) return;
        descriptor_sets_dirty = false;

        VkDescriptorSet sets[2] = { cbv_srv_uav_set, sampler_set };
        uint32_t set_count = 0;
        if (sets[0] != VK_NULL_HANDLE) set_count = 1;
        if (sets[1] != VK_NULL_HANDLE) set_count = 2;
        if (set_count == 0) return;

        // Bind for both graphics and compute so set is available regardless of pipeline type.
        vkCmdBindDescriptorSets(vk_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                current_pipeline_layout, 0, set_count, sets, 0, nullptr);
    }

    // ---- Pipeline binding ---------------------------------------------------

    void CommandList::set_pipeline(std::shared_ptr<TrackedPipeline> pipeline)
    {
        if (!pipeline || vk_cmd == VK_NULL_HANDLE) return;
        if (pipeline->vk_pipeline == VK_NULL_HANDLE) return;

        VkPipelineBindPoint bind_point = pipeline->is_compute
            ? VK_PIPELINE_BIND_POINT_COMPUTE
            : VK_PIPELINE_BIND_POINT_GRAPHICS;

        if (!pipeline->is_compute)
            current_graphics_pipeline = pipeline->vk_pipeline;

        vkCmdBindPipeline(vk_cmd, bind_point, pipeline->vk_pipeline);
        flush_descriptor_sets();
    }

    void CommandList::set_graphics_signature(const HAL::RootSignature::ptr& sig)
    {
        if (!sig) return;
        current_pipeline_layout = static_cast<API::RootSignature&>(*sig).get_vk_pipeline_layout();
        descriptor_sets_dirty   = true;
    }

    void CommandList::set_compute_signature(const HAL::RootSignature::ptr& sig)
    {
        if (!sig) return;
        current_pipeline_layout = static_cast<API::RootSignature&>(*sig).get_vk_pipeline_layout();
        descriptor_sets_dirty   = true;
    }

    // ---- Descriptor heap binding -------------------------------------------

    void CommandList::set_descriptor_heaps(DescriptorHeap* cbv, DescriptorHeap* sampler)
    {
        auto* api_cbv     = static_cast<API::DescriptorHeap*>(cbv);
        auto* api_sampler = static_cast<API::DescriptorHeap*>(sampler);

        cbv_srv_uav_set = api_cbv     ? api_cbv->get_vk_set()     : VK_NULL_HANDLE;
        sampler_set     = api_sampler ? api_sampler->get_vk_set() : VK_NULL_HANDLE;
        descriptor_sets_dirty = true;
    }

    // ---- Draw / dispatch ---------------------------------------------------

    void CommandList::draw(UINT vertex_count, UINT vertex_offset,
                            UINT instance_count, UINT instance_offset)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        ensure_rendering_active();
        if (!in_render_pass) return; // no RTV set — skip rather than crash validation
        reapply_draw_state();
        flush_descriptor_sets();
        vkCmdDraw(vk_cmd, vertex_count, instance_count, vertex_offset, instance_offset);
    }

    void CommandList::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset,
                                    UINT instance_count, UINT instance_offset)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        ensure_rendering_active();
        if (!in_render_pass) return; // no RTV set — skip rather than crash validation
        reapply_draw_state();
        if (current_index_buffer != VK_NULL_HANDLE)
            vkCmdBindIndexBuffer(vk_cmd, current_index_buffer, current_index_offset, current_index_type);
        flush_descriptor_sets();
        vkCmdDrawIndexed(vk_cmd, index_count, instance_count,
                         index_offset, static_cast<int32_t>(vertex_offset), instance_offset);
    }

    void CommandList::dispatch(ivec3 v)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        // Compute doesn't use a render pass — end any active one first.
        end_rendering_if_active();
        if (current_pipeline_layout != VK_NULL_HANDLE &&
            (cbv_srv_uav_set != VK_NULL_HANDLE || sampler_set != VK_NULL_HANDLE))
        {
            VkDescriptorSet sets[2] = { cbv_srv_uav_set, sampler_set };
            uint32_t count = (sampler_set != VK_NULL_HANDLE) ? 2 : 1;
            vkCmdBindDescriptorSets(vk_cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                                    current_pipeline_layout, 0, count, sets, 0, nullptr);
        }
  /*      vkCmdDispatch(vk_cmd, static_cast<uint32_t>(v.x),
                               static_cast<uint32_t>(v.y),
                               static_cast<uint32_t>(v.z));*/
    }

    void CommandList::dispatch_mesh(ivec3 /*v*/)
    {
        // VK_EXT_mesh_shader not yet requested — Phase 5.
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
        current_index_offset = index.OffsetInBytes;
        current_index_type   = idx_type;
        vkCmdBindIndexBuffer(vk_cmd, api_res.get_vk_buffer(),
                             index.OffsetInBytes, idx_type);
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
        current_scissor = scissor;   // remember for per-draw re-apply (CB-split safe)
        has_scissor     = true;
        vkCmdSetScissor(vk_cmd, 0, 1, &scissor);
    }

    void CommandList::set_stencil_ref(UINT ref)
    {
        if (vk_cmd != VK_NULL_HANDLE)
            vkCmdSetStencilReference(vk_cmd, VK_STENCIL_FACE_FRONT_AND_BACK, ref);
    }

    // ---- Push constants ---------------------------------------------------

    void CommandList::graphics_set_constant(UINT slot, UINT offset, UINT value)
    {
        if (vk_cmd == VK_NULL_HANDLE || current_pipeline_layout == VK_NULL_HANDLE) return;
        uint32_t byte_offset = (slot + offset) * sizeof(uint32_t);
        // Stage so reapply_draw_state() can re-push before each draw — vkCmdPushConstants
        // does not survive a command-buffer split; without re-pushing, the shader reads 0
        // for every bindless descriptor index (s4, …) → all bindless reads fail → black UI.
        if ((slot + offset) < push_constants.size())
            push_constants[slot + offset] = value;
        vkCmdPushConstants(vk_cmd, current_pipeline_layout,
                           VK_SHADER_STAGE_ALL, // must cover all stages in pipeline layout range
                           byte_offset, sizeof(uint32_t), &value);
    }

    void CommandList::compute_set_constant(UINT slot, UINT offset, UINT value)
    {
        if (vk_cmd == VK_NULL_HANDLE || current_pipeline_layout == VK_NULL_HANDLE) return;
        uint32_t byte_offset = (slot + offset) * sizeof(uint32_t);
        vkCmdPushConstants(vk_cmd, current_pipeline_layout,
                           VK_SHADER_STAGE_ALL, // must cover all stages in pipeline layout range
                           byte_offset, sizeof(uint32_t), &value);
    }

    // Phase 5: push descriptors for inline CBV binding
    void CommandList::graphics_set_const_buffer(UINT, const ResourceAddress&) {}
    void CommandList::compute_set_const_buffer(UINT, const ResourceAddress&)  {}

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
            region.size = dest->get_desc().as_buffer().SizeInBytes;
            vkCmdCopyBuffer(vk_cmd, src_api.get_vk_buffer(),
                            dst_api.get_vk_buffer(), 1, &region);
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
        region.srcOffset = src_offset;
        region.dstOffset = dest_offset;
        region.size      = size;
        vkCmdCopyBuffer(vk_cmd, src_api.get_vk_buffer(),
                        dst_api.get_vk_buffer(), 1, &region);
    }

    void CommandList::update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box,
                                      UINT sub_resource, ResourceAddress address,
                                      texture_layout layout)
    {
        if (!resource || vk_cmd == VK_NULL_HANDLE) return;
        auto& dst = static_cast<API::Resource&>(*resource);
        if (dst.get_vk_image() == VK_NULL_HANDLE) return;

        // `address` is a GPU address to a staging buffer; we need the VkBuffer.
        // Phase 5: look up staging buffer from the address.
        // For now this is a stub — real implementation needs the staging buffer handle.
    }

    void CommandList::read_texture(const HAL::Resource*, ivec3, ivec3, UINT,
                                    ResourceAddress, texture_layout) {}

    void CommandList::copy_texture(const Resource::ptr& dest, int dest_sub,
                                    const Resource::ptr& source, int src_sub)
    {
        if (!dest || !source || vk_cmd == VK_NULL_HANDLE) return;
        auto& dst = static_cast<API::Resource&>(*dest);
        auto& src = static_cast<API::Resource&>(*source);
        if (dst.get_vk_image() == VK_NULL_HANDLE || src.get_vk_image() == VK_NULL_HANDLE) return;

        // Use imported_extent for the copy region (covers swapchain backbuffers).
        // For regular textures, get_imported_extent() falls back to {0,0}; use 1 as minimum.
        VkExtent2D ext = dst.get_imported_extent();
        VkImageCopy region{};
        region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.extent         = { ext.width  ? ext.width  : 1,
                                   ext.height ? ext.height : 1, 1 };
        vkCmdCopyImage(vk_cmd,
            src.get_vk_image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dst.get_vk_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
        region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.srcOffset      = { src_pos.x, src_pos.y, src_pos.z };
        region.dstOffset      = { dest_pos.x, dest_pos.y, dest_pos.z };
        region.extent         = { static_cast<uint32_t>(size.x),
                                   static_cast<uint32_t>(size.y),
                                   static_cast<uint32_t>(size.z) };
        vkCmdCopyImage(vk_cmd,
            src.get_vk_image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dst.get_vk_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region);
    }

    // ---- Set topology (stored for potential dynamic topology) ---------------
    void CommandList::set_topology(HAL::PrimitiveTopologyType, HAL::PrimitiveTopologyFeed,
                                    bool, uint) {}

    // ---- Debug labels (VK_EXT_debug_utils) ----------------------------------

    void CommandList::start_event(std::wstring_view name)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        auto* dev = static_cast<API::Device*>(m_device);
        if (!dev) return;
        auto fn = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
            vkGetDeviceProcAddr(dev->get_native_device(), "vkCmdBeginDebugUtilsLabelEXT"));
        if (!fn) return;
        // Narrow wstring → UTF-8 for Vulkan
        std::string label(name.begin(), name.end());
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
    void CommandList::execute_indirect(const IndirectCommand&, UINT, Resource*, UINT64,
                                        Resource*, UINT64) {}

    // ---- Misc ---------------------------------------------------------------
    void CommandList::set_name(std::wstring_view) {}
    void CommandList::discard(const HAL::Resource*) {}
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
        // Phase 5: vkCmdCopyQueryPoolResults requires a VkBuffer handle,
        // but ResourceAddress is a GPU virtual address.  The device-address →
        // VkBuffer reverse-lookup is not yet implemented.  Leave as stub until
        // that infrastructure is in place.
        (void)heap; (void)count; (void)dest;
    }
}
