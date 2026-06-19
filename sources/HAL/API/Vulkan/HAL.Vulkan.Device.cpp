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

// Vulkan native implementation of HAL::Device.
// Mirrors the partition layout of D3D12/HAL.D3D12.Device.cpp.

namespace HAL
{
    // ---- Common HAL::Device methods ----------------------------------------

    texture_layout Device::get_texture_layout(const ResourceDesc& rdesc, UINT sub_resource)
    {
        auto& desc = rdesc.as_texture();
        auto info = desc.Format.surface_info({ desc.Dimensions.x, desc.Dimensions.y });
        return {
            info.numBytes, info.numRows, info.rowBytes,
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
                // Required for ResourceDescriptorHeap: binding 0 must accept multiple
                // descriptor types (SAMPLED_IMAGE, STORAGE_IMAGE, UNIFORM_BUFFER,
                // STORAGE_BUFFER) simultaneously — exactly what mutable descriptors do.
                VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
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

            // ---- Feature chain ----------------------------------------------
            VkPhysicalDeviceBufferDeviceAddressFeatures bda_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
            bda_features.bufferDeviceAddress = VK_TRUE;

            VkPhysicalDeviceTimelineSemaphoreFeatures ts_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES };
            ts_features.timelineSemaphore = VK_TRUE;
            ts_features.pNext = &bda_features;

            VkPhysicalDeviceSynchronization2Features sync2_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES };
            sync2_features.synchronization2 = VK_TRUE;
            sync2_features.pNext = &ts_features;

