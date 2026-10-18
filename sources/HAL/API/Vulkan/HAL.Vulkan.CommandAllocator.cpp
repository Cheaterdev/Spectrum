module HAL:API.CommandAllocator;

import vulkan;
import Core;
import HAL;

namespace HAL
{
    CommandAllocator::CommandAllocator(Device& device, const CommandListType type)
        : device(device), type(type)
    {
        auto& api_dev = static_cast<API::Device&>(device);
        if (api_dev.vk_device == VK_NULL_HANDLE) return;

        uint32_t family = api_dev.get_queue_family(static_cast<int>(type));
        if (family == static_cast<uint32_t>(-1)) return;

        VkCommandPoolCreateInfo info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        // RESET_COMMAND_BUFFER_BIT lets individual buffers be reset without
        // resetting the whole pool — needed since DelayedCommandList recycles
        // command lists individually.
        info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = family;

        vkCreateCommandPool(api_dev.vk_device, &info, nullptr, &vk_command_pool);
    }

    CommandAllocator::~CommandAllocator()
    {
        if (vk_command_pool == VK_NULL_HANDLE) return;
        auto& api_dev = static_cast<API::Device&>(device);
        if (api_dev.vk_device == VK_NULL_HANDLE) return;
        vkDestroyCommandPool(api_dev.vk_device, vk_command_pool, nullptr);
        vk_command_pool = VK_NULL_HANDLE;
    }

    void CommandAllocator::reset()
    {
        if (vk_command_pool == VK_NULL_HANDLE) return;
        auto& api_dev = static_cast<API::Device&>(device);
        std::lock_guard g(pool_mutex);
        vkResetCommandPool(api_dev.vk_device, vk_command_pool, 0);
        next_buffer = 0;
    }

    namespace API
    {
        VkCommandBuffer CommandAllocator::acquire(VkDevice vk_dev)
        {
            if (next_buffer == buffers.size())
            {
                VkCommandBufferAllocateInfo alloc{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
                alloc.commandPool        = vk_command_pool;
                alloc.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                alloc.commandBufferCount = 1;
                VkCommandBuffer cb = VK_NULL_HANDLE;
                vkAllocateCommandBuffers(vk_dev, &alloc, &cb);
                buffers.push_back(cb);
            }
            return buffers[next_buffer++];
        }
    }
}
