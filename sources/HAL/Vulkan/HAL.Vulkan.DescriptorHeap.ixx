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
        public:
            const DescriptorHeapDesc desc;
            const Device& device;

            uint handle_size = 0;

            friend class Descriptor;
        public:
            DescriptorHeap(Device& device, const DescriptorHeapDesc& desc);

            Descriptor operator[](uint i);

            // No get_dx() in Vulkan — backend-specific command list code must
            // not call this outside D3D12/ folder files.
        };
    }
}
