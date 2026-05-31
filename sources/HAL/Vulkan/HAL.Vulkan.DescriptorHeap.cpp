module HAL:DescriptorHeap;

import :Debug;
import :Resource;
import :Resource.Buffer;

import vulkan;
import Core;
#undef THIS

// Vulkan native implementation of the descriptor pieces that the D3D12 build
// puts in D3D12/HAL.D3D12.DescriptorHeap.cpp (also `module HAL:DescriptorHeap`).
// The backend-agnostic Handle / DescriptorHeapStorage / DescriptorHeapFactory
// code lives in the common HAL.DescriptorHeap.cpp and is shared.
//
// Phase 4 implements real VkDescriptorPool / VkDescriptorSet writes; for now
// place() records nothing and the get_cpu/get_gpu stubs return zero handles.

namespace HAL
{
    Descriptor::Descriptor(DescriptorHeap& heap, uint offset) : heap(heap), offset(offset) {}

    void Descriptor::place(const Views::ShaderResource&)  {}
    void Descriptor::place(const Views::UnorderedAccess&) {}
    void Descriptor::place(const Views::RenderTarget&)    {}
    void Descriptor::place(const Views::DepthStencil&)    {}
    void Descriptor::place(const Views::ConstantBuffer&)  {}

    void Descriptor::operator=(const Descriptor&) {}

    D3D12_CPU_DESCRIPTOR_HANDLE Descriptor::get_cpu() { return {}; }
    D3D12_GPU_DESCRIPTOR_HANDLE Descriptor::get_gpu() { return {}; }

    uint DescriptorHeap::get_size() { return desc.Count; }

    namespace API
    {
        DescriptorHeap::DescriptorHeap(Device& device, const DescriptorHeapDesc& desc)
            : device(device), desc(desc)
        {
            // Phase 4: create VkDescriptorPool / VkDescriptorSetLayout sized to
            // desc.Count for desc.HeapType.
            handle_size = 0;
        }

        Descriptor DescriptorHeap::operator[](uint i)
        {
            auto THIS = static_cast<HAL::DescriptorHeap*>(this);
            return Descriptor{ *THIS, i };
        }
    }
}
