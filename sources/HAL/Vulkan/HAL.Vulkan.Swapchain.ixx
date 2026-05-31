export module HAL:API.SwapChain;
import vulkan;
import Core;
import :Types;

export
{
    namespace HAL
    {
        namespace API
        {
            class SwapChain
            {
            protected:
                VkSurfaceKHR   vk_surface   = VK_NULL_HANDLE;
                VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
                VkFormat       vk_format    = VK_FORMAT_B8G8R8A8_UNORM;
                uint32_t       image_count  = 0;
                uint32_t       current_image = 0;

                std::vector<VkImage>     swapchain_images;
                std::vector<VkImageView> swapchain_views;

                // Sync objects
                std::vector<VkSemaphore> image_available;  // one per frame-in-flight
                std::vector<VkSemaphore> render_finished;

            public:
                virtual ~SwapChain() = default;
            };
        }
    }
}
