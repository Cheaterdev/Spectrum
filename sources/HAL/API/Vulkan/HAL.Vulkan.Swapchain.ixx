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
                VkSwapchainKHR  vk_swapchain   = VK_NULL_HANDLE;
                VkFormat        vk_format      = VK_FORMAT_R8G8B8A8_UNORM;
                VkColorSpaceKHR vk_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                VkExtent2D      sc_extent      = {};
                uint32_t       image_count   = 0;
                uint32_t       current_image = 0;  // index into swapchain_images[], set by vkAcquireNextImageKHR

                std::vector<VkImage>     swapchain_images;
                std::vector<VkImageView> swapchain_views;

                // Sync objects
                std::vector<VkSemaphore> image_available;  // one per frame-in-flight
                std::vector<VkSemaphore> render_finished;

                // Set during acquire_next_frame(); consumed by present().
                VkSemaphore current_image_available = VK_NULL_HANDLE;

            public:
                virtual ~SwapChain() = default;
            };
        }
    }
}
