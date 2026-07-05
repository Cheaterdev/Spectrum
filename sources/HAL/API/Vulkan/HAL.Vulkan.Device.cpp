module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
module HAL:Device;

import :Debug;
import :Utils;
import :Impl;    // get_vk_instance()
import :Sampler;
import :HeapAllocators;

import stl.core;
import Core;

// ---- VK_EXT_descriptor_heap entry points --------------------------------
// These commands are guarded by VK_ONLY_EXPORTED_PROTOTYPES in vulkan_core.h,
// so vulkan-1.dll does NOT export them and the linker cannot resolve them
// directly.  We load them via vkGetDeviceProcAddr into function pointers and
// provide trampoline definitions (matching the extern "C" header declarations)
// that delegate through those pointers.  Because they have C language linkage
// they attach to the global module, so every Vulkan TU that includes
// vulkan.h can call them and link against this single definition.
namespace
{
    PFN_vkWriteResourceDescriptorsEXT pfn_vkWriteResourceDescriptorsEXT = nullptr;
    PFN_vkWriteSamplerDescriptorsEXT  pfn_vkWriteSamplerDescriptorsEXT  = nullptr;
    PFN_vkCmdBindResourceHeapEXT      pfn_vkCmdBindResourceHeapEXT      = nullptr;
    PFN_vkCmdBindSamplerHeapEXT       pfn_vkCmdBindSamplerHeapEXT       = nullptr;
    PFN_vkCmdPushDataEXT              pfn_vkCmdPushDataEXT              = nullptr;

    void load_descriptor_heap_ext(VkDevice dev)
    {
    #define LOAD(name)                                                           \
        pfn_##name = reinterpret_cast<PFN_##name>(vkGetDeviceProcAddr(dev, #name)); \
        if (!pfn_##name) Log::get() << Log::LEVEL_ERROR                           \
            << "[Vulkan] vkGetDeviceProcAddr returned null for " #name << Log::endl
        LOAD(vkWriteResourceDescriptorsEXT);
        LOAD(vkWriteSamplerDescriptorsEXT);
        LOAD(vkCmdBindResourceHeapEXT);
        LOAD(vkCmdBindSamplerHeapEXT);
        LOAD(vkCmdPushDataEXT);
    #undef LOAD
    }
}

VKAPI_ATTR VkResult VKAPI_CALL vkWriteResourceDescriptorsEXT(
    VkDevice device, uint32_t resourceCount,
    const VkResourceDescriptorInfoEXT* pResources,
    const VkHostAddressRangeEXT* pDescriptors)
{ return pfn_vkWriteResourceDescriptorsEXT(device, resourceCount, pResources, pDescriptors); }

VKAPI_ATTR VkResult VKAPI_CALL vkWriteSamplerDescriptorsEXT(
    VkDevice device, uint32_t samplerCount,
    const VkSamplerCreateInfo* pSamplers,
    const VkHostAddressRangeEXT* pDescriptors)
{ return pfn_vkWriteSamplerDescriptorsEXT(device, samplerCount, pSamplers, pDescriptors); }

VKAPI_ATTR void VKAPI_CALL vkCmdBindResourceHeapEXT(VkCommandBuffer cb, const VkBindHeapInfoEXT* pInfo)
{ pfn_vkCmdBindResourceHeapEXT(cb, pInfo); }

VKAPI_ATTR void VKAPI_CALL vkCmdBindSamplerHeapEXT(VkCommandBuffer cb, const VkBindHeapInfoEXT* pInfo)
{ pfn_vkCmdBindSamplerHeapEXT(cb, pInfo); }

VKAPI_ATTR void VKAPI_CALL vkCmdPushDataEXT(VkCommandBuffer cb, const VkPushDataInfoEXT* pInfo)
{ pfn_vkCmdPushDataEXT(cb, pInfo); }

// Vulkan native implementation of HAL::Device.
// Mirrors the partition layout of D3D12/HAL.D3D12.Device.cpp.

namespace HAL
{
    // ---- Common HAL::Device methods ----------------------------------------

