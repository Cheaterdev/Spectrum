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

        // Everything needed to (re)write one heap slot.  D3D12 descriptors are
        // self-contained blobs that CopyDescriptors can memcpy between any two
        // heaps; a VkDescriptorSet slot can't be read back, so each heap keeps the
        // source of every slot and a copy re-writes it at the destination.
        struct DescriptorRecord
        {
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;  // MAX_ENUM: empty slot
            union
            {
                VkDescriptorImageInfo  image;
                VkDescriptorBufferInfo buffer;
            };
            // Append/Consume counter -- DXC reads slot i's counter from element i
            // of set 0, binding 2.  buffer == VK_NULL_HANDLE when there is none.
            VkDescriptorBufferInfo counter{};

            DescriptorRecord() : buffer{} {}
            bool empty() const noexcept { return type == VK_DESCRIPTOR_TYPE_MAX_ENUM; }
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
        // A shader-visible CBV_SRV_UAV or SAMPLER heap is one descriptor set of the
        // device's bindless layout (set 0 / set 1), and a slot index is the array
        // element the shader indexes ResourceDescriptorHeap / SamplerDescriptorHeap
        // with.  Other heaps own no Vulkan objects: RTV/DSV are handled by dynamic
        // rendering, and CPU-only heaps only keep records to copy from.
        class DescriptorHeap
        {
        protected:
            VkDescriptorPool vk_pool = VK_NULL_HANDLE;
            VkDescriptorSet  vk_set  = VK_NULL_HANDLE;
            uint             capacity = 0;
            std::vector<DescriptorRecord> records;

            // vkUpdateDescriptorSets requires external synchronization of dstSet.
            std::mutex write_mutex;

        public:
            const DescriptorHeapDesc desc;
            Device& device;

            uint handle_size = 0;

            friend class HAL::Descriptor;
        public:
            DescriptorHeap(Device& device, const DescriptorHeapDesc& desc);
            virtual ~DescriptorHeap();

            HAL::Descriptor operator[](uint i);

            // D3D12 stages descriptors in a CPU heap and copies ranges to the GPU
            // heap on demand; here place() writes the shader-visible set directly,
            // so this batched sync is a no-op -- kept for shared-code interface
            // parity (FrameManager calls it).
            void copy_ranges_to_gpu(std::span<const std::pair<uint64, uint64>> ranges) {}

            VkDescriptorSet get_vk_set() const noexcept { return vk_set; }

            // Record slot `slot` and, for a shader-visible heap, write it to the set.
            void store(uint slot, const DescriptorRecord& record);
            const DescriptorRecord* get_record(uint slot) const noexcept
            {
                return slot < records.size() ? &records[slot] : nullptr;
            }
        };
    }
}
