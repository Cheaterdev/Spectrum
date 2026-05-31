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
            VkPipelineLayout       vk_pipeline_layout = VK_NULL_HANDLE;
        public:
            virtual ~RootSignature() = default;
        };
    }
}
