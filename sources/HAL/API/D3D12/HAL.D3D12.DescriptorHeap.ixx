export module HAL:API.DescriptorHeap;

import :API.Device;
import :Utils;
import :Types;
import :Descriptors;
import :API.Resource;

import Core;

export namespace HAL
{
    namespace API
    {
        class DescriptorHeap;

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
    class Descriptor;
    namespace API {

        class DescriptorHeap
        {
            friend class Descriptor;
        public:
            D3D::DescriptorHeap m_cpu_heap;
            D3D::DescriptorHeap m_gpu_heap;

            const DescriptorHeapDesc desc;
            const Device& device;

            CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_start;
            CD3DX12_CPU_DESCRIPTOR_HANDLE gpu_cpu_start;
            CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_start;

            uint handle_size;

            DescriptorHeap(Device& device, const DescriptorHeapDesc& desc);

            HAL::Descriptor operator[](uint i);

            auto get_dx() const
            {
                if (m_gpu_heap)
                    return m_gpu_heap.Get();
                return m_cpu_heap.Get();
            }
        };
    }
}