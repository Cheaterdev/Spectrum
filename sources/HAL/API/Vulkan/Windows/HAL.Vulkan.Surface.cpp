// The Windows half of Vulkan swapchain creation: the window-system surface. A plain (non-module)
// translation unit so <windows.h> and the Win32 Vulkan header stay out of the HAL module; the module
// side (HAL.Vulkan.Swapchain.cpp) sees only the declarations. Android has its own copy in ../Android.
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstdint>

VkSurfaceKHR hal_create_window_surface(VkInstance instance, void* native_window)
{
    VkWin32SurfaceCreateInfoKHR ci{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    ci.hinstance = GetModuleHandle(nullptr);
    ci.hwnd      = static_cast<HWND>(native_window);

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    vkCreateWin32SurfaceKHR(instance, &ci, nullptr, &surface);
    return surface;
}

// Used when the surface leaves the size to the application (currentExtent == UINT32_MAX).
VkExtent2D hal_query_window_extent(void* native_window, VkExtent2D min_extent, VkExtent2D max_extent)
{
    RECT r{};
    GetClientRect(static_cast<HWND>(native_window), &r);
    return {
        std::clamp(static_cast<uint32_t>(std::max(0L, r.right  - r.left)), min_extent.width,  max_extent.width),
        std::clamp(static_cast<uint32_t>(std::max(0L, r.bottom - r.top)),  min_extent.height, max_extent.height)
    };
}
