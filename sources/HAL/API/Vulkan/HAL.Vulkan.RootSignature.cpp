module HAL:API.RootSignature;

import Core;
import :RootSignature;
import :API.Device;

// A root signature owns no Vulkan objects -- every pipeline uses the Device's
// global VkPipelineLayout (see API::RootSignature).  The HAL-layer RootSignature
// just retains its RootSignatureDesc.

namespace HAL
{
    RootSignature::RootSignature(Device& device, const RootSignatureDesc& desc)
        : device(device)
    {
        this->desc = desc;
    }
}
