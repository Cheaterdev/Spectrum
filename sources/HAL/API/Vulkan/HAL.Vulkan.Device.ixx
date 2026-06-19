export module HAL:API.Device;

import stl.core;
import vulkan;
import Core;

import :Types;
import :Sampler;
import :Utils;
import :Adapter;

using namespace HAL;

// Forward declarations for HAL-layer classes that live outside namespace API
// but must access protected API::Device fields (e.g. via static_cast<API::Device&>).
// Full definitions live in their own partitions; only names needed here.
namespace HAL
{
    class Heap;
    class SwapChain;
    class CommandAllocator;
}

export namespace HAL
{
    namespace API
    {
        class Device
        {
            std::map<ResourceDesc, ResourceAllocationInfo> alloc_info;

            // API-namespace sibling classes access fields directly (same module,
            // same namespace — friend grants protected access cleanly).
            friend class Resource;
            friend class Queue;

            // HAL-layer wrappers (namespace HAL, not API) that cast to API::Device
            // and touch Vulkan internals. Forward-declared above.
            friend class HAL::Heap;
            friend class HAL::SwapChain;
            friend class HAL::CommandAllocator;

        protected:
            void init(DeviceDesc& desc);
            virtual ~Device();

            // ---- Vulkan objects (backend-internal) ---------------------------
            // vk_instance is owned by HAL::init() (HAL.Impl.cpp static).
            VkInstance       vk_instance   = VK_NULL_HANDLE;
            VkPhysicalDevice vk_physical   = VK_NULL_HANDLE;
            VkDevice         vk_device     = VK_NULL_HANDLE;
            VmaAllocator     vma_allocator = VK_NULL_HANDLE;

            // Queue family indices: DIRECT=0, COMPUTE=1, COPY=2.
            uint32_t queue_families[3] = {
                static_cast<uint32_t>(-1),
                static_cast<uint32_t>(-1),
                static_cast<uint32_t>(-1)
            };

            // Descriptor sizes — interface compat; always 0 in Vulkan.
            enum_array<DescriptorHeapType, uint> descriptor_sizes;

            // Global bindless descriptor set layouts (created in init()).
            VkDescriptorSetLayout cbv_srv_uav_layout = VK_NULL_HANDLE;
            VkDescriptorSetLayout sampler_layout     = VK_NULL_HANDLE;

            // Inline static samplers s0..s4 (FrameLayout.h, binding 384+N in set 0).
            // Created in init(); written into every CBV_SRV_UAV DescriptorHeap set.
            static constexpr uint32_t NUM_INLINE_SMP = 5;
            VkSampler inline_samplers[NUM_INLINE_SMP] = {};

            // ---- Pending initial-layout transitions --------------------------
            // D3D12 creates resources directly in their initial state; Vulkan
            // images always start in UNDEFINED.  The HAL state manager assumes
            // the D3D12 model (resources rest in their initial layout between
            // command lists), so every freshly created VkImage queues a one-time
            // UNDEFINED -> initial_layout barrier here.  The next Queue::execute
            // flushes the batch in a small command buffer submitted ahead of the
            // real work, making the state manager's assumption true.
            std::mutex pending_init_mutex;
            std::vector<VkImageMemoryBarrier2> pending_init_barriers;

        public:
            using ptr = std::shared_ptr<Device>;

            // ---- HAL contract (public interface) -----------------------------
            void     process_result(VkResult result, std::string_view line) const;
            uint     get_descriptor_size(DescriptorHeapType type) const;
            VkDevice get_native_device() const;   // returns vk_device
            VkResult get_device_removed_reason() const;

            // ---- Backend accessors (used by sibling Vulkan modules) ----------
            // Cross-partition friend declarations are not reliably enforced by
            // MSVC, so we expose the Vulkan internals through thin inline getters
            // rather than relying on friend access across partition boundaries.
            VmaAllocator     get_vma_allocator()     const noexcept { return vma_allocator; }
            VkPhysicalDevice get_vk_physical_dev()   const noexcept { return vk_physical; }
            uint32_t         get_queue_family(int i) const noexcept { return queue_families[i]; }

            // ---- Global bindless descriptor set layouts ----------------------
            // Created once in init(); shared by DescriptorHeap and RootSignature.
            VkDescriptorSetLayout get_cbv_srv_uav_layout() const noexcept { return cbv_srv_uav_layout; }
            VkDescriptorSetLayout get_sampler_layout()     const noexcept { return sampler_layout; }
            const VkSampler*      get_inline_samplers()    const noexcept { return inline_samplers; }

            // Queue a one-time UNDEFINED -> layout transition for a new image.
            // Flushed by the next Queue::execute on whichever queue submits first
            // (cross-queue first use is safe: queues sharing a resource are
            // already fence-synchronized by the FrameGraph).
            void queue_initial_transition(VkImage image, VkImageLayout layout, VkImageAspectFlags aspect);
            void cancel_pending_init_transition(VkImage image);
            std::vector<VkImageMemoryBarrier2> take_pending_init_transitions();

            ResourceAllocationInfo get_alloc_info(const ResourceDesc& desc);
            uint   Subresources(const ResourceDesc& desc) const;
            size_t get_vram();
            size_t get_upload_heap();
            size_t get_readback_heap();

            RaytracingPrebuildInfo calculateBuffers(const RaytracingBuildDescBottomInputs& desc);
            RaytracingPrebuildInfo calculateBuffers(const RaytracingBuildDescTopInputs& desc);
        };
    }
}
