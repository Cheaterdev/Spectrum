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
        // Always use whatever the driver reports as supported — never assume.
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
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT;
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

        // ---- Per-frame sync semaphores --------------------------------------
        // One pair per image: acquire semaphore + render-done semaphore.
        image_available.resize(image_count);
        render_finished.resize(image_count);
        for (uint32_t i = 0; i < image_count; ++i)
        {
            image_available[i] = make_semaphore(api_dev.vk_device);
            render_finished[i] = make_semaphore(api_dev.vk_device);
        }

        // ---- Wrap backbuffers as TextureResources ---------------------------
        frames.resize(image_count);
        m_frameIndex = 0;
        on_change();

        Log::get() << "Vulkan swapchain: " << image_count << " images, "
                   << extent.width << "x" << extent.height << Log::endl;

        // Pre-acquire the first image so m_frameIndex is valid before the
        // first wait_for_free() + get_current_frame() calls.
        do_acquire(api_dev.vk_device, vk_swapchain,
                   image_available[0], current_image, m_frameIndex);

        // Wire acquire/present semaphores for the first frame's execute().
        auto& api_queue = static_cast<API::Queue&>(*device.get_queue(CommandListType::DIRECT));
        api_queue.set_frame_semaphores(image_available[0], render_finished[0]);
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

    void SwapChain::present()
    {
        auto& api_dev   = static_cast<API::Device&>(device);
        auto  gfx_queue = device.get_queue(CommandListType::DIRECT);
        auto& api_queue = static_cast<API::Queue&>(*gfx_queue);

        // Present waits on render_finished semaphore signaled by this frame's execute().
        VkSemaphore wait_sem = render_finished[current_image];
        VkPresentInfoKHR pi{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        pi.swapchainCount     = 1;
        pi.pSwapchains        = &vk_swapchain;
        pi.pImageIndices      = &current_image;
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores    = &wait_sem;

        VkResult pr = vkQueuePresentKHR(gfx_queue->get_native(), &pi);

        // CPU fence so wait_for_free() can pace the next frame.
        frames[m_frameIndex].fence_event = gfx_queue->signal();

        // Acquire the NEXT image now so that m_frameIndex is already correct
        // when the caller calls wait_for_free() + get_current_frame() on the
        // next iteration — identical to D3D12's GetCurrentBackBufferIndex().
        uint32_t next_image = current_image;
        uint32_t next_idx   = m_frameIndex;
        uint32_t next_sem   = (m_frameIndex + 1) % image_count;
        if (pr != VK_ERROR_OUT_OF_DATE_KHR &&
            do_acquire(api_dev.vk_device, vk_swapchain,
                       image_available[next_sem], next_image, next_idx))
        {
            current_image = next_image;
            m_frameIndex  = next_idx;
            // Wire semaphores for the next frame's command buffer submission.
            api_queue.set_frame_semaphores(image_available[next_sem],
                                           render_finished[next_sem]);
        }
        else
        {
            // Out-of-date — caller's next resize() will rebuild the swapchain.
            m_frameIndex = (m_frameIndex + 1) % image_count;
        }
    }

    void SwapChain::resize(ivec2 size)
    {
        if (size.x < 64) size.x = 64;
        if (size.y < 64) size.y = 64;

        auto& api_dev = static_cast<API::Device&>(device);
        if (api_dev.vk_device == VK_NULL_HANDLE) return;

        // Wait for all queues idle before destroying swapchain resources.
        device.get_queue(CommandListType::DIRECT)->signal_and_wait();
        device.get_queue(CommandListType::COMPUTE)->signal_and_wait();
        device.get_queue(CommandListType::COPY)->signal_and_wait();

        // Destroy old views and semaphores.
        for (uint32_t i = 0; i < image_count; ++i)
        {
            frames[i].m_renderTarget = nullptr;
            vkDestroyImageView(api_dev.vk_device, swapchain_views[i], nullptr);
            vkDestroySemaphore(api_dev.vk_device, image_available[i], nullptr);
            vkDestroySemaphore(api_dev.vk_device, render_finished[i],  nullptr);
        }
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
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT;
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
        image_available.resize(image_count);
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

            image_available[i] = make_semaphore(api_dev.vk_device);
            render_finished[i] = make_semaphore(api_dev.vk_device);
        }

        frames.resize(image_count);
        m_frameIndex = 0;
        on_change();
    }
}
