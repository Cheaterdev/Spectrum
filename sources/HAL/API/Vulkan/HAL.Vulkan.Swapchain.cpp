module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
module HAL:SwapChain;

import Core;
import HAL;

// ============================================================================
//  Vulkan swapchain implementation (Phase 2).
//  Integrates with the existing HAL::SwapChain common wrapper:
//    - frames[i].m_renderTarget  — TextureResource wrapping each backbuffer
//    - frames[i].fence_event     — FenceWaiter for CPU-side frame pacing
//    - m_frameIndex              — toggles between frames in flight
// ============================================================================

namespace
{
    VkSemaphore make_semaphore(VkDevice dev)
    {
        VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VkSemaphore sem = VK_NULL_HANDLE;
        vkCreateSemaphore(dev, &info, nullptr, &sem);
        return sem;
    }

    // Record a one-time COLOR_ATTACHMENT_OPTIMAL → PRESENT_SRC_KHR barrier into cb
    // for the given swapchain image.  The command buffer must already be in recording
    // state.  Uses SIMULTANEOUS_USE_BIT so the same CB can be in-flight while we re-submit it.
    void record_present_transition(VkCommandBuffer cb, VkImage image)
    {
        VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        barrier.srcStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstStageMask  = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        barrier.dstAccessMask = 0;
        barrier.oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.image         = image;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dep.imageMemoryBarrierCount = 1;
        dep.pImageMemoryBarriers    = &barrier;
        vkCmdPipelineBarrier2(cb, &dep);
    }
}

namespace HAL
{
    // Forward declaration — defined below in present().
    static bool do_acquire(VkDevice, VkSwapchainKHR, VkSemaphore,
                           uint32_t& out_image, uint32_t& out_frame_index);

    // ---- VkResult diagnostics -----------------------------------------------
    static const char* vk_result_string(VkResult r)
    {
        switch (r)
        {
        case VK_SUCCESS:                        return "VK_SUCCESS";
        case VK_NOT_READY:                      return "VK_NOT_READY";
        case VK_TIMEOUT:                        return "VK_TIMEOUT";
        case VK_SUBOPTIMAL_KHR:                 return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_HOST_MEMORY:       return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:    return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:              return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_SURFACE_LOST_KHR:         return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:          return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:    return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:        return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_UNKNOWN:                  return "VK_ERROR_UNKNOWN";
        default:                                return "<unknown VkResult>";
        }
    }

    // Log a VkResult and dump the key swapchain create-info fields so the
    // exact failure reason is always visible in the log.
    static void log_swapchain_result(VkResult r,
                                     const VkSwapchainCreateInfoKHR& ci,
                                     const VkSurfaceCapabilitiesKHR& caps)
    {
        Log::get() << "[Vulkan swapchain] vkCreateSwapchainKHR → "
                   << vk_result_string(r) << " (" << static_cast<int>(r) << ")"
                   << Log::endl;
        if (r == VK_SUCCESS) return;

        Log::get() << "  extent            : "
                   << ci.imageExtent.width << " x " << ci.imageExtent.height << Log::endl;
        Log::get() << "  surface extent    : "
                   << caps.currentExtent.width << " x " << caps.currentExtent.height
                   << "  min=" << caps.minImageExtent.width << "x" << caps.minImageExtent.height
                   << "  max=" << caps.maxImageExtent.width << "x" << caps.maxImageExtent.height
                   << Log::endl;
        Log::get() << "  minImageCount     : " << caps.minImageCount
                   << "  maxImageCount="      << caps.maxImageCount
                   << "  requested="          << ci.minImageCount << Log::endl;
        Log::get() << "  supportedUsage    : 0x" << std::hex << caps.supportedUsageFlags
                   << "  requested=0x"           << ci.imageUsage << std::dec << Log::endl;
        Log::get() << "  supportedAlpha    : 0x" << std::hex << caps.supportedCompositeAlpha
                   << "  requested=0x"           << ci.compositeAlpha << std::dec << Log::endl;
        Log::get() << "  supportedTransform: 0x" << std::hex << caps.supportedTransforms
                   << "  currentTransform=0x"    << caps.currentTransform << std::dec << Log::endl;
    }

