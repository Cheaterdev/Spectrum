module HAL:SwapChain;

import Core;
import HAL;
import vulkan;

// Vulkan native implementation of the swapchain methods the D3D12 build puts in
// DXGI/HAL.DXGI.Swapchain.cpp: the constructor, present(), on_change(), resize().
// The backend-agnostic methods (get_fence, wait_for_free, get_current_frame,
// get_prev_frame) live in the common HAL.Swapchain.cpp and are shared.
//
// Phase 2 implements VkSurfaceKHR (Win32) + VkSwapchainKHR + backbuffer image
// import via API::NativeImportHandle{ image, view, format }.

namespace HAL
{
    SwapChain::SwapChain(Device& device, swap_chain_desc /*c_desc*/) : device(device)
    {
        // Phase 2:
        //  - vkCreateWin32SurfaceKHR from c_desc.window->get_hwnd()
        //  - choose format (prefer VK_FORMAT_B8G8R8A8_UNORM) / present mode
        //  - vkCreateSwapchainKHR (triple-buffered)
        //  - vkGetSwapchainImagesKHR, create image views
        //  - wrap each backbuffer in a TextureResource via NativeImportHandle
        //  - create per-frame acquire/submit semaphores
        frames.resize(2);
        m_frameIndex = 0;
    }

    void SwapChain::present()
    {
        // Phase 2/3: vkQueuePresentKHR + advance frame index, signal fence.
    }

    void SwapChain::on_change()
    {
        // Phase 2: (re)wrap backbuffer images into frames[].m_renderTarget.
    }

    void SwapChain::resize(ivec2 /*size*/)
    {
        // Phase 2: wait idle, destroy + recreate swapchain at new size, on_change().
    }
}
