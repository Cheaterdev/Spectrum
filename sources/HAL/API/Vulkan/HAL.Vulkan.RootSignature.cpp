module HAL:API.RootSignature;

import Core;
import :RootSignature;
import :API.Device;

// VK_EXT_descriptor_heap: a root signature owns no Vulkan objects.  There is no
// VkPipelineLayout and no VkDescriptorSetLayout — binding mappings live on the
// Device and are chained into each pipeline's stages at creation, while root
// constants / root descriptors are pushed via vkCmdPushDataEXT.  The HAL-layer
// RootSignature just retains its RootSignatureDesc.

namespace HAL
{
    RootSignature::RootSignature(Device& device, const RootSignatureDesc& desc)
        : device(device)
    {
        this->desc = desc;
    }
}
