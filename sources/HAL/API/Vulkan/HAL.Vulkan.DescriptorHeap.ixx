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
    }

    struct DescriptorHeapDesc
    {
        uint Count;
        DescriptorHeapType HeapType;
        DescriptorHeapFlags Flags;
    };

    class DescriptorHeap;

    // Descriptor — Vulkan descriptor set entry.
    // Keeps the same interface as the D3D12 version so HAL.DescriptorHeap.ixx
    // compiles unchanged.  get_cpu() / get_gpu() return stub handles; real
    // Vulkan descriptor management will be added in Phase 4.
    class Descriptor
    {
        DescriptorHeap& heap;
        const uint offset;

        Descriptor(DescriptorHeap& heap, uint offset);

        friend class API::DescriptorHeap;
    public:
        void operator=(const Descriptor& r);

        void place(const Views::ShaderResource& view);
        void place(const Views::UnorderedAccess& view);
        void place(const Views::RenderTarget& view);
        void place(const Views::ConstantBuffer& view);
        void place(const Views::DepthStencil& view);

        // Return stub handles (no D3D12 dependency in Vulkan builds)
        D3D12_CPU_DESCRIPTOR_HANDLE get_cpu();
        D3D12_GPU_DESCRIPTOR_HANDLE get_gpu();
    };

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

            friend class Descriptor;
        public:
            DescriptorHeap(Device& device, const DescriptorHeapDesc& desc);
            virtual ~DescriptorHeap();

            Descriptor operator[](uint i);

            // Returns the backing VkDescriptorSet so CommandList can bind it.
            VkDescriptorSet get_vk_set() const noexcept { return vk_set; }
        };
    }
}
