module HAL:Adapter;
import stl.core;
import vulkan;
import Core;

namespace HAL
{
    Adapter::Adapter(VkPhysicalDevice physical) : vk_physical(physical)
    {
        // Fill the DXGI_ADAPTER_DESC stub from VkPhysicalDeviceProperties
        // so HAL::Device::create_singleton() can log the adapter name and do
        // "Basic" string detection.
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(physical, &props);

        // Convert UTF-8 deviceName to wchar Description
        const char* name = props.deviceName;
        for (int i = 0; i < 127 && name[i]; ++i)
            adapter_desc.Description[i] = static_cast<wchar_t>(name[i]);

        VkPhysicalDeviceMemoryProperties mem_props{};
        vkGetPhysicalDeviceMemoryProperties(physical, &mem_props);
        for (uint32_t i = 0; i < mem_props.memoryHeapCount; ++i)
        {
            if (mem_props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                adapter_desc.DedicatedVideoMemory += mem_props.memoryHeaps[i].size;
        }

        adapter_desc.VendorId = props.vendorID;
        adapter_desc.DeviceId = props.deviceID;
    }

    const DXGI_ADAPTER_DESC& Adapter::get_desc() const
    {
        return adapter_desc;
    }
}
