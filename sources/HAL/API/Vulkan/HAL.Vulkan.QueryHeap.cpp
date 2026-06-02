module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
module HAL:QueryHeap;

import vulkan;
import Core;
import :API.QueryHeap;
import :API.Device;

namespace HAL
{
    // ---- API::QueryHeap destructor ------------------------------------------

    API::QueryHeap::~QueryHeap()
    {
        if (vk_query_pool != VK_NULL_HANDLE && vk_qh_device != VK_NULL_HANDLE)
        {
            vkDestroyQueryPool(vk_qh_device, vk_query_pool, nullptr);
            vk_query_pool = VK_NULL_HANDLE;
        }
    }

    // ---- HAL::QueryHeap constructor -----------------------------------------

    QueryHeap::QueryHeap(Device& device, const QueryHeapDesc& desc) : desc(desc)
    {
        auto& api_dev = static_cast<API::Device&>(device);
        VkDevice vk_dev = api_dev.get_native_device();
        vk_qh_device = vk_dev;

        if (vk_dev == VK_NULL_HANDLE) { read_back_data.resize(desc.Count); return; }

        // Map HAL QueryType → Vulkan query type.
        VkQueryType vk_type = VK_QUERY_TYPE_TIMESTAMP;
        switch (desc.type)
        {
        case QueryType::Timestamp:  vk_type = VK_QUERY_TYPE_TIMESTAMP;           break;
        case QueryType::Statistics: vk_type = VK_QUERY_TYPE_PIPELINE_STATISTICS; break;
        default: break;
        }

        VkQueryPoolCreateInfo ci{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
        ci.queryType  = vk_type;
        ci.queryCount = desc.Count;
        vkCreateQueryPool(vk_dev, &ci, nullptr, &vk_query_pool);

        read_back_data.resize(desc.Count);
    }
}
