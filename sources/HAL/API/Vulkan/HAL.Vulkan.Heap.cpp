module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
module HAL:Heap;

import HAL;
import Core;

namespace HAL
{
    Heap::Heap(Device& device, const HeapDesc& desc) : desc(desc)
    {
        auto& api_dev = static_cast<API::Device&>(device);

        // For UPLOAD and READBACK heaps, allocate a single host-visible VMA
        // buffer covering the full heap size, then persistently map it.
        // The HAL::Buffer wrapper placed at offset 0 gets buffer_data directly
        // from cpu_address so update_buffer / place_data work without stalls.
        if (api_dev.vma_allocator &&
            (desc.Type == HeapType::UPLOAD || desc.Type == HeapType::READBACK))
        {
            VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bci.size  = desc.Size;
            bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                      | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                      | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

            VmaAllocationCreateInfo vma_ci{};
            vma_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                         | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            vma_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;

            VkBuffer vk_buf = VK_NULL_HANDLE;
            VmaAllocationInfo alloc_info{};
            VmaAllocation alloc = VK_NULL_HANDLE;
            vmaCreateBuffer(api_dev.vma_allocator, &bci, &vma_ci, &vk_buf, &alloc, &alloc_info);

            if (alloc_info.pMappedData)
            {
                vma_allocation = alloc;
                cpu_address    = static_cast<std::byte*>(alloc_info.pMappedData);

                // Buffer device address for GPU-side access
                VkBufferDeviceAddressInfo dai{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
                dai.buffer = vk_buf;
                gpu_address = vkGetBufferDeviceAddress(api_dev.vk_device, &dai);

                // Store the VkBuffer handle via the allocation so the destructor can free it.
                // We abuse vk_memory to stash the VkBuffer (both are opaque handles).
                vk_memory = alloc_info.deviceMemory;
                // Keep the VkBuffer alive for the heap lifetime; store in a side channel.
                heap_vk_buffer = vk_buf;
            }
        }

        // Create the HAL::Buffer wrapper at offset 0 of this heap.
        // Resource::init(PlacementAddress) will read cpu_address + gpu_address
        // from this Heap object and store them as mapped_data / address on the
        // resource.  Buffer::init() then sets buffer_data = get_cpu_mapping().
        buffer.reset(new HAL::Buffer(device, ResourceDesc::Buffer(desc.Size), PlacementAddress{ this, 0 }));

        if (desc.Type == HeapType::UPLOAD)
            buffer->set_name("Upload Heap Buffer");
        else if (desc.Type == HeapType::READBACK)
            buffer->set_name("Readback Heap Buffer");
        else
            buffer->set_name("GPU Heap Buffer");
    }

    std::span<std::byte> Heap::get_data()
    {
        return std::span(cpu_address, cpu_address ? desc.Size : 0);
    }

    namespace API
    {
        Heap::~Heap()
        {
            // The VkBuffer and VmaAllocation are both freed by vmaDestroyBuffer.
            // We stored the VkBuffer in HAL::Heap::heap_vk_buffer and the
            // allocation in vma_allocation.
            if (vma_allocation)
            {
                // Retrieve vma_allocator from the owning device — but at dtor
                // time we don't have a device reference.  Walk the buffer instead.
                // (Phase 1 TODO: store device ref in API::Heap for clean teardown.)
                // For now the process exits cleanly and the driver reclaims memory.
            }
        }

        GPUAddressPtr Heap::get_address() const { return gpu_address; }
    }
}
