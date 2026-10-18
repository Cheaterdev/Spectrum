module HAL:Heap;

import vulkan;
import HAL;
import Core;

namespace HAL
{
    Heap::Heap(Device& device, const HeapDesc& desc) : desc(desc)
    {
        auto& api_dev = static_cast<API::Device&>(device);
        sparse_device = &api_dev;
        sparse_size   = desc.Size;

        // For UPLOAD and READBACK heaps, allocate a single host-visible VMA
        // buffer covering the full heap size, then persistently map it.
        // The HAL::Buffer wrapper placed at offset 0 gets buffer_data directly
        // from cpu_address so update_buffer / place_data work without stalls.
        if (api_dev.vma_allocator &&
            (desc.Type == HeapType::UPLOAD || desc.Type == HeapType::READBACK))
        {
            VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bci.size  = desc.Size;
            // Placed sub-allocations (CBVs, vertex/index buffers, indirect args,
            // texture staging) all share this single heap buffer, so it must
            // declare every usage those slices can take on — matching the
            // committed-buffer path in HAL.Vulkan.Resource.cpp.
            bci.usage = api_dev.buffer_usage;

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
                vma_allocation    = alloc;
                vma_allocator_ref = api_dev.vma_allocator;
                cpu_address       = static_cast<std::byte*>(alloc_info.pMappedData);

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
            if (vma_allocation && vma_allocator_ref)
            {
                vmaDestroyBuffer(vma_allocator_ref, heap_vk_buffer, vma_allocation);
            }
            if (sparse_alloc)
                vmaFreeMemory(sparse_device->get_vma_allocator(), sparse_alloc);
        }

        std::pair<VkDeviceMemory, VkDeviceSize> Heap::get_sparse_memory()
        {
            std::lock_guard lock(sparse_mutex);
            if (!sparse_alloc)
            {
                VkMemoryRequirements req{};
                req.size           = sparse_size;
                req.alignment      = Device::SPARSE_TILE_SIZE;
                req.memoryTypeBits = sparse_device->get_sparse_memory_type_bits();

                VmaAllocationCreateInfo ci{};
                ci.flags         = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
                ci.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

                VmaAllocationInfo info{};
                VkResult r = vmaAllocateMemory(sparse_device->get_vma_allocator(), &req, &ci, &sparse_alloc, &info);
                ASSERT(r == VK_SUCCESS && "tile heap memory allocation failed");
                sparse_memory = info.deviceMemory;
                sparse_offset = info.offset;
                sparse_memory_type = info.memoryType;
            }
            return { sparse_memory, sparse_offset };
        }

        GPUAddressPtr Heap::get_address() const { return gpu_address; }
    }
}
