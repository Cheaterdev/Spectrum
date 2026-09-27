export module HAL:API.RootSignature;

import vulkan;
export import :Utils;  // Re-exported — same reason as DescriptorHeap.

export namespace HAL
{
    namespace API
    {
        // A root signature owns no Vulkan objects: the SIG system uses
        // DefaultLayout everywhere, so every pipeline shares the one
        // VkPipelineLayout the Device creates (bindless sets + push constants
        // for the root constants).  Kept only for HAL-layer type compatibility.
        class RootSignature
        {
        public:
            virtual ~RootSignature() = default;
        };
    }
}
