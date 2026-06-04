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

                // Sync objects.
                // image_available is a FREE-RUNNING RING indexed by acquire_counter,
                // NOT by image index.  vkAcquireNextImageKHR can return the same image
                // index on consecutive frames (always true under MAILBOX, possible under
                // any mode), so tying the acquire semaphore to the image index would reuse
                // a semaphore whose previous wait has not yet completed → VUID-01779
                // "Semaphore must not have any pending operations" → GPU hang → device lost.
                // Ring size = image_count + 1; per-image fence pacing (wait_for_free) bounds
                // frames-in-flight to <= image_count, so the slot acquire_counter reuses is
                // always fully drained.  render_finished stays per-image (waited by present).
                std::vector<VkSemaphore> image_available;  // ring of (image_count + 1)
                std::vector<VkSemaphore> render_finished;  // one per swapchain image
                uint32_t                 acquire_counter = 0;

                // Set during acquire_next_frame(); consumed by present().
                VkSemaphore current_image_available = VK_NULL_HANDLE;

                // Present-time transition infrastructure.
                // create_transition_list() in the common HAL is stubbed out, so the
                // RENDER_TARGET→PRESENT layout transition never fires from the frame graph.
                // We inject it here: one pre-recorded command buffer per swapchain image
                // (COLOR_ATTACHMENT_OPTIMAL → PRESENT_SRC_KHR) plus one "image ready for
                // present" semaphore per image.  The chain in present() is:
                //   execute() → signals render_finished[I]
                //   transition_submit waits render_finished[I], signals present_sem[I]
                //   vkQueuePresentKHR waits present_sem[I]
                VkCommandPool                present_pool = VK_NULL_HANDLE;
                std::vector<VkCommandBuffer> present_cbs;   // COLOR_ATTACHMENT→PRESENT_SRC_KHR
                std::vector<VkSemaphore>     present_sems;  // signalled when transition done

                // Build / destroy the present-transition command buffers and semaphores.
                void setup_present_commands(VkDevice vk_dev, uint32_t queue_family);
                void teardown_present_commands(VkDevice vk_dev);

            public:
                virtual ~SwapChain() = default;
            };
        }
    }
}
