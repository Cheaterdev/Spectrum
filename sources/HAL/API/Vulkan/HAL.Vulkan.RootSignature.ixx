export module HAL:API.RootSignature;

import vulkan;
export import :Utils;  // Re-exported — same reason as DescriptorHeap.

export namespace HAL
{
    namespace API
    {
        class RootSignature
        {
        protected:
            // Vulkan: root signature maps to VkPipelineLayout + descriptor set layouts.
            VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;
            VkDevice         vk_root_device     = VK_NULL_HANDLE; // for destructor

        public:
            virtual ~RootSignature();

            VkPipelineLayout get_vk_pipeline_layout() const noexcept { return vk_pipeline_layout; }
        };
    }
}
