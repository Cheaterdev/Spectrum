module;
// Global module fragment: VK_EXT_descriptor_heap types (VkDescriptorSetAndBindingMappingEXT,
// VkShaderDescriptorSetAndBindingMappingInfoEXT, VkPhysicalDeviceDescriptorHeapPropertiesEXT)
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

            // ---- VK_EXT_descriptor_heap ------------------------------------
            // Descriptor sizes reported by the driver.  Unlike D3D12 (one uniform
            // CBV_SRV_UAV increment), Vulkan reports separate image/buffer/sampler
            // sizes.  We use a single uniform resource stride = max(image,buffer)
            // so a flat heap keeps D3D12's "slot index == array element, uniform
            // increment" model; the sampler heap uses its own stride.
            VkDeviceSize resource_descriptor_size = 0;   // uniform resource stride
            VkDeviceSize sampler_descriptor_size  = 0;
            VkDeviceSize resource_heap_alignment  = 0;
            VkDeviceSize sampler_heap_alignment   = 0;
            VkDeviceSize resource_reserved_range  = 0;   // minResourceHeapReservedRange
            VkDeviceSize sampler_reserved_range   = 0;   // minSamplerHeapReservedRange(WithEmbedded)

            // Shader set/binding -> heap mapping table, built once in init().
            // Storage must outlive every pipeline (pipelines reference pMappings
            // by pointer at creation), so it is owned here for the device lifetime.
            // Inline static samplers s0..s4 (FrameLayout.h) are embedded directly
            // into the resource heap via pEmbeddedSampler; we keep their
            // VkSamplerCreateInfo alive (pointed at by the mapping table).
            static constexpr uint32_t NUM_INLINE_SMP = 5;
            std::vector<VkSamplerCreateInfo>                 embedded_sampler_cis;
            std::vector<VkDescriptorSetAndBindingMappingEXT> binding_mappings;
            VkShaderDescriptorSetAndBindingMappingInfoEXT    binding_mapping_info{};

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

            // ---- VK_EXT_descriptor_heap accessors ----------------------------
            // Uniform resource-heap slot stride (D3D12 "handle increment size").
            VkDeviceSize get_resource_descriptor_size() const noexcept { return resource_descriptor_size; }
            VkDeviceSize get_sampler_descriptor_size()  const noexcept { return sampler_descriptor_size; }
            VkDeviceSize get_resource_heap_alignment()  const noexcept { return resource_heap_alignment; }
            VkDeviceSize get_sampler_heap_alignment()   const noexcept { return sampler_heap_alignment; }
            VkDeviceSize get_resource_reserved_range()  const noexcept { return resource_reserved_range; }
            VkDeviceSize get_sampler_reserved_range()   const noexcept { return sampler_reserved_range; }

            // Shader set/binding -> heap mapping, chained into every pipeline's stages.
            const VkShaderDescriptorSetAndBindingMappingInfoEXT& get_binding_mapping_info() const noexcept
            { return binding_mapping_info; }

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
