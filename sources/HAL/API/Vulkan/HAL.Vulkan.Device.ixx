module;
// Global module fragment: extension types (e.g. VkMutableDescriptorTypeCreateInfoEXT)
// are not reliably visible through the `vulkan` header unit, so include the header directly.
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
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
        // True if `result` is VK_ERROR_DEVICE_LOST. The first loss logs `where`
        // and the VK_EXT_device_fault report (faulting addresses, vendor info),
        // when the device supports it. Call on the result of every submit,
        // present, acquire and semaphore wait/query.
        bool check_device_lost(VkResult result, const char* where);

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

            // Per HAL::CommandListType (DIRECT, COMPUTE, COPY, COMPUTE2, COMPUTE3):
            // queue family and queue index within it. Types get distinct queues
            // while the family has them, then share the family's last one.
            static constexpr uint32_t queue_type_count = 5;
            uint32_t queue_families[queue_type_count] = {
                static_cast<uint32_t>(-1), static_cast<uint32_t>(-1), static_cast<uint32_t>(-1),
                static_cast<uint32_t>(-1), static_cast<uint32_t>(-1)
            };
            uint32_t queue_indices[queue_type_count] = {};

            // VkQueue must be externally synchronized, so types that share a
            // VkQueue must share its mutex: queue_mutex_slot maps each type to
            // the first type using the same (family, index).
            std::array<std::mutex, queue_type_count> queue_mutexes;
            uint32_t queue_mutex_slot[queue_type_count] = { 0, 1, 2, 3, 4 };

            // Descriptor sizes — interface compat; always 0 in Vulkan.
            enum_array<DescriptorHeapType, uint> descriptor_sizes;

            // ---- Bindless descriptor model ----------------------------------
            // Every pipeline shares one layout, created here (see init() for the
            // set/binding table it must match).  A shader-visible heap is one
            // descriptor set of set_layouts[set].
            static constexpr uint32_t NUM_INLINE_SMP     = 7;   // Frame::FrameLayout static samplers
            static constexpr uint32_t PUSH_CONSTANT_SIZE = 128; // Vulkan's guaranteed minimum
            VkSampler             inline_samplers[NUM_INLINE_SMP] = {};
            VkDescriptorSetLayout resource_set_layout = VK_NULL_HANDLE;   // set 0
            VkDescriptorSetLayout sampler_set_layout  = VK_NULL_HANDLE;   // set 1
            VkPipelineLayout      pipeline_layout     = VK_NULL_HANDLE;
            uint32_t              resource_heap_capacity = 0;
            uint32_t              sampler_heap_capacity  = 0;
            bool                  null_descriptor        = false;   // VK_EXT_robustness2

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
            uint32_t         get_queue_index(int i)  const noexcept { return queue_indices[i]; }
            std::mutex&      get_queue_mutex(int i)        noexcept { return queue_mutexes[queue_mutex_slot[i]]; }

            // ---- Bindless descriptor model accessors -------------------------
            // The types a resource-heap slot can hold. Every slot costs the size of
            // the largest of these, so keep the list minimal. Layouts and pools must
            // declare the same list.
            static constexpr VkDescriptorType mutable_resource_types[] = {
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            };

            VkDescriptorSetLayout get_resource_set_layout()    const noexcept { return resource_set_layout; }
            VkDescriptorSetLayout get_sampler_set_layout()     const noexcept { return sampler_set_layout; }
            VkPipelineLayout      get_pipeline_layout()        const noexcept { return pipeline_layout; }
            uint32_t              get_resource_heap_capacity() const noexcept { return resource_heap_capacity; }
            uint32_t              get_sampler_heap_capacity()  const noexcept { return sampler_heap_capacity; }
            // Whether a descriptor may be written with a null resource (D3D12 null view).
            bool                  supports_null_descriptor()   const noexcept { return null_descriptor; }
            static constexpr uint32_t get_push_constant_size()  noexcept { return PUSH_CONSTANT_SIZE; }
            static constexpr uint32_t get_inline_sampler_count() noexcept { return NUM_INLINE_SMP; }

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
