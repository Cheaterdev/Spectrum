module HAL:API.RootSignature;

import Core;
import vulkan;
import :Types;
import :Sampler;
import :RootSignature;
import :API.Device;
import :Utils;

// Vulkan implementation of HAL::RootSignature.
// Mirrors D3D12/HAL.D3D12.RootSignature.cpp.  A "root signature" maps to a
// VkPipelineLayout plus its VkDescriptorSetLayouts.
// Phase 4 builds the real descriptor-set layouts from RootSignatureDesc;
// Phase 0 records the desc and creates no Vulkan objects yet.

namespace HAL
{
    RootSignature::RootSignature(Device& device, const RootSignatureDesc& desc) : device(device)
    {
        this->desc = desc;
        // Phase 4: translate desc.parameters / desc.samplers() into
        // VkDescriptorSetLayoutBinding[] + push constants, then
        // vkCreatePipelineLayout → vk_pipeline_layout.
    }
}
