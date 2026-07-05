export module HAL:API.RootSignature;

import vulkan;
export import :Utils;  // Re-exported — same reason as DescriptorHeap.

export namespace HAL
{
    namespace API
    {
        // With VK_EXT_descriptor_heap a root signature owns NO Vulkan objects:
        // there is no VkPipelineLayout and no VkDescriptorSetLayout.  Binding
        // mappings live on the Device and are chained into each pipeline's stages;
        // root constants / root descriptors are pushed via vkCmdPushDataEXT.
        // This class is kept only for HAL-layer type compatibility.
        class RootSignature
        {
        public:
            virtual ~RootSignature() = default;
        };
    }
}
