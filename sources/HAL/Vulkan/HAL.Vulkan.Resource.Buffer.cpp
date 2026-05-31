module HAL:Resource.Buffer;

import vulkan;
import Core;

import :FrameManager;
#undef THIS

// Vulkan implementation of the backend-specific Buffer methods.
// Mirrors D3D12/HAL.D3D12.Resource.Buffer.cpp (excluded from Vulkan build).
// The backend-agnostic Buffer methods (init, read, write, constructors,
// get_size, get_resource_address) live in the common HAL.Resource.Buffer.cpp.

namespace HAL
{
    std::span<std::byte> Buffer::cpu_data() const
    {
        return { buffer_data, buffer_data + get_size() };
    }

    Buffer::~Buffer()
    {
        // Phase 1: VMA-mapped memory is unmapped by vmaDestroyBuffer; if we
        // hold a persistent mapping we release it here.
        buffer_data = nullptr;
    }

    std::byte* ResourceAddress::get_cpu_data() const
    {
        return resource->cpu_data().data() + resource_offset;
    }
}

// Global helper mirroring the D3D12 ::to_native(ResourceAddress) — converts a
// HAL::ResourceAddress to a raw GPU virtual address (buffer device address).
HAL::GPUAddressPtr to_native(const HAL::ResourceAddress& address)
{
    return address.resource ? (address.resource->get_address() + address.resource_offset) : 0;
}