    texture_layout Device::get_texture_layout(const ResourceDesc& rdesc, UINT sub_resource)
    {
        auto& desc = rdesc.as_texture();
        // Use the actual mip level's dimensions (and depth for 3D) — NOT mip 0.
        // The previous version always returned the mip-0 layout, which corrupted
        // readback (and thus the texture cache) for multi-mip and volume textures.
        const uint mip = desc.get_mip(sub_resource);
        const uint w = std::max(1u, desc.Dimensions.x >> mip);
        const uint h = desc.is1D() ? 1u : std::max(1u, desc.Dimensions.y >> mip);
        const uint d = desc.is3D() ? std::max(1u, desc.Dimensions.z >> mip) : 1u;
        auto info = desc.Format.surface_info({ w, h });
        const uint64 size = static_cast<uint64>(info.numBytes) * d;
        return {
            size, info.numRows, info.rowBytes,
            static_cast<uint>(info.numBytes), 256u, desc.Format
        };
    }

    texture_layout Device::get_texture_layout(const ResourceDesc& rdesc, UINT sub_resource, ivec3 box)
    {
        auto& desc = rdesc.as_texture();
        auto info = desc.Format.surface_info({ (uint)box.x, (uint)box.y });
        uint64 res_stride = Math::AlignUp((uint64)info.rowBytes, 256ull);
        uint64 size = res_stride * info.numRows * box.z;
        return {
            size, info.numRows, static_cast<uint>(res_stride),
            static_cast<uint>(res_stride * info.numRows), 512u, desc.Format
        };
    }

    std::vector<std::byte> Device::compress(std::span<std::byte> source)
    {
        std::vector<std::byte> dest;
        dest.assign(source.data(), source.data() + source.size());
        return dest;
    }

