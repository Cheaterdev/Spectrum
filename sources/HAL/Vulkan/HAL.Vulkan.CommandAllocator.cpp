module HAL:API.CommandAllocator;

import Core;
import HAL;
import vulkan;

// Vulkan implementation of HAL::CommandAllocator.
// Mirrors D3D12/HAL.D3D12.CommandAllocator.cpp.  In Vulkan a "command
// allocator" maps to a VkCommandPool (one per command-list type / frame).

namespace HAL
{
    CommandAllocator::CommandAllocator(Device& device, const CommandListType type)
        : device(device), type(type)
    {
        // Phase 1: vkCreateCommandPool with the queue family matching `type`,
        // using VK_COMMAND_POOL_CREATE_TRANSIENT_BIT for per-frame reset.
    }

    void CommandAllocator::reset()
    {
        // Phase 1: vkResetCommandPool(device, vk_command_pool, 0).
    }
}
