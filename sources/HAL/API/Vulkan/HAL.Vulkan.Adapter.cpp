module HAL:Adapter;
import stl.core;
import vulkan;
import Core;

namespace HAL
{
    Adapter::Adapter(VkPhysicalDevice physical) : vk_physical(physical)
    {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(physical, &props);

        const char* name = props.deviceName;
        for (int i = 0; i < 127 && name[i]; ++i)
            info.name.push_back(static_cast<wchar_t>(name[i]));

        VkPhysicalDeviceMemoryProperties mem_props{};
        vkGetPhysicalDeviceMemoryProperties(physical, &mem_props);
        for (uint32_t i = 0; i < mem_props.memoryHeapCount; ++i)
        {
            if (mem_props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                info.dedicated_video_memory += mem_props.memoryHeaps[i].size;
        }

        info.vendor_id = props.vendorID;
        info.device_id = props.deviceID;
    }
}