    HAL::DeviceProperties Device::probe(HAL::Adapter::ptr adapter)
    {
        HAL::DeviceProperties props;

        VkPhysicalDevice vk_physical = adapter ? adapter->get_vk_physical() : VK_NULL_HANDLE;
        if (vk_physical == VK_NULL_HANDLE)
            return props;

        VkPhysicalDeviceProperties2 phys_props{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        vkGetPhysicalDeviceProperties2(vk_physical, &phys_props);
        props.name = phys_props.properties.deviceName;
        props.min_storage_buffer_offset_alignment =
            static_cast<uint32_t>(phys_props.properties.limits.minStorageBufferOffsetAlignment);

        uint32_t avail_count = 0;
        vkEnumerateDeviceExtensionProperties(vk_physical, nullptr, &avail_count, nullptr);
        std::vector<VkExtensionProperties> avail_exts(avail_count);
        vkEnumerateDeviceExtensionProperties(vk_physical, nullptr, &avail_count, avail_exts.data());

        auto has_ext = [&](const char* name) {
            for (auto& ext : avail_exts)
                if (strcmp(ext.extensionName, name) == 0) return true;
            return false;
        };

        props.mesh_shader   = has_ext(VK_EXT_MESH_SHADER_EXTENSION_NAME);
        props.full_bindless = has_ext(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        props.rtx           = false;
        props.work_graph    = false;

        return props;
    }

    // ---- HAL::API::Device --------------------------------------------------

    namespace API
    {
        void Device::queue_initial_transition(VkImage image, VkImageLayout layout, VkImageAspectFlags aspect)
        {
            if (image == VK_NULL_HANDLE || layout == VK_IMAGE_LAYOUT_UNDEFINED) return;

            VkImageMemoryBarrier2 b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
            b.srcStageMask     = VK_PIPELINE_STAGE_2_NONE;
            b.srcAccessMask    = 0;
            b.dstStageMask     = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            b.dstAccessMask    = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
            b.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
            b.newLayout        = layout;
            b.image            = image;
            b.subresourceRange = { aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };

            std::lock_guard lock(pending_init_mutex);
            pending_init_barriers.push_back(b);
        }

        void Device::cancel_pending_init_transition(VkImage image)
        {
            if (image == VK_NULL_HANDLE) return;
            std::lock_guard lock(pending_init_mutex);
            auto& v = pending_init_barriers;
            v.erase(std::remove_if(v.begin(), v.end(),
                [image](const VkImageMemoryBarrier2& b) { return b.image == image; }),
                v.end());
        }

        std::vector<VkImageMemoryBarrier2> Device::take_pending_init_transitions()
        {
            std::lock_guard lock(pending_init_mutex);
            return std::move(pending_init_barriers);
        }

        void Device::init(DeviceDesc& device_desc)
        {
            auto THIS = static_cast<HAL::Device*>(this);
            THIS->adapter = device_desc.adapter;
            vk_instance = HAL::get_vk_instance();
            vk_physical = device_desc.adapter ? device_desc.adapter->get_vk_physical() : VK_NULL_HANDLE;

            if (vk_physical == VK_NULL_HANDLE) return;

            // ---- Queue families --------------------------------------------
            uint32_t qf_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(vk_physical, &qf_count, nullptr);
            std::vector<VkQueueFamilyProperties> qf_props(qf_count);
            vkGetPhysicalDeviceQueueFamilyProperties(vk_physical, &qf_count, qf_props.data());

            uint32_t graphics_family = UINT32_MAX;
            uint32_t compute_family  = UINT32_MAX;
            uint32_t transfer_family = UINT32_MAX;

            for (uint32_t i = 0; i < qf_count; ++i)
            {
                if (graphics_family == UINT32_MAX &&
                    (qf_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                    graphics_family = i;

                if (compute_family == UINT32_MAX &&
                    (qf_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                    !(qf_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                    compute_family = i;

                if (transfer_family == UINT32_MAX &&
                    (qf_props[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                    !(qf_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                    !(qf_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
                    transfer_family = i;
            }
            // Fall back to graphics queue if dedicated queues not found
            if (compute_family  == UINT32_MAX) compute_family  = graphics_family;
            if (transfer_family == UINT32_MAX) transfer_family = graphics_family;

            // Store for Queue::construct() and CommandAllocator
            queue_families[0] = graphics_family;   // DIRECT
            queue_families[1] = compute_family;    // COMPUTE
            queue_families[2] = transfer_family;   // COPY

            // ---- Queue create infos ----------------------------------------
            const float priority = 1.0f;
            std::vector<VkDeviceQueueCreateInfo> queue_infos;
            std::set<uint32_t> unique_families = { graphics_family, compute_family, transfer_family };
            for (uint32_t qf : unique_families)
            {
                VkDeviceQueueCreateInfo qi{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
                qi.queueFamilyIndex = qf;
                qi.queueCount       = 1;
                qi.pQueuePriorities = &priority;
                queue_infos.push_back(qi);
            }

            // ---- Extensions: only request what the device actually supports ----
            // Many of these are promoted to Vulkan 1.2/1.3 core.  Some drivers
            // reject listing already-core extensions in ppEnabledExtensionNames,
            // so we filter against the device's reported extension list first.
            const char* wanted_extensions[] = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
                VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
                VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
                // The D3D12-identical descriptor model: heaps are memory-backed
                // buffers, root signatures carry no layout objects, SM6.6
                // ResourceDescriptorHeap/SamplerDescriptorHeap map straight in.
                VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME,
                // Required dependency of descriptor_heap; also provides
                // VkPipelineCreateFlags2CreateInfo and VK_NULL_HANDLE layouts.
                VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
                // Optional: allows vkCmdCopyImage between depth (D32) and color (R32)
                // images — needed to build the color Hi-Z pyramid from the depth GBuffer.
                VK_KHR_MAINTENANCE_8_EXTENSION_NAME,
                // Required by DXC SPIRV for 'discard' in pixel shaders.
                VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME,
                // Optional: ddx/ddy in compute shaders.
                VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME,
                // Optional: mesh/task shaders.
                VK_EXT_MESH_SHADER_EXTENSION_NAME,
            };

            uint32_t avail_count = 0;
            vkEnumerateDeviceExtensionProperties(vk_physical, nullptr, &avail_count, nullptr);
            std::vector<VkExtensionProperties> avail_exts(avail_count);
            vkEnumerateDeviceExtensionProperties(vk_physical, nullptr, &avail_count, avail_exts.data());

            std::vector<const char*> device_extensions;
            for (auto wanted : wanted_extensions)
            {
                for (auto& ext : avail_exts)
                    if (strcmp(ext.extensionName, wanted) == 0)
                    { device_extensions.push_back(wanted); break; }
            }

            auto has_ext = [&](const char* name) {
                return std::any_of(device_extensions.begin(), device_extensions.end(),
                    [name](const char* e) { return strcmp(e, name) == 0; });
            };
            const bool has_mesh_shader    = has_ext(VK_EXT_MESH_SHADER_EXTENSION_NAME);
            const bool has_cs_derivatives = has_ext(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
            const bool has_maintenance8   = has_ext(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);

            // VK_EXT_descriptor_heap is not optional — the entire descriptor/root
            // signature model depends on it.  Fail loudly if the GPU/driver lacks it.
            if (!has_ext(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME) ||
                !has_ext(VK_KHR_MAINTENANCE_5_EXTENSION_NAME))
            {
                Log::get().crash_error(
                    "[Vulkan] VK_EXT_descriptor_heap (+ VK_KHR_maintenance5) is required "
                    "but not supported by this GPU/driver. Update your driver or select a "
                    "device that supports it.");
                return;
            }

            // ---- Feature chain ----------------------------------------------
            // All Vulkan 1.2 promoted features must live in a single
            // VkPhysicalDeviceVulkan12Features — the individual VkPhysicalDevice*Features
            // structs for these cannot coexist with it in the same pNext chain.
            VkPhysicalDeviceVulkan12Features vk12_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
            vk12_features.bufferDeviceAddress                       = VK_TRUE;
            vk12_features.timelineSemaphore                         = VK_TRUE;
            vk12_features.runtimeDescriptorArray                    = VK_TRUE;
            vk12_features.descriptorBindingPartiallyBound           = VK_TRUE;
            vk12_features.descriptorBindingVariableDescriptorCount  = VK_TRUE;
            vk12_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
            vk12_features.scalarBlockLayout                         = VK_TRUE;  // sub-16 StructuredBuffer strides
            vk12_features.hostQueryReset                            = VK_TRUE;  // vkResetQueryPool from CPU
            vk12_features.separateDepthStencilLayouts               = VK_TRUE;  // DEPTH-only barriers on D24S8
            vk12_features.drawIndirectCount                         = VK_TRUE;  // execute_indirect *IndirectCount

            VkPhysicalDeviceSynchronization2Features sync2_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES };
            sync2_features.synchronization2 = VK_TRUE;
            sync2_features.pNext = &vk12_features;

            VkPhysicalDeviceDynamicRenderingFeatures dr_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES };
            dr_features.dynamicRendering = VK_TRUE;
            dr_features.pNext = &sync2_features;

            // VK_EXT_descriptor_heap: the D3D12-style memory-backed heap model.
            VkPhysicalDeviceDescriptorHeapFeaturesEXT heap_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT };
            heap_features.descriptorHeap = VK_TRUE;
            heap_features.pNext = &dr_features;

            // maintenance5: dependency of descriptor_heap; enables
            // VkPipelineCreateFlags2CreateInfo and VK_NULL_HANDLE pipeline layouts.
            VkPhysicalDeviceMaintenance5Features maint5_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES };
            maint5_features.maintenance5 = VK_TRUE;
            maint5_features.pNext = &heap_features;

            // DemoteToHelperInvocation: 'discard' in pixel shaders uses this capability.
            VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures demote_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES };
            demote_features.shaderDemoteToHelperInvocation = VK_TRUE;
            demote_features.pNext = &maint5_features;

            // extendedDynamicState: required for vkCmdSetPrimitiveTopology (used by
            // set_topology / reapply_draw_state).
            VkPhysicalDeviceExtendedDynamicStateFeaturesEXT eds_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
            eds_features.extendedDynamicState = VK_TRUE;
            eds_features.pNext = &demote_features;

            // Optional feature structs — only inserted into the chain if the
            // extension is present.  Including structs for absent extensions in
            // vkCreateDevice's pNext may trigger VK_ERROR_EXTENSION_NOT_PRESENT.
            VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR cs_deriv_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR };
            cs_deriv_features.computeDerivativeGroupQuads  = VK_TRUE;
            cs_deriv_features.computeDerivativeGroupLinear = VK_TRUE;

            VkPhysicalDeviceMeshShaderFeaturesEXT mesh_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
            mesh_features.meshShader = VK_TRUE;
            mesh_features.taskShader = VK_TRUE;

            // maintenance8: depth<->color image copies (build color Hi-Z from depth GBuffer).
            VkPhysicalDeviceMaintenance8FeaturesKHR maint8_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR };
            maint8_features.maintenance8 = VK_TRUE;

            void* feature_head = &eds_features;
            if (has_cs_derivatives) {
                cs_deriv_features.pNext = feature_head;
                feature_head = &cs_deriv_features;
            }
            if (has_mesh_shader) {
                mesh_features.pNext = feature_head;
                feature_head = &mesh_features;
            }
            if (has_maintenance8) {
                maint8_features.pNext = feature_head;
                feature_head = &maint8_features;
            }

            VkPhysicalDeviceFeatures2 features2{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
            features2.pNext = feature_head;
            vkGetPhysicalDeviceFeatures2(vk_physical, &features2);

            // ---- Create logical device --------------------------------------
            VkDeviceCreateInfo device_ci{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
            device_ci.queueCreateInfoCount    = static_cast<uint32_t>(queue_infos.size());
            device_ci.pQueueCreateInfos       = queue_infos.data();
            device_ci.enabledExtensionCount   = static_cast<uint32_t>(device_extensions.size());
            device_ci.ppEnabledExtensionNames = device_extensions.data();
            device_ci.pNext                   = &features2;

            VkResult result = vkCreateDevice(vk_physical, &device_ci, nullptr, &vk_device);
            if (result != VK_SUCCESS)
            {
                Log::get().crash_error(
                    std::string("vkCreateDevice failed, VkResult=") + std::to_string(static_cast<int>(result)));
                return;
            }

            // ---- Load VK_EXT_descriptor_heap entry points -------------------
            load_descriptor_heap_ext(vk_device);

            // ---- DeviceProperties -------------------------------------------
            VkPhysicalDeviceDescriptorHeapPropertiesEXT heap_props{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT };
            VkPhysicalDeviceProperties2 props2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
            props2.pNext = &heap_props;
            vkGetPhysicalDeviceProperties2(vk_physical, &props2);

            // Uniform resource stride = max(image,buffer) so a flat heap keeps
            // D3D12's "slot index == array element, uniform increment" model.
            resource_descriptor_size = std::max(heap_props.imageDescriptorSize,
                                                heap_props.bufferDescriptorSize);
            sampler_descriptor_size  = heap_props.samplerDescriptorSize;
            resource_heap_alignment  = heap_props.resourceHeapAlignment;
            sampler_heap_alignment   = heap_props.samplerHeapAlignment;
            resource_reserved_range  = heap_props.minResourceHeapReservedRange;
            sampler_reserved_range   = heap_props.minSamplerHeapReservedRangeWithEmbedded;

            auto& p = THIS->properties;
            p.name = props2.properties.deviceName;
            p.rtx           = false;  // Phase: VK_KHR_ray_tracing_pipeline check
            p.mesh_shader   = has_mesh_shader && (mesh_features.meshShader == VK_TRUE);
            p.work_graph    = false;  // no Vulkan equivalent yet
            p.min_storage_buffer_offset_alignment =
                static_cast<uint32_t>(props2.properties.limits.minStorageBufferOffsetAlignment);
            // full_bindless = true whenever the Vulkan device creates successfully.
            // The D3D12 version gates on shader model 6.6; on Vulkan, bindless is
            // always available once descriptor indexing features are enabled, so
            // we just require a working device.  This is what the common
            // Device::create_singleton() selection logic checks.
            p.full_bindless = true;

            // ---- VMA allocator ----------------------------------------------
            VmaAllocatorCreateInfo vma_info{};
            vma_info.physicalDevice   = vk_physical;
            vma_info.device           = vk_device;
            vma_info.instance         = vk_instance;
            vma_info.vulkanApiVersion = VK_API_VERSION_1_3;
            vma_info.flags            = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            vmaCreateAllocator(&vma_info, &vma_allocator);

            // ---- VRAM info --------------------------------------------------
            VkPhysicalDeviceMemoryProperties mem_props{};
            vkGetPhysicalDeviceMemoryProperties(vk_physical, &mem_props);
            size_t vram = 0;
            for (uint32_t i = 0; i < mem_props.memoryHeapCount; ++i)
                if (mem_props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                    vram += mem_props.memoryHeaps[i].size;

            // ---- Shader set/binding -> heap mapping table -------------------
            // VK_EXT_descriptor_heap replaces descriptor set layouts entirely.
            // DXC SPIR-V (see HAL.Vulkan.ShaderReflection.cpp flags) emits:
            //   set 0, binding 0        : ResourceDescriptorHeap  (CBV/SRV/UAV, any type)
            //   set 0, binding 2        : counter.var.ResourceDescriptorHeap (append counters)
            //   set 0, bindings 384..388: inline static samplers s0..s4 (s-shift = 384)
            //   set 1, binding 0        : SamplerDescriptorHeap
            // Each is translated to a heap access here.  Storage lives on the
            // device for the pipeline lifetime (pMappings referenced by pointer).
            {
                constexpr uint32_t SMP_BASE = 384; // s-shift

                // Embedded static samplers s0..s4 — create-infos kept alive so the
                // mapping's pEmbeddedSampler stays valid.  Order matches FrameLayout.h:
                //   s0=linearWrap, s1=pointClamp, s2=linearClamp, s3=anisoBorder, s4=pointBorder
                const SamplerDesc* descs[NUM_INLINE_SMP] = {
                    &Samplers::SamplerLinearWrapDesc,
                    &Samplers::SamplerPointClampDesc,
                    &Samplers::SamplerLinearClampDesc,
                    &Samplers::SamplerAnisoBorderDesc,
                    &Samplers::SamplerPointBorderDesc,
                };
                embedded_sampler_cis.resize(NUM_INLINE_SMP);
                for (uint32_t i = 0; i < NUM_INLINE_SMP; ++i)
                    embedded_sampler_cis[i] = to_native_sampler_ci(*descs[i]);

                auto make_mapping = [](uint32_t set, uint32_t binding,
                                       VkSpirvResourceTypeFlagsEXT mask,
                                       VkDescriptorMappingSourceEXT source,
                                       VkDescriptorMappingSourceDataEXT data)
                {
                    VkDescriptorSetAndBindingMappingEXT m{
                        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT };
                    m.descriptorSet = set;
                    m.firstBinding  = binding;
                    m.bindingCount  = 1;
                    m.resourceMask  = mask;
                    m.source        = source;
                    m.sourceData    = data;
                    return m;
                };

                binding_mappings.clear();

                // set 0, binding 0 — resource heap: element index == descriptor slot,
                // uniform stride (D3D12 handle increment).
                {
                    VkDescriptorMappingSourceDataEXT d{};
                    d.constantOffset.heapOffset      = 0;
                    d.constantOffset.heapArrayStride = static_cast<uint32_t>(resource_descriptor_size);
                    binding_mappings.push_back(make_mapping(
                        0, 0, VK_SPIRV_RESOURCE_TYPE_ALL_EXT,
                        VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT, d));
                }

                // set 0, binding 2 — append/consume hidden counter (storage buffer),
                // indexed into the resource heap like binding 0.
                {
                    VkDescriptorMappingSourceDataEXT d{};
                    d.constantOffset.heapOffset      = 0;
                    d.constantOffset.heapArrayStride = static_cast<uint32_t>(resource_descriptor_size);
                    binding_mappings.push_back(make_mapping(
                        0, 2, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT,
                        VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT, d));
                }

                // set 0, bindings 384..388 — embedded static samplers s0..s4.
                // Fully baked into the pipeline via pEmbeddedSampler.
                for (uint32_t i = 0; i < NUM_INLINE_SMP; ++i)
                {
                    VkDescriptorMappingSourceDataEXT d{};
                    d.constantOffset.pEmbeddedSampler = &embedded_sampler_cis[i];
                    binding_mappings.push_back(make_mapping(
                        0, SMP_BASE + i, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT,
                        VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT, d));
                }

                // set 1, binding 0 — sampler heap: element index == sampler slot.
                {
                    VkDescriptorMappingSourceDataEXT d{};
                    d.constantOffset.samplerHeapOffset      = 0;
                    d.constantOffset.samplerHeapArrayStride = static_cast<uint32_t>(sampler_descriptor_size);
                    binding_mappings.push_back(make_mapping(
                        1, 0, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT,
                        VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT, d));
                }

                binding_mapping_info.sType =
                    VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT;
                binding_mapping_info.mappingCount = static_cast<uint32_t>(binding_mappings.size());
                binding_mapping_info.pMappings    = binding_mappings.data();
            }

            Log::get() << "Vulkan device: " << p.name.c_str()
                       << "  VRAM: " << (vram / 1024 / 1024) << " MB" << Log::endl;
        }

        Device::~Device()
        {
            // Embedded samplers are baked into pipelines; no VkSampler/layout objects
            // to destroy (VK_EXT_descriptor_heap owns no per-device descriptor objects).
            if (vma_allocator) vmaDestroyAllocator(vma_allocator);
            if (vk_device)     vkDestroyDevice(vk_device, nullptr);
            // Instance and debug messenger are owned by the static in HAL::init() /
            // HAL.Impl.cpp — do NOT destroy them here.
        }

        void Device::process_result(VkResult result, std::string_view line) const
        {
            if (result != VK_SUCCESS)
                Log::get().crash_error(static_cast<int>(result), line);
        }

        uint Device::get_descriptor_size(DescriptorHeapType) const { return 0; }
        VkDevice Device::get_native_device() const { return vk_device; }
        VkResult Device::get_device_removed_reason() const { return VK_SUCCESS; }

        uint Device::Subresources(const ResourceDesc& desc) const
        {
            if (desc.is_buffer()) return 1;
            auto t = desc.as_texture();
            return t.MipLevels * t.ArraySize;
        }

        size_t Device::get_vram()
        {
            VkPhysicalDeviceMemoryProperties mem_props{};
            vkGetPhysicalDeviceMemoryProperties(vk_physical, &mem_props);
            size_t total = 0;
            for (uint32_t i = 0; i < mem_props.memoryHeapCount; ++i)
                if (mem_props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                    total += mem_props.memoryHeaps[i].size;
            return total / 1024 / 1024;
        }

        size_t Device::get_upload_heap()
        {
            auto THIS = static_cast<HAL::Device*>(this);
            return THIS->get_heap_factory().get_upload_bytes() / 1024 / 1024;
        }

        size_t Device::get_readback_heap()
        {
            auto THIS = static_cast<HAL::Device*>(this);
            return THIS->get_heap_factory().get_readback_bytes() / 1024 / 1024;
        }

        ResourceAllocationInfo Device::get_alloc_info(const ResourceDesc& desc)
        {
            auto it = alloc_info.find(desc);
            if (it != alloc_info.end()) return it->second;

            ResourceAllocationInfo result{};

            if (desc.is_buffer())
            {
                VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
                bci.size  = desc.as_buffer().SizeInBytes;
                bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                          | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                          | VK_BUFFER_USAGE_INDEX_BUFFER_BIT   | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
                          | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

                VkMemoryRequirements2 req{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
                VkDeviceBufferMemoryRequirements info{ VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS };
                info.pCreateInfo = &bci;
                vkGetDeviceBufferMemoryRequirements(vk_device, &info, &req);

                result.size      = req.memoryRequirements.size;
                result.alignment = req.memoryRequirements.alignment;
                result.flags     = HeapFlags::BUFFERS_ONLY;
            }
            else if (desc.is_texture())
            {
                auto& t = desc.as_texture();
                VkImageCreateInfo ici{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
                ici.imageType   = t.is3D() ? VK_IMAGE_TYPE_3D :
                                  t.is1D() ? VK_IMAGE_TYPE_1D : VK_IMAGE_TYPE_2D;
                ici.format      = to_native(t.Format);
                ici.extent      = { t.Dimensions.x,
                                    t.is1D() ? 1u : t.Dimensions.y,
                                    t.is3D() ? t.Dimensions.z : 1u };
                // MipLevels=0 means "full chain" (resolved the same way in
                // Resource::init); vkGetDeviceImageMemoryRequirements requires >= 1.
                ici.mipLevels   = t.MipLevels;
                if (ici.mipLevels == 0)
                {
                    uint max_dim = std::max({ t.Dimensions.x,
                                              t.is1D() ? 1u : t.Dimensions.y,
                                              t.is3D() ? t.Dimensions.z : 1u });
                    ici.mipLevels = max_dim > 0u
                        ? static_cast<uint>(std::floor(std::log2(
                              static_cast<float>(max_dim)))) + 1u
                        : 1u;
                }
                ici.arrayLayers = t.ArraySize;
                ici.samples     = VK_SAMPLE_COUNT_1_BIT;
                // Keep in sync with Resource::init — requirements must be queried
                // for the same usage the image is actually created with.
                ici.usage       = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                if (check(desc.Flags & ResFlags::UnorderedAccess))
                    ici.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
                if (check(desc.Flags & ResFlags::RenderTarget))
                    ici.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                if (check(desc.Flags & ResFlags::DepthStencil))
                    ici.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

                VkMemoryRequirements2 req{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
                VkDeviceImageMemoryRequirements info{ VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS };
                info.pCreateInfo = &ici;
                vkGetDeviceImageMemoryRequirements(vk_device, &info, &req);

                result.size      = req.memoryRequirements.size;
                result.alignment = req.memoryRequirements.alignment;
                result.flags     = check(desc.Flags & (ResFlags::RenderTarget | ResFlags::DepthStencil))
                                 ? HeapFlags::RTDS_ONLY : HeapFlags::TEXTURES_ONLY;
            }
            else
            {
                result.size = 0; result.alignment = 256;
                result.flags = HeapFlags::NONE;
            }

            alloc_info[desc] = result;
            return result;
        }

        RaytracingPrebuildInfo Device::calculateBuffers(const RaytracingBuildDescBottomInputs&) { return {}; }
        RaytracingPrebuildInfo Device::calculateBuffers(const RaytracingBuildDescTopInputs&)    { return {}; }
    }
}
