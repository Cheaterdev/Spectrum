module HAL:Heap;

import HAL;
import vulkan;
import Core;
#undef THIS

// Vulkan implementation of HAL::Heap + HAL::API::Heap.
// Mirrors D3D12/HAL.D3D12.Heap.cpp.  A "heap" maps to a single large VMA/device
// allocation that sub-resources are placed into.
// Phase 0: minimal — allocates no real device memory yet (Phase 1 wires VMA).

namespace HAL
{
    Heap::Heap(Device& device, const HeapDesc& desc) : desc(desc)
    {
        // Phase 1: vmaAllocateMemory / vkAllocateMemory of desc.Size, then create
        // a placed buffer covering it (for UPLOAD/READBACK map cpu_address).
        // For now we create the backing Buffer wrapper like the D3D12 path so
        // as_buffer()/get_data() are valid.
        buffer.reset(new HAL::Buffer(device, ResourceDesc::Buffer(desc.Size), PlacementAddress{ this, 0 }));

        if (desc.Type == HeapType::UPLOAD)
            buffer->set_name("Upload Heap Buffer");
        else if (desc.Type == HeapType::READBACK)
            buffer->set_name("Readback Heap Buffer");
        else
            buffer->set_name("GPU Heap Buffer");
    }

    // NOTE: get_type(), get_size(), as_buffer() are NOT defined here — they are
    // backend-agnostic and defined in a common TU (mirrors D3D12.Heap.cpp which
    // also omits them).  Defining them here would duplicate-link in Vulkan.

    std::span<std::byte> Heap::get_data()
    {
        return std::span(cpu_address, cpu_address ? desc.Size : 0);
    }

    namespace API
    {
        Heap::~Heap()
        {
            // Phase 1: vmaFreeMemory / vkFreeMemory.
        }

        GPUAddressPtr Heap::get_address() const { return gpu_address; }
    }
}
