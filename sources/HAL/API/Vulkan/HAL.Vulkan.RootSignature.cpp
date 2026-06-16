module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
module HAL:API.RootSignature;

import Core;
import vulkan;
import :Types;
import :Sampler;
import :RootSignature;
import :API.Device;
import :Utils;

// Vulkan Phase 4 — real VkPipelineLayout creation.
//
// Mapping:
//   Set 0 — CBV_SRV_UAV:  global bindless layout from API::Device
//   Set 1 — Sampler:      global bindless layout from API::Device
//   Push constants:       128 bytes (ALL_STAGES) for DescriptorConstants entries

namespace HAL
{
    RootSignature::RootSignature(Device& device, const RootSignatureDesc& desc)
        : device(device)
    {
        this->desc = desc;

        auto& api_dev = static_cast<API::Device&>(device);
        VkDevice vk_dev = api_dev.get_native_device();
        if (vk_dev == VK_NULL_HANDLE) return;

        // ---- Descriptor set layouts ----------------------------------------
        // Set 0 = CBV_SRV_UAV, Set 1 = Sampler.
        // Both come from the global layouts created by Device::init().
        VkDescriptorSetLayout set_layouts[2] = {
            api_dev.get_cbv_srv_uav_layout(),
            api_dev.get_sampler_layout(),
        };

        // ---- Push constant ranges ------------------------------------------
        // Gather DescriptorConstants entries and compute the total byte range.
        // Each entry contributes count × 4 bytes at offset × 4 within the block.
        uint32_t push_bytes = 0;
        for (auto& [idx, param] : desc.parameters)
        {
            if (auto* c = std::get_if<DescriptorConstants>(&param))
            {
                uint32_t end = (c->offset + c->count) * sizeof(uint32_t);
                push_bytes = std::max(push_bytes, end);
            }
        }
        // Always allocate at least 128 bytes so common "set_constant" calls work
        // even when the root signature doesn't explicitly declare them.
        push_bytes = std::max(push_bytes, 128u);
        // Clamp to device limit (typically 128–256 bytes).
        push_bytes = std::min(push_bytes, 128u);

        VkPushConstantRange push_range{};
        push_range.stageFlags = VK_SHADER_STAGE_ALL;
        push_range.offset     = 0;
        push_range.size       = push_bytes;

        // ---- Pipeline layout -----------------------------------------------
        VkPipelineLayoutCreateInfo layout_ci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        layout_ci.setLayoutCount         = 2;
        layout_ci.pSetLayouts            = set_layouts;
        layout_ci.pushConstantRangeCount = 1;
        layout_ci.pPushConstantRanges    = &push_range;

        vkCreatePipelineLayout(vk_dev, &layout_ci, nullptr, &vk_pipeline_layout);
        vk_root_device = vk_dev;
    }
}

namespace HAL::API
{
    RootSignature::~RootSignature()
    {
        if (vk_pipeline_layout != VK_NULL_HANDLE && vk_root_device != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(vk_root_device, vk_pipeline_layout, nullptr);
    }
}
