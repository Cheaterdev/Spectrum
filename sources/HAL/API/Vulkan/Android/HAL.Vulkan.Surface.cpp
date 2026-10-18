// The Android half of Vulkan swapchain creation: the window-system surface. A plain (non-module)
// translation unit, like the Windows one in ../Windows; the module side (HAL.Vulkan.Swapchain.cpp)
// sees only the declarations.
#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>

// The "HWND" the window hands out is its ANativeWindow*.
VkSurfaceKHR hal_create_window_surface(VkInstance instance, void* native_window)
{
    VkAndroidSurfaceCreateInfoKHR ci{ VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR };
    ci.window = static_cast<ANativeWindow*>(native_window);

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    vkCreateAndroidSurfaceKHR(instance, &ci, nullptr, &surface);
    return surface;
}

// Android surfaces always report a definite currentExtent, so this is never reached; the smallest
// extent keeps the swapchain on its zero-size path if a driver ever says otherwise.
VkExtent2D hal_query_window_extent(void*, VkExtent2D min_extent, VkExtent2D)
{
    return min_extent;
}
