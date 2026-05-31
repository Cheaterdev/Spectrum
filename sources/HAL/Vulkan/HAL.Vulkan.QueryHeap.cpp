module HAL:QueryHeap;

import vulkan;
import Core;
import :API.QueryHeap;
import :API.Device;
#undef THIS

// Vulkan implementation of HAL::QueryHeap (timestamp queries).
// Mirrors D3D12/HAL.D3D12.QueryHeap.cpp.  API::QueryHeap::get_native() is
// inline in HAL.Vulkan.QueryHeap.ixx.

namespace HAL
{
    QueryHeap::QueryHeap(Device& device, const QueryHeapDesc& desc) : desc(desc)
    {
        // Phase 1: vkCreateQueryPool(VK_QUERY_TYPE_TIMESTAMP, desc.Count).
        read_back_data.resize(desc.Count);
    }
}
