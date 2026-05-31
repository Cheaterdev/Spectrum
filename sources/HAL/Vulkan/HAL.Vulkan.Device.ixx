export module HAL:API.Device;

import stl.core;
import vulkan;
import Core;

import :Types;
import :Sampler;
import :Utils;
import :Adapter;

using namespace HAL;

export namespace HAL
{
    struct DeviceDesc
    {
        HAL::Adapter::ptr adapter;
    };

    struct DeviceProperties
    {
        std::string name;
        bool rtx              = false;
        bool mesh_shader      = false;
        bool full_bindless    = false;
        bool direct_gpu_upload_heap = false;
        bool work_graph       = false;
    };

    namespace API
    {
        class Device
        {
            std::map<ResourceDesc, ResourceAllocationInfo> alloc_info;
        protected:
            void init(DeviceDesc& desc);
            virtual ~Device();

        public:
            using ptr = std::shared_ptr<Device>;

            // Vulkan objects
            VkInstance       vk_instance  = VK_NULL_HANDLE;
            VkPhysicalDevice vk_physical  = VK_NULL_HANDLE;
            VkDevice         vk_device    = VK_NULL_HANDLE;
            VmaAllocator     vma_allocator = VK_NULL_HANDLE;

            // Debug messenger (debug builds)
            VkDebugUtilsMessengerEXT vk_debug_messenger = VK_NULL_HANDLE;

            // Descriptor sizes — kept for interface compat; unused in Vulkan
            enum_array<DescriptorHeapType, uint> descriptor_sizes;

            void process_result(VkResult result, std::string_view line) const;

            uint get_descriptor_size(DescriptorHeapType type) const;
            VkDevice get_native_device() const;

            VkResult get_device_removed_reason() const;

            ResourceAllocationInfo get_alloc_info(const ResourceDesc& desc);
            uint Subresources(const ResourceDesc& desc) const;

            size_t get_vram();

            RaytracingPrebuildInfo calculateBuffers(const RaytracingBuildDescBottomInputs& desc);
            RaytracingPrebuildInfo calculateBuffers(const RaytracingBuildDescTopInputs& desc);
        };
    }
}
