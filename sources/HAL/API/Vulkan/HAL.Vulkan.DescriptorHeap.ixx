export module HAL:API.DescriptorHeap;

import :API.Device;
export import :Utils;  // Re-exported so D3D12-compat stubs are visible to HAL.DescriptorHeap.ixx
import :Types;
import :Descriptors;
import :API.Resource;

import Core;

export namespace HAL
{
    namespace API
    {
        class DescriptorHeap;

        // Vulkan equivalent of D3D12's API::Descriptor base.  The shared
        // HAL::Descriptor (HAL.DescriptorHeap.ixx) derives from this and the
        // public HAL::Handle interface still exposes D3D12-style handles
        // (stubbed in Vulkan builds); for Vulkan the handle value carries the
        // bindless slot index.
        class Descriptor
        {
        protected:
            D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = {};
            D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = {};
        public:
            D3D12_CPU_DESCRIPTOR_HANDLE get_cpu() const { return cpu_handle; }
            D3D12_GPU_DESCRIPTOR_HANDLE get_gpu() const { return gpu_handle; }
        };
    }

    struct DescriptorHeapDesc
    {
        uint Count;
        DescriptorHeapType HeapType;
        DescriptorHeapFlags Flags;
    };

    class DescriptorHeap;
    class Descriptor;   // shared HAL::Descriptor (HAL.DescriptorHeap.ixx)

    namespace API
    {
        class DescriptorHeap
        {
        protected:
            // Vulkan descriptor objects — only valid for CBV_SRV_UAV / SAMPLER heaps.
            VkDescriptorPool    vk_pool       = VK_NULL_HANDLE;
            VkDescriptorSet     vk_set        = VK_NULL_HANDLE;

        public:
            const DescriptorHeapDesc desc;
            Device& device;   // non-const: place() calls vkUpdateDescriptorSets

            uint handle_size = 0;

            friend class HAL::Descriptor;
        public:
            DescriptorHeap(Device& device, const DescriptorHeapDesc& desc);
            virtual ~DescriptorHeap();

            HAL::Descriptor operator[](uint i);

            // Returns the backing VkDescriptorSet so CommandList can bind it.
            VkDescriptorSet get_vk_set() const noexcept { return vk_set; }
        };
    }
}