    SwapChain::SwapChain(Device& device, swap_chain_desc c_desc) : device(device)
    {
        auto& api_dev = static_cast<API::Device&>(device);
        if (api_dev.vk_device == VK_NULL_HANDLE) return;

        VkInstance instance = api_dev.vk_instance;

        // ---- Win32 surface --------------------------------------------------
        VkWin32SurfaceCreateInfoKHR surface_ci{
            VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
        surface_ci.hinstance = GetModuleHandle(nullptr);
        surface_ci.hwnd      = c_desc.window->get_hwnd();
        vkCreateWin32SurfaceKHR(instance, &surface_ci, nullptr, &vk_surface);

        // ---- Surface capabilities + format ----------------------------------
        VkSurfaceCapabilitiesKHR caps{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            api_dev.vk_physical, vk_surface, &caps);

        uint32_t fmt_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            api_dev.vk_physical, vk_surface, &fmt_count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(fmt_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            api_dev.vk_physical, vk_surface, &fmt_count, formats.data());

        // Pick the best available surface format.
        // Priority: BGRA8_UNORM → RGBA8_UNORM → BGRA8_SRGB → RGBA8_SRGB → first.
        // BGRA8_UNORM matches the D3D12/DXGI convention (DXGI promotes R8G8B8A8 to
        // B8G8R8A8 anyway) and is the native Windows DWM format, so it is first.
        // UI PSOs are compiled with B8G8R8A8_UNORM to match both backends.
        vk_format      = formats[0].format;
        vk_color_space = formats[0].colorSpace;
        VkColorSpaceKHR color_space = formats[0].colorSpace;
        {
            constexpr VkFormat preferred[] = {
                VK_FORMAT_B8G8R8A8_UNORM,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_FORMAT_B8G8R8A8_SRGB,
                VK_FORMAT_R8G8B8A8_SRGB,
            };
            bool found = false;
            for (VkFormat want : preferred)
            {
                for (auto& f : formats)
                {
                    if (f.format == want &&
                        f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    {
                        vk_format      = f.format;
                        vk_color_space = f.colorSpace;
                        color_space    = f.colorSpace;
                        found = true; break;
                    }
                }
                if (found) break;
            }
        }
        Log::get() << "[Vulkan swapchain] " << fmt_count << " surface formats available"
                   << "; selected format=" << vk_format << Log::endl;

        // Present mode: prefer MAILBOX, fall back to FIFO (always available).
        uint32_t pm_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            api_dev.vk_physical, vk_surface, &pm_count, nullptr);
        std::vector<VkPresentModeKHR> present_modes(pm_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            api_dev.vk_physical, vk_surface, &pm_count, present_modes.data());
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        for (auto pm : present_modes)
            if (pm == VK_PRESENT_MODE_MAILBOX_KHR) { present_mode = pm; break; }

        // ---- Swapchain ------------------------------------------------------
        // Request double-buffering (or what the surface supports).
        uint32_t desired_images = std::max(2u, caps.minImageCount);
        if (caps.maxImageCount > 0)
            desired_images = std::min(desired_images, caps.maxImageCount);

        // Extent: when currentExtent is not the "app chooses" sentinel
        // (0xFFFFFFFF) the compositor dictates the size — use it directly.
        // Otherwise fall back to GetClientRect, clamped to surface limits.
        VkExtent2D extent{};
        if (caps.currentExtent.width != UINT32_MAX &&
            caps.currentExtent.height != UINT32_MAX)
        {
            extent = caps.currentExtent;
        }
        else
        {
            RECT r{};
            GetClientRect(c_desc.window->get_hwnd(), &r);
            extent = {
                std::clamp(static_cast<uint32_t>(std::max(0L, r.right  - r.left)),
                           caps.minImageExtent.width,  caps.maxImageExtent.width),
                std::clamp(static_cast<uint32_t>(std::max(0L, r.bottom - r.top)),
                           caps.minImageExtent.height, caps.maxImageExtent.height)
            };
        }

        Log::get() << "[Vulkan swapchain] caps.currentExtent="
                   << caps.currentExtent.width << "x" << caps.currentExtent.height
                   << "  selected extent=" << extent.width << "x" << extent.height
                   << "  minImage=" << caps.minImageCount
                   << "  maxImage=" << caps.maxImageCount
                   << Log::endl;

        // If the extent is still 0 (window minimised / not yet shown), we
        // cannot create a swapchain.  Pre-fill `frames` with dummy slots so
        // wait_for_free() / get_current_frame() don't crash on empty access.
        if (extent.width == 0 || extent.height == 0)
        {
            Log::get() << "[Vulkan swapchain] zero-size window — deferring "
                          "swapchain creation; pre-allocating " << desired_images
                       << " dummy frames" << Log::endl;
            frames.resize(desired_images);   // FenceWaiter default-constructs to already-signalled
            m_frameIndex = 0;
            return;
        }

        // Pick a supported compositeAlpha; OPAQUE is most common but not universal.
        VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        for (auto flag : { VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                           VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                           VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                           VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR })
        {
            if (caps.supportedCompositeAlpha & flag)
            { composite_alpha = flag; break; }
        }

        VkSwapchainCreateInfoKHR sc_ci{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        sc_ci.surface          = vk_surface;
        sc_ci.minImageCount    = desired_images;
        sc_ci.imageFormat      = vk_format;
        sc_ci.imageColorSpace  = color_space;
        sc_ci.imageExtent      = extent;
        sc_ci.imageArrayLayers = 1;
        sc_ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT     |
                                  VK_IMAGE_USAGE_SAMPLED_BIT;          // needed for SRV in descriptor heap
        sc_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        sc_ci.preTransform     = caps.currentTransform;
        sc_ci.compositeAlpha   = composite_alpha;
        sc_ci.presentMode      = present_mode;
        sc_ci.clipped          = VK_TRUE;

        sc_extent = extent;  // store for on_change() and resize()
        {
            VkResult r = vkCreateSwapchainKHR(api_dev.vk_device, &sc_ci, nullptr, &vk_swapchain);
            log_swapchain_result(r, sc_ci, caps);
            if (r != VK_SUCCESS) return;
        }

        // ---- Images + views -------------------------------------------------
        vkGetSwapchainImagesKHR(api_dev.vk_device, vk_swapchain, &image_count, nullptr);
        swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(
            api_dev.vk_device, vk_swapchain, &image_count, swapchain_images.data());

        swapchain_views.resize(image_count);
        for (uint32_t i = 0; i < image_count; ++i)
        {
            VkImageViewCreateInfo view_ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            view_ci.image            = swapchain_images[i];
            view_ci.viewType         = VK_IMAGE_VIEW_TYPE_2D;
            view_ci.format           = vk_format;
            view_ci.components       = { VK_COMPONENT_SWIZZLE_IDENTITY,
                                          VK_COMPONENT_SWIZZLE_IDENTITY,
                                          VK_COMPONENT_SWIZZLE_IDENTITY,
                                          VK_COMPONENT_SWIZZLE_IDENTITY };
            view_ci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            vkCreateImageView(api_dev.vk_device, &view_ci, nullptr, &swapchain_views[i]);
        }

        // ---- Sync semaphores ------------------------------------------------
        // image_available: free-running ring of (image_count + 1) — see .ixx.
        // render_finished: one per swapchain image (waited by vkQueuePresentKHR).
        image_available.resize(image_count + 1);
        render_finished.resize(image_count);
        for (auto& s : image_available) s = make_semaphore(api_dev.vk_device);
        for (auto& s : render_finished) s = make_semaphore(api_dev.vk_device);
        acquire_counter = 0;

        // ---- Wrap backbuffers as TextureResources ---------------------------
        frames.resize(image_count);
        m_frameIndex = 0;
        on_change();

        // ---- Present-transition command buffers & semaphores ----------------
        setup_present_commands(api_dev.vk_device, api_dev.get_queue_family(
            static_cast<int>(CommandListType::DIRECT)));

        // Transition all fresh images UNDEFINED → PRESENT_SRC_KHR so that
        // on_change()'s TextureLayout::PRESENT assumption is valid on frame 0.
        {
            auto& aq = static_cast<API::Queue&>(*device.get_queue(CommandListType::DIRECT));
            init_swapchain_layouts(api_dev.vk_device, aq.get_native());
        }

        Log::get() << "Vulkan swapchain: " << image_count << " images, "
                   << extent.width << "x" << extent.height << Log::endl;

        // Pre-acquire the first image so m_frameIndex is valid before the
        // first wait_for_free() + get_current_frame() calls.  Use ring slot 0.
        do_acquire(api_dev.vk_device, vk_swapchain,
                   image_available[acquire_counter % image_available.size()],
                   current_image, m_frameIndex);

        // Wire the image-available wait semaphore into the first execute() call so the
        // GPU waits for the presentation engine before writing to the swapchain image.
        // render_finished is signalled explicitly in present() (not via pending_signal_sem)
        // so it always fires after ALL render command lists — not just the first submit.
        auto& api_queue = static_cast<API::Queue&>(*device.get_queue(CommandListType::DIRECT));
        api_queue.set_frame_semaphores(
            image_available[acquire_counter % image_available.size()], VK_NULL_HANDLE);
        ++acquire_counter;
    }

    void SwapChain::on_change()
    {
        // Wrap each swapchain image as a TextureResource via NativeImportHandle.
        for (uint32_t i = 0; i < image_count; ++i)
        {
            API::NativeImportHandle handle;
            handle.image      = swapchain_images[i];
            handle.image_view = swapchain_views[i];
            handle.format     = vk_format;
            handle.extent     = sc_extent;

            frames[i].m_renderTarget.reset(
                // PRESENT: We explicitly transition all fresh swapchain images to
                // PRESENT_SRC_KHR after creation (see init_swapchain_layouts below), so
                // PRESENT is accurate here.  The frame graph will emit the correct
                // PRESENT_SRC_KHR→COLOR_ATTACHMENT_OPTIMAL barrier on first render.
                new TextureResource(device, handle, TextureLayout::PRESENT));
            frames[i].m_renderTarget->set_name(
                std::string("swapchain_") + std::to_string(i));
        }
    }

    // Helper: acquire the next swapchain image and update m_frameIndex.
    // Called at the end of present() (for the NEXT frame) and once in the
    // constructor (for the very first frame).  Mirrors D3D12's
    // GetCurrentBackBufferIndex() pattern so wait_for_free() works unchanged.
    static bool do_acquire(VkDevice vk_device, VkSwapchainKHR vk_swapchain,
                            VkSemaphore sem,
                            uint32_t& out_image, uint32_t& out_frame_index)
    {
        VkResult r = vkAcquireNextImageKHR(vk_device, vk_swapchain,
                                            UINT64_MAX, sem,
                                            VK_NULL_HANDLE, &out_image);
        if (r == VK_SUCCESS || r == VK_SUBOPTIMAL_KHR)
        {
            out_frame_index = out_image;
            return true;
        }
        return false;   // VK_ERROR_OUT_OF_DATE_KHR or worse
    }

    // ---- Present-transition helpers (defined in HAL::API namespace) -----------
} // namespace HAL
namespace HAL::API
{
    void SwapChain::setup_present_commands(VkDevice vk_dev, uint32_t queue_family)
    {
        if (vk_dev == VK_NULL_HANDLE || image_count == 0) return;

        // Pool (not pooled with the frame-graph pools — owned exclusively by present())
        VkCommandPoolCreateInfo pool_ci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        pool_ci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_ci.queueFamilyIndex = queue_family;
        vkCreateCommandPool(vk_dev, &pool_ci, nullptr, &present_pool);
        if (present_pool == VK_NULL_HANDLE) return;

        present_cbs.resize(image_count, VK_NULL_HANDLE);
        present_sems.resize(image_count, VK_NULL_HANDLE);

        VkCommandBufferAllocateInfo alloc_ci{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        alloc_ci.commandPool        = present_pool;
        alloc_ci.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_ci.commandBufferCount = image_count;
        vkAllocateCommandBuffers(vk_dev, &alloc_ci, present_cbs.data());

        // Allocate command buffers but do NOT pre-record them.
        // They are reset and re-recorded fresh in present() each frame so the
        // driver always associates the layout transition with the current
        // vkAcquireNextImageKHR acquisition cycle.  SIMULTANEOUS_USE_BIT is
        // intentionally omitted — we guarantee no re-submission before the
        // previous frame's present has completed by waiting on render_finished.
        for (uint32_t i = 0; i < image_count; ++i)
            present_sems[i] = make_semaphore(vk_dev);
    }

    void SwapChain::init_swapchain_layouts(VkDevice vk_dev, VkQueue vk_queue)
    {
        // Transition every fresh swapchain image from UNDEFINED to PRESENT_SRC_KHR
        // so that on_change()'s TextureLayout::PRESENT initial-state assumption holds
        // on frame 0.
        //
        // The Vulkan spec forbids transitioning a presentable image that has not been
        // acquired (VUID-vkCmdPipelineBarrier2-image-09373).  We therefore loop:
        //   acquire → transition UNDEFINED→PRESENT_SRC_KHR → present back
        // until every image index has been visited.  After this function returns all
        // images are in PRESENT_SRC_KHR and owned by the presentation engine; the
        // caller then does one final do_acquire() to claim the first rendering image.
        if (vk_dev       == VK_NULL_HANDLE || vk_queue == VK_NULL_HANDLE ||
            vk_swapchain == VK_NULL_HANDLE || image_count == 0 ||
            present_pool == VK_NULL_HANDLE || present_cbs.size() < image_count)
            return;

        // Allocate one acquire-semaphore and one present-semaphore per iteration.
        // Upper bound: visit each image at least once; allow 4× slack for MAILBOX mode
        // where the presentation engine may return the same image several times before
        // handing back the others.
        const uint32_t max_iters = image_count * 4;
        std::vector<VkSemaphore> acq_sems (max_iters, VK_NULL_HANDLE);
        std::vector<VkSemaphore> pres_sems(max_iters, VK_NULL_HANDLE);
        for (auto& s : acq_sems ) s = make_semaphore(vk_dev);
        for (auto& s : pres_sems) s = make_semaphore(vk_dev);

        std::vector<bool> done(image_count, false);
        uint32_t          done_count = 0;

        for (uint32_t iter = 0; iter < max_iters && done_count < image_count; ++iter)
        {
            VkSemaphore acq_sem  = acq_sems [iter];
            VkSemaphore pres_sem = pres_sems[iter];

            uint32_t img_idx = 0;
            VkResult r = vkAcquireNextImageKHR(vk_dev, vk_swapchain,
                                               UINT64_MAX, acq_sem,
                                               VK_NULL_HANDLE, &img_idx);
            if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) break;

            // Re-use the per-image present CB slot: reset and re-record.
            // For a newly-seen image record UNDEFINED→PRESENT_SRC_KHR; for an
            // already-done image record an empty CB (we still need to consume
            // acq_sem before presenting).
            VkCommandBuffer cb = present_cbs[img_idx];
            vkResetCommandBuffer(cb, 0);
            {
                VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
                bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                vkBeginCommandBuffer(cb, &bi);

                if (!done[img_idx])
                {
                    VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
                    barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
                    barrier.srcAccessMask = 0;
                    barrier.dstStageMask  = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
                    barrier.dstAccessMask = 0;
                    barrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
                    barrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    barrier.image         = swapchain_images[img_idx];
                    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

                    VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
                    dep.imageMemoryBarrierCount = 1;
                    dep.pImageMemoryBarriers    = &barrier;
                    vkCmdPipelineBarrier2(cb, &dep);

                    done[img_idx] = true;
                    ++done_count;
                }

                vkEndCommandBuffer(cb);
            }

            // Submit: wait on the acquire semaphore, signal the present semaphore.
            VkSemaphoreSubmitInfo wait_si{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
            wait_si.semaphore = acq_sem;
            wait_si.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;

            VkSemaphoreSubmitInfo sig_si{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
            sig_si.semaphore = pres_sem;
            sig_si.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

            VkCommandBufferSubmitInfo cb_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
            cb_info.commandBuffer = cb;

            VkSubmitInfo2 submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
            submit.waitSemaphoreInfoCount   = 1;
            submit.pWaitSemaphoreInfos      = &wait_si;
            submit.commandBufferInfoCount   = 1;
            submit.pCommandBufferInfos      = &cb_info;
            submit.signalSemaphoreInfoCount = 1;
            submit.pSignalSemaphoreInfos    = &sig_si;
            vkQueueSubmit2(vk_queue, 1, &submit, VK_NULL_HANDLE);

            // Present the image back so the presentation engine can return it
            // on the next acquire (necessary to cycle through all image indices).
            VkPresentInfoKHR pi{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
            pi.waitSemaphoreCount = 1;
            pi.pWaitSemaphores    = &pres_sem;
            pi.swapchainCount     = 1;
            pi.pSwapchains        = &vk_swapchain;
            pi.pImageIndices      = &img_idx;
            vkQueuePresentKHR(vk_queue, &pi);
        }

        // Synchronous flush: all presents above are now complete before we return.
        vkQueueWaitIdle(vk_queue);

        // Destroy the temporary semaphores (safe: all GPU work has finished).
        for (auto s : acq_sems ) if (s) vkDestroySemaphore(vk_dev, s, nullptr);
        for (auto s : pres_sems) if (s) vkDestroySemaphore(vk_dev, s, nullptr);
    }

    void SwapChain::teardown_present_commands(VkDevice vk_dev)
    {
        if (vk_dev == VK_NULL_HANDLE) return;
        for (auto s : present_sems)
            if (s) vkDestroySemaphore(vk_dev, s, nullptr);
        present_sems.clear();
        present_cbs.clear(); // freed with pool
        if (present_pool) { vkDestroyCommandPool(vk_dev, present_pool, nullptr); present_pool = VK_NULL_HANDLE; }
    }

} // namespace HAL::API
namespace HAL
{
    // ---- SwapChain::present() -----------------------------------------------

    void SwapChain::present()
    {
        auto& api_dev   = static_cast<API::Device&>(device);
        auto  gfx_queue = device.get_queue(CommandListType::DIRECT);
        auto& api_queue = static_cast<API::Queue&>(*gfx_queue);

        if (vk_swapchain == VK_NULL_HANDLE || image_available.empty())
            return;   // zero-size / failed swapchain — nothing to present.

        // Index of the image we are about to present (still identifies this frame's
        // image; the acquire below updates current_image to the NEXT frame's image).
        const uint32_t presented_image = current_image;

        // ===================================================================
        //  CRITICAL ORDERING.
        //  HAL::Queue::execute() does NOT submit synchronously — it ENQUEUES the
        //  render command buffers onto the queue's single gpu_execute_thread (a FIFO
        //  worker).  If we called vkQueuePresentKHR / vkAcquireNextImageKHR here on
        //  the *main* thread we would present and re-acquire BEFORE the render was
        //  even submitted: the presentation engine shows an un-rendered (black) image,
        //  and validation reports "image not acquired" / "semaphore not waited on".
        //
        //  So we enqueue the ENTIRE present sequence (signal render_finished, present,
        //  acquire next, wire the next acquire-wait semaphore) onto that same FIFO via
        //  run().  It therefore executes AFTER all render submits, exactly like the
        //  D3D12 backend routes Present through queue->run().  Acquire and present run
        //  on the same thread, satisfying the swapchain's external-sync requirement,
        //  and pending_wait_sem is only ever touched on the worker thread (no race
        //  with execute_internal).  A promise hands the freshly-acquired frame index
        //  back to the main thread before present() returns, so the next frame's
        //  get_current_frame()/compile() read a valid m_frameIndex.
        // ===================================================================
        const VkDevice vk_dev = api_dev.vk_device;
        API::Queue*    q      = &api_queue;
        auto idx_promise = std::make_shared<std::promise<void>>();
        std::future<void> idx_future = idx_promise->get_future();

        gfx_queue->run([this, vk_dev, q, presented_image, idx_promise]()
        {
            API::Queue& api_queue = *q;
            // 1) Signal render_finished[presented] after all render CBs (same-thread
            //    FIFO guarantees those submits already happened).
            if (presented_image < static_cast<uint32_t>(render_finished.size()) &&
                render_finished[presented_image] != VK_NULL_HANDLE)
            {
                VkSemaphoreSubmitInfo rf_sig{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                rf_sig.semaphore = render_finished[presented_image];
                rf_sig.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                VkSubmitInfo2 rf_submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
                rf_submit.signalSemaphoreInfoCount = 1;
                rf_submit.pSignalSemaphoreInfos    = &rf_sig;
                api_queue.submit_raw(rf_submit);
            }

            // 2) Present, waiting on render_finished[presented].  The frame graph's
            //    non_tracked_resources loop already left the image in PRESENT_SRC_KHR.
            VkSemaphore      wait_sem = render_finished[presented_image];
            uint32_t         img      = presented_image;
            VkPresentInfoKHR pi{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
            pi.swapchainCount     = 1;
            pi.pSwapchains        = &vk_swapchain;
            pi.pImageIndices      = &img;
            pi.waitSemaphoreCount = (wait_sem != VK_NULL_HANDLE) ? 1u : 0u;
            pi.pWaitSemaphores    = (wait_sem != VK_NULL_HANDLE) ? &wait_sem : nullptr;
            VkResult pr = api_queue.present(pi);

            // 3) Re-assert initial_layout=PRESENT so next frame's compile_transitions
            //    emits a fresh PRESENT→COLOR_ATTACHMENT barrier.
            if (presented_image < static_cast<uint32_t>(frames.size()) &&
                frames[presented_image].m_renderTarget)
            {
                frames[presented_image].m_renderTarget->get_state_manager()
                    .init_subres(1, TextureLayout::PRESENT);
            }

            // 4) Acquire the NEXT image (free-running ring slot — NOT keyed by image
            //    index) and wire its wait semaphore into the next render submit.
            const uint32_t sem_idx = acquire_counter % image_available.size();
            uint32_t next_image = current_image, next_idx = m_frameIndex;
            if (pr != VK_ERROR_OUT_OF_DATE_KHR &&
                do_acquire(vk_dev, vk_swapchain,
                           image_available[sem_idx], next_image, next_idx))
            {
                current_image = next_image;
                m_frameIndex  = next_idx;
                api_queue.set_frame_semaphores(image_available[sem_idx], VK_NULL_HANDLE);
                ++acquire_counter;
            }
            else
            {
                m_frameIndex = (m_frameIndex + 1) % image_count;
            }

            idx_promise->set_value();
        });

        // CPU fence so wait_for_free() can pace the next frame.  signal() enqueues on
        // the same FIFO AFTER the present task, so the ordering is correct.
        frames[presented_image].fence_event = gfx_queue->signal();

        // Block until the worker task has presented + acquired the next image and
        // updated m_frameIndex / current_image.  This is no more blocking than the
        // previous synchronous do_acquire() (it waits on image availability), but now
        // it also guarantees the render submit precedes the present on the GPU timeline.
        idx_future.wait();
    }

    void SwapChain::resize(ivec2 size)
    {
        if (size.x < 64) size.x = 64;
        if (size.y < 64) size.y = 64;

        // render() calls resize() every frame with the current window size.
        // Skip the expensive teardown+recreate when nothing actually changed.
        if (vk_swapchain != VK_NULL_HANDLE &&
            static_cast<int>(sc_extent.width)  == size.x &&
            static_cast<int>(sc_extent.height) == size.y)
            return;

        auto& api_dev = static_cast<API::Device&>(device);
        if (api_dev.vk_device == VK_NULL_HANDLE) return;

        // Wait for all queues idle before destroying swapchain resources.
        device.get_queue(CommandListType::DIRECT)->signal_and_wait();
        device.get_queue(CommandListType::COMPUTE)->signal_and_wait();
        device.get_queue(CommandListType::COPY)->signal_and_wait();

        // Destroy old views, semaphores, and present-transition resources.
        teardown_present_commands(api_dev.vk_device);
        for (uint32_t i = 0; i < image_count; ++i)
        {
            frames[i].m_renderTarget = nullptr;
            vkDestroyImageView(api_dev.vk_device, swapchain_views[i], nullptr);
            vkDestroySemaphore(api_dev.vk_device, render_finished[i],  nullptr);
        }
        // image_available is the (image_count + 1) acquire ring — destroy all of it.
        for (auto s : image_available)
            if (s) vkDestroySemaphore(api_dev.vk_device, s, nullptr);
        swapchain_views.clear();
        swapchain_images.clear();
        image_available.clear();
        render_finished.clear();

        // Re-create swapchain at new size (reuse existing surface + format).
        VkSurfaceCapabilitiesKHR caps{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(api_dev.vk_physical, vk_surface, &caps);

        VkExtent2D extent{
            std::clamp(static_cast<uint32_t>(size.x),
                       caps.minImageExtent.width,  caps.maxImageExtent.width),
            std::clamp(static_cast<uint32_t>(size.y),
                       caps.minImageExtent.height, caps.maxImageExtent.height)
        };

        VkSwapchainCreateInfoKHR sc_ci{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        sc_ci.surface          = vk_surface;
        sc_ci.minImageCount    = image_count;
        sc_ci.imageFormat      = vk_format;
        sc_ci.imageColorSpace  = vk_color_space;
        sc_ci.imageExtent      = extent;
        sc_ci.imageArrayLayers = 1;
        sc_ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT     |
                                  VK_IMAGE_USAGE_SAMPLED_BIT;          // needed for SRV in descriptor heap
        sc_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        sc_ci.preTransform     = caps.currentTransform;
        {
            VkCompositeAlphaFlagBitsKHR ca = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            for (auto f : { VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                             VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                             VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                             VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR })
                if (caps.supportedCompositeAlpha & f) { ca = f; break; }
            sc_ci.compositeAlpha = ca;
        }
        sc_ci.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
        sc_ci.clipped          = VK_TRUE;
        sc_ci.oldSwapchain     = vk_swapchain;  // lets driver reuse resources

        sc_extent = extent;  // update stored extent
        VkSwapchainKHR new_swapchain = VK_NULL_HANDLE;
        {
            VkResult r = vkCreateSwapchainKHR(api_dev.vk_device, &sc_ci, nullptr, &new_swapchain);
            log_swapchain_result(r, sc_ci, caps);
            if (r != VK_SUCCESS) return;
        }
        vkDestroySwapchainKHR(api_dev.vk_device, vk_swapchain, nullptr);
        vk_swapchain = new_swapchain;

        // Rebuild images / views / semaphores / wrappers.
        vkGetSwapchainImagesKHR(api_dev.vk_device, vk_swapchain, &image_count, nullptr);
        swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(
            api_dev.vk_device, vk_swapchain, &image_count, swapchain_images.data());

        swapchain_views.resize(image_count);
        image_available.resize(image_count + 1);
        render_finished.resize(image_count);
        for (uint32_t i = 0; i < image_count; ++i)
        {
            VkImageViewCreateInfo view_ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            view_ci.image            = swapchain_images[i];
            view_ci.viewType         = VK_IMAGE_VIEW_TYPE_2D;
            view_ci.format           = vk_format;
            view_ci.components       = { VK_COMPONENT_SWIZZLE_IDENTITY,
                                          VK_COMPONENT_SWIZZLE_IDENTITY,
                                          VK_COMPONENT_SWIZZLE_IDENTITY,
                                          VK_COMPONENT_SWIZZLE_IDENTITY };
            view_ci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            vkCreateImageView(api_dev.vk_device, &view_ci, nullptr, &swapchain_views[i]);

            render_finished[i] = make_semaphore(api_dev.vk_device);
        }
        // Acquire ring is one larger than image_count (see .ixx).
        for (auto& s : image_available) s = make_semaphore(api_dev.vk_device);
        acquire_counter = 0;

        frames.resize(image_count);
        m_frameIndex = 0;
        on_change();

        // Rebuild present-transition resources for the new images.
        setup_present_commands(api_dev.vk_device,
            api_dev.get_queue_family(static_cast<int>(CommandListType::DIRECT)));

        // Transition fresh images UNDEFINED → PRESENT_SRC_KHR.
        {
            auto& aq = static_cast<API::Queue&>(*device.get_queue(CommandListType::DIRECT));
            init_swapchain_layouts(api_dev.vk_device, aq.get_native());
        }

        // Null out the stale semaphore handles the queue is holding before we
        // re-acquire, so a racing execute() sees VK_NULL_HANDLE rather than
        // a destroyed semaphore.
        auto& api_queue = static_cast<API::Queue&>(*device.get_queue(CommandListType::DIRECT));
        api_queue.set_frame_semaphores(VK_NULL_HANDLE, VK_NULL_HANDLE);

        // Re-acquire the first image post-resize using ring slot 0, then advance
        // the counter so present() rotates through the rest of the ring.
        do_acquire(api_dev.vk_device, vk_swapchain,
                   image_available[acquire_counter % image_available.size()],
                   current_image, m_frameIndex);
        // Wire only the wait side; render_finished is signalled explicitly in present().
        api_queue.set_frame_semaphores(
            image_available[acquire_counter % image_available.size()], VK_NULL_HANDLE);
        ++acquire_counter;
    }
}
