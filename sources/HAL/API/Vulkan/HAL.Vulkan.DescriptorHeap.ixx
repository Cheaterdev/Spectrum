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
        // VK_EXT_descriptor_heap: a heap is a host-visible, device-addressable
        // VkBuffer holding raw descriptors at a uniform stride.  This mirrors
        // D3D12 exactly — a descriptor "handle" is just base + slot*stride, and
        // CopyDescriptors is a plain memcpy between mapped heaps.
        class DescriptorHeap
        {
        protected:
            VkBuffer      vk_heap_buffer = VK_NULL_HANDLE;
            VmaAllocation vma_alloc      = VK_NULL_HANDLE;
            uint8_t*      mapped         = nullptr;  // persistent CPU mapping
            VkDeviceAddress device_address = 0;      // heap base GPU address

            VkDeviceSize  descriptor_size = 0;       // per-slot stride (uniform)
            VkDeviceSize  main_size       = 0;       // Count * stride
            VkDeviceSize  reserved_offset = 0;       // start of reserved (embedded) range
            VkDeviceSize  reserved_size   = 0;       // size of reserved range

        public:
            const DescriptorHeapDesc desc;
            Device& device;   // non-const: place() calls vkWriteResourceDescriptorsEXT

            uint handle_size = 0;

            friend class HAL::Descriptor;
        public:
            DescriptorHeap(Device& device, const DescriptorHeapDesc& desc);
            virtual ~DescriptorHeap();

            HAL::Descriptor operator[](uint i);

            // D3D12 stages descriptors in a CPU heap and copies ranges to the GPU
            // heap on demand.  VK_EXT_descriptor_heap writes straight into the
            // host-visible GPU heap, so this batched sync is a no-op — kept for
            // shared-code interface parity (FrameManager calls it).
            void copy_ranges_to_gpu(std::span<const std::pair<uint64, uint64>> ranges) {}

            // Heap binding info for CommandList::set_descriptor_heaps.
            VkDeviceAddress get_device_address() const noexcept { return device_address; }
            VkDeviceSize    get_total_size()     const noexcept { return main_size + reserved_size; }
            VkDeviceSize    get_reserved_offset()const noexcept { return reserved_offset; }
            VkDeviceSize    get_reserved_size()  const noexcept { return reserved_size; }
            VkDeviceSize    get_descriptor_size()const noexcept { return descriptor_size; }
            uint8_t*        get_mapped()         const noexcept { return mapped; }
        };
    }
}