            VkPhysicalDeviceDynamicRenderingFeatures dr_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES };
            dr_features.dynamicRendering = VK_TRUE;
            dr_features.pNext = &sync2_features;

            VkPhysicalDeviceDescriptorIndexingFeatures di_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
            di_features.runtimeDescriptorArray                    = VK_TRUE;
            di_features.descriptorBindingPartiallyBound           = VK_TRUE;
            di_features.descriptorBindingVariableDescriptorCount  = VK_TRUE;
            di_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
            di_features.pNext = &dr_features;

            // Mutable descriptors: binding 0 can hold SAMPLED_IMAGE, STORAGE_IMAGE,
            // UNIFORM_BUFFER, STORAGE_BUFFER simultaneously (needed for ResourceDescriptorHeap).
            VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutable_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT };
            mutable_features.mutableDescriptorType = VK_TRUE;
            mutable_features.pNext = &di_features;

            // DemoteToHelperInvocation: 'discard' in pixel shaders uses this capability.
            VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures demote_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES };
            demote_features.shaderDemoteToHelperInvocation = VK_TRUE;
            demote_features.pNext = &mutable_features;

            // scalarBlockLayout: allows StructuredBuffers with strides not aligned to 16
            // (e.g. Glyph stride=28, VSLine stride=24).
            VkPhysicalDeviceScalarBlockLayoutFeatures scalar_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES };
            scalar_features.scalarBlockLayout = VK_TRUE;
            scalar_features.pNext = &demote_features;

            // hostQueryReset: allows vkResetQueryPool() from the CPU without a command buffer.
            // Used in QueryHeap constructor to bring queries to the "unavailable" initial state.
            VkPhysicalDeviceHostQueryResetFeatures host_reset_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES };
            host_reset_features.hostQueryReset = VK_TRUE;
            host_reset_features.pNext = &scalar_features;

            // extendedDynamicState: required for vkCmdSetPrimitiveTopology (used by
            // set_topology / reapply_draw_state). Promoted to Vulkan 1.3 core but still
            // must be explicitly enabled in the device feature chain.
            VkPhysicalDeviceExtendedDynamicStateFeaturesEXT eds_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
            eds_features.extendedDynamicState = VK_TRUE;
            eds_features.pNext = &host_reset_features;

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

            void* feature_head = &eds_features;
            if (has_cs_derivatives) {
                cs_deriv_features.pNext = feature_head;
                feature_head = &cs_deriv_features;
            }
            if (has_mesh_shader) {
                mesh_features.pNext = feature_head;
                feature_head = &mesh_features;
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

            // ---- DeviceProperties -------------------------------------------
            VkPhysicalDeviceProperties2 props2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
            vkGetPhysicalDeviceProperties2(vk_physical, &props2);

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

            // ---- Global bindless descriptor set layouts ---------------------
            // DXC with -fvk-bind-resource-heap 0 0 maps the ENTIRE ResourceDescriptorHeap
            // to Set 0, Binding 0 — regardless of resource type (SAMPLED_IMAGE,
            // STORAGE_IMAGE, UNIFORM_BUFFER, STORAGE_BUFFER all land at binding 0).
            // The b/t/u shifts (0/128/256) partition the element INDEX space:
            //   b-registers (CBV)  → binding 0, elements [0,   127]  (b-shift = 0)
            //   t-registers (SRV)  → binding 0, elements [128, 255]  (t-shift = 128)
            //   u-registers (UAV)  → binding 0, elements [256, 383]  (u-shift = 256)
            // Binding 384 = SAMPLER for inline s-register declarations (s-shift = 384).
            //   s-registers (SMP)  → set 0, binding 384+register#    (s-shift = 384)
            // SamplerDescriptorHeap → set 1, binding 0                (-fvk-bind-sampler-heap 1 0)
            //
            // Because binding 0 holds multiple descriptor types simultaneously,
            // we use VK_EXT_mutable_descriptor_type (VK_DESCRIPTOR_TYPE_MUTABLE_EXT).
            {
                constexpr uint32_t HEAP_MAX  = 65536 * 8; // matches DescriptorHeapFactory
                constexpr uint32_t SMP_BASE  = 384;       // inline sampler binding (s-shift)
                constexpr uint32_t SMP_COUNT = 128;       // max inline samplers

                constexpr VkDescriptorBindingFlags bind_flags =
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

                // Mutable descriptor type list for binding 0.
                const VkDescriptorType mutable_types[] = {
                    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                };
                VkMutableDescriptorTypeListEXT mutable_list{};
                mutable_list.descriptorTypeCount = 4;
                mutable_list.pDescriptorTypes    = mutable_types;

                // Bindings: 0 = mutable heap; 384..388 = inline static samplers s0..s4.
                // Each s-register (s0..s4) maps to its own binding: sN → binding SMP_BASE+N.
                // (With s-shift=384: s2 → binding 386, NOT element 2 of binding 384.)
                constexpr uint32_t NUM_INLINE_SMP = 5; // s0..s4 in FrameLayout.h
                // mutable heap (0) + counter heap (2) + 5 inline samplers (384-388)
                constexpr uint32_t TOTAL_BINDINGS = 2 + NUM_INLINE_SMP;

                VkDescriptorSetLayoutBinding bindings[TOTAL_BINDINGS]{};
                VkDescriptorBindingFlags     bflags[TOTAL_BINDINGS]{};

                // Binding 0: mutable resource heap (CBV/SRV/UAV via ResourceDescriptorHeap)
                bindings[0].binding         = 0;
                bindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
                bindings[0].descriptorCount = HEAP_MAX;
                bindings[0].stageFlags      = VK_SHADER_STAGE_ALL;
                bflags[0]                   = bind_flags;

                // Binding 2: counter heap — DXC SPIR-V places AppendStructuredBuffer
                // hidden counters here by default (counter.var.ResourceDescriptorHeap).
                bindings[1].binding         = 2;
                bindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[1].descriptorCount = 1;
                bindings[1].stageFlags      = VK_SHADER_STAGE_ALL;
                bflags[1]                   = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

                for (uint32_t si = 0; si < NUM_INLINE_SMP; ++si)
                {
                    bindings[2 + si].binding         = SMP_BASE + si; // 384, 385, 386, 387, 388
                    bindings[2 + si].descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
                    bindings[2 + si].descriptorCount = 1;
                    bindings[2 + si].stageFlags      = VK_SHADER_STAGE_ALL;
                    bflags[2 + si]                   = bind_flags;
                }

                // Mutable type list: one entry per binding.
                // Binding 0 = mutable; binding 2 (counter) and 384-388 (samplers) = not mutable.
                VkMutableDescriptorTypeListEXT mutable_lists[TOTAL_BINDINGS]{};
                mutable_lists[0] = mutable_list; // binding 0 only
                // mutable_lists[1..6] remain zero-initialised

                VkMutableDescriptorTypeCreateInfoEXT mutable_ci{
                    VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT };
                mutable_ci.mutableDescriptorTypeListCount = TOTAL_BINDINGS;
                mutable_ci.pMutableDescriptorTypeLists    = mutable_lists;

                VkDescriptorSetLayoutBindingFlagsCreateInfo ext_info{
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
                ext_info.bindingCount  = TOTAL_BINDINGS;
                ext_info.pBindingFlags = bflags;

                mutable_ci.pNext = nullptr;
                ext_info.pNext   = &mutable_ci;

                VkDescriptorSetLayoutCreateInfo layout_ci{
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
                layout_ci.bindingCount = TOTAL_BINDINGS;
                layout_ci.pBindings    = bindings;
                layout_ci.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
                layout_ci.pNext        = &ext_info;

                vkCreateDescriptorSetLayout(vk_device, &layout_ci, nullptr, &cbv_srv_uav_layout);

                // ---- Set 1 layout: Sampler (dedicated) ----------------------
                VkDescriptorSetLayoutBinding samp_binding{};
                samp_binding.binding         = 0;
                samp_binding.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
                samp_binding.descriptorCount = 2048;
                samp_binding.stageFlags      = VK_SHADER_STAGE_ALL;

                VkDescriptorBindingFlags samp_flag = bind_flags;

                VkDescriptorSetLayoutBindingFlagsCreateInfo samp_ext{
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
                samp_ext.bindingCount  = 1;
                samp_ext.pBindingFlags = &samp_flag;

                VkDescriptorSetLayoutCreateInfo samp_ci{
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
                samp_ci.bindingCount = 1;
                samp_ci.pBindings    = &samp_binding;
                samp_ci.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
                samp_ci.pNext        = &samp_ext;

                vkCreateDescriptorSetLayout(vk_device, &samp_ci, nullptr, &sampler_layout);
            }

            // ---- Inline static samplers s0..s4 (FrameLayout.h) --------------
            // These map to set 0, bindings 384-388 (s-shift = 384).
            // The order must match FrameLayout.h:
            //   s0=linearSampler, s1=pointClampSampler, s2=linearClampSampler,
            //   s3=anisoBordeSampler, s4=pointBorderSampler
            {
                const SamplerDesc* descs[NUM_INLINE_SMP] = {
                    &Samplers::SamplerLinearWrapDesc,
                    &Samplers::SamplerPointClampDesc,
                    &Samplers::SamplerLinearClampDesc,
                    &Samplers::SamplerAnisoBorderDesc,
                    &Samplers::SamplerPointBorderDesc,
                };
                for (uint32_t i = 0; i < NUM_INLINE_SMP; ++i)
                {
                    auto ci = to_native_sampler_ci(*descs[i]);
                    vkCreateSampler(vk_device, &ci, nullptr, &inline_samplers[i]);
                }
            }

            Log::get() << "Vulkan device: " << p.name.c_str()
                       << "  VRAM: " << (vram / 1024 / 1024) << " MB" << Log::endl;
        }

        Device::~Device()
        {
            for (uint32_t i = 0; i < NUM_INLINE_SMP; ++i)
                if (inline_samplers[i]) vkDestroySampler(vk_device, inline_samplers[i], nullptr);
            if (cbv_srv_uav_layout) vkDestroyDescriptorSetLayout(vk_device, cbv_srv_uav_layout, nullptr);
            if (sampler_layout)     vkDestroyDescriptorSetLayout(vk_device, sampler_layout, nullptr);
            if (vma_allocator)      vmaDestroyAllocator(vma_allocator);
            if (vk_device)          vkDestroyDevice(vk_device, nullptr);
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
