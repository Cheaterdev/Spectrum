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

namespace
{
    // Set once the device exists; check_device_lost() can be reached from any
    // thread that submits or waits, without a Device reference.
    VkDevice                    fault_device      = VK_NULL_HANDLE;
    PFN_vkGetDeviceFaultInfoEXT fn_get_fault_info = nullptr;
    std::atomic_bool            fault_reported    = false;

    void log_device_fault(const char* where)
    {
        Log::get() << Log::LEVEL_ERROR << "[Vulkan] VK_ERROR_DEVICE_LOST detected at " << where << Log::endl;
        if (!fn_get_fault_info || fault_device == VK_NULL_HANDLE)
        {
            Log::get() << Log::LEVEL_ERROR << "[Vulkan] no VK_EXT_device_fault report available" << Log::endl;
            return;
        }

        VkDeviceFaultCountsEXT counts{ VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT };
        if (fn_get_fault_info(fault_device, &counts, nullptr) != VK_SUCCESS)
        {
            Log::get() << Log::LEVEL_ERROR << "[Vulkan] vkGetDeviceFaultInfoEXT (counts) failed" << Log::endl;
            return;
        }

        std::vector<VkDeviceFaultAddressInfoEXT> addresses(counts.addressInfoCount);
        std::vector<VkDeviceFaultVendorInfoEXT>  vendors(counts.vendorInfoCount);
        counts.vendorBinarySize = 0;   // the vendor crash-dump blob isn't decoded here

        VkDeviceFaultInfoEXT info{ VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT };
        info.pAddressInfos = addresses.data();
        info.pVendorInfos  = vendors.data();
        VkResult r = fn_get_fault_info(fault_device, &counts, &info);
        if (r != VK_SUCCESS && r != VK_INCOMPLETE)
        {
            Log::get() << Log::LEVEL_ERROR << "[Vulkan] vkGetDeviceFaultInfoEXT failed: " << static_cast<int>(r) << Log::endl;
            return;
        }

        Log::get() << Log::LEVEL_ERROR << "[Vulkan] device fault: " << info.description
                   << " (" << addresses.size() << " addresses, " << vendors.size() << " vendor records)" << Log::endl;
        for (auto& a : addresses)
        {
            // The fault lies within reportedAddress rounded down/up to addressPrecision.
            const uint64_t mask = a.addressPrecision ? ~(a.addressPrecision - 1) : ~0ull;
            Log::get() << Log::LEVEL_ERROR << "[Vulkan]   " << std::string(magic_enum::enum_name(a.addressType))
                       << " address 0x" << std::hex << a.reportedAddress
                       << " (range 0x" << (a.reportedAddress & mask) << " +0x" << a.addressPrecision << ")"
                       << std::dec << Log::endl;
        }
        for (auto& v : vendors)
            Log::get() << Log::LEVEL_ERROR << "[Vulkan]   vendor: " << v.description
                       << " code 0x" << std::hex << v.vendorFaultCode
                       << " data 0x" << v.vendorFaultData << std::dec << Log::endl;
    }
}

namespace HAL::API
{
    bool check_device_lost(VkResult result, const char* where)
    {
        if (result != VK_ERROR_DEVICE_LOST) return false;
        if (!fault_reported.exchange(true))
            log_device_fault(where);
        return true;
    }
}

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

    texture_range_layout Device::get_texture_range_layout(const ResourceDesc& rdesc, UINT first_subresource, UINT count)
    {
        // No GPU-decompression placement constraint on this backend (Vulkan's
        // compress() is a no-op -- see below), so subresources are packed
        // contiguously here rather than mirroring D3D12's GetCopyableFootprints1
        // alignment. Save and load both go through this same function, so the
        // exact packing scheme only has to be self-consistent, not bit-exact
        // with the other backend.
        texture_range_layout result;
        result.subresource_offsets.resize(count);
        uint64 offset = 0;
        for (UINT i = 0; i < count; i++)
        {
            result.subresource_offsets[i] = offset;
            offset += Math::AlignUp(get_texture_layout(rdesc, first_subresource + i).size, 256ull);
        }
        result.total_size = offset;
        return result;
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

            // Store for Queue::construct() and CommandAllocator, indexed by
            // HAL::CommandListType. The async compute queues (COMPUTE2/3) take
            // further queues of the compute family.
            const uint32_t type_families[queue_type_count] = {
                graphics_family,   // DIRECT
                compute_family,    // COMPUTE
                transfer_family,   // COPY
                compute_family,    // COMPUTE2
                compute_family,    // COMPUTE3
            };

            std::map<uint32_t, uint32_t> family_queue_count;
            for (uint32_t t = 0; t < queue_type_count; ++t)
            {
                const uint32_t family = type_families[t];
                uint32_t& used        = family_queue_count[family];

                queue_families[t] = family;
                queue_indices[t]  = used < qf_props[family].queueCount ? used++ : used - 1;

                for (uint32_t other = 0; other < t; ++other)
                    if (queue_families[other] == family && queue_indices[other] == queue_indices[t])
                    {
                        queue_mutex_slot[t] = queue_mutex_slot[other];
                        break;
                    }
            }

            // ---- Queue create infos ----------------------------------------
            const std::vector<float> priorities(queue_type_count, 1.0f);
            std::vector<VkDeviceQueueCreateInfo> queue_infos;
            for (auto [family, count] : family_queue_count)
            {
                VkDeviceQueueCreateInfo qi{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
                qi.queueFamilyIndex = family;
                qi.queueCount       = count;
                qi.pQueuePriorities = priorities.data();
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
                // Required: DXC maps every ResourceDescriptorHeap[i] access, whatever
                // its type, onto one binding -- only a mutable binding can hold that.
                VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
                // Optional: VK_FORMAT_A8_UNORM (glyph atlases).
                VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
                // Optional: allows vkCmdCopyImage between depth (D32) and color (R32)
                // images — needed to build the color Hi-Z pyramid from the depth GBuffer.
                VK_KHR_MAINTENANCE_8_EXTENSION_NAME,
                // Optional: faulting addresses/vendor info after VK_ERROR_DEVICE_LOST.
                VK_EXT_DEVICE_FAULT_EXTENSION_NAME,
                // nullDescriptor: D3D12-style null views for unbound slots.
                VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
                // TEMP: device-lost investigation -- GPU address bind/unbind reports.
                VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME,
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

            if (!has_ext(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME))
            {
                Log::get().crash_error(
                    "[Vulkan] VK_EXT_mutable_descriptor_type is required but not supported "
                    "by this GPU/driver.");
                return;
            }

            // ---- Feature chain ----------------------------------------------
            // vkGetPhysicalDeviceFeatures2 below overwrites every struct in this
            // chain with what the device supports, and that same chain is what
            // vkCreateDevice enables -- so everything supported gets enabled and
            // the required features are checked after the query.
            // All Vulkan 1.2 promoted features must live in a single
            // VkPhysicalDeviceVulkan12Features — the individual VkPhysicalDevice*Features
            // structs for these cannot coexist with it in the same pNext chain.
            VkPhysicalDeviceVulkan12Features vk12_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };

            VkPhysicalDeviceSynchronization2Features sync2_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES };
            sync2_features.synchronization2 = VK_TRUE;
            sync2_features.pNext = &vk12_features;

            VkPhysicalDeviceDynamicRenderingFeatures dr_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES };
            dr_features.dynamicRendering = VK_TRUE;
            dr_features.pNext = &sync2_features;

            VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutable_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT };
            mutable_features.pNext = &dr_features;

            // DemoteToHelperInvocation: 'discard' in pixel shaders uses this capability.
            VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures demote_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES };
            demote_features.pNext = &mutable_features;

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

            VkPhysicalDeviceMaintenance5Features maint5_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES };

            void* feature_head = &eds_features;
            if (has_ext(VK_KHR_MAINTENANCE_5_EXTENSION_NAME)) {
                maint5_features.pNext = feature_head;
                feature_head = &maint5_features;
            }
            if (has_cs_derivatives) {
                cs_deriv_features.pNext = feature_head;
                feature_head = &cs_deriv_features;
            }
            if (has_mesh_shader) {
                mesh_features.pNext = feature_head;
                feature_head = &mesh_features;
            }
            VkPhysicalDeviceFaultFeaturesEXT fault_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT };
            const bool has_device_fault = has_ext(VK_EXT_DEVICE_FAULT_EXTENSION_NAME);
            if (has_device_fault) {
                fault_features.pNext = feature_head;
                feature_head = &fault_features;
            }
            VkPhysicalDeviceRobustness2FeaturesEXT robustness2_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT };
            const bool has_robustness2 = has_ext(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
            if (has_robustness2) {
                robustness2_features.pNext = feature_head;
                feature_head = &robustness2_features;
            }

            // TEMP: device-lost investigation.
            VkPhysicalDeviceAddressBindingReportFeaturesEXT binding_report_features{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT };
            if (has_ext(VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME)) {
                binding_report_features.pNext = feature_head;
                feature_head = &binding_report_features;
            }
            if (has_maintenance8) {
                maint8_features.pNext = feature_head;
                feature_head = &maint8_features;
            }

            VkPhysicalDeviceFeatures2 features2{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
            features2.pNext = feature_head;
            vkGetPhysicalDeviceFeatures2(vk_physical, &features2);

            // Their parent features (multiview, primitiveFragmentShadingRate) are not
            // in the chain, and enabling a dependent feature without its parent is invalid.
            mesh_features.multiviewMeshShader                    = VK_FALSE;
            mesh_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;

            // Only nullDescriptor is wanted from robustness2; robust buffer/image
            // access would add bounds checks to every access.
            robustness2_features.robustBufferAccess2 = VK_FALSE;
            robustness2_features.robustImageAccess2  = VK_FALSE;
            null_descriptor = has_robustness2 && robustness2_features.nullDescriptor;
            if (!null_descriptor)
                Log::get() << Log::LEVEL_WARNING << "[Vulkan] nullDescriptor unsupported: unbound descriptor "
                              "slots stay unwritten (reading them is undefined)" << Log::endl;

            {
                const std::pair<VkBool32, const char*> required[] = {
                    { mutable_features.mutableDescriptorType,                       "mutableDescriptorType" },
                    { vk12_features.runtimeDescriptorArray,                         "runtimeDescriptorArray" },
                    { vk12_features.descriptorBindingPartiallyBound,                "descriptorBindingPartiallyBound" },
                    { vk12_features.descriptorBindingSampledImageUpdateAfterBind,   "descriptorBindingSampledImageUpdateAfterBind" },
                    { vk12_features.descriptorBindingStorageImageUpdateAfterBind,   "descriptorBindingStorageImageUpdateAfterBind" },
                    { vk12_features.descriptorBindingStorageBufferUpdateAfterBind,  "descriptorBindingStorageBufferUpdateAfterBind" },
                    { vk12_features.descriptorBindingUniformBufferUpdateAfterBind,  "descriptorBindingUniformBufferUpdateAfterBind" },
                    { vk12_features.shaderSampledImageArrayNonUniformIndexing,      "shaderSampledImageArrayNonUniformIndexing" },
                    { vk12_features.shaderStorageImageArrayNonUniformIndexing,      "shaderStorageImageArrayNonUniformIndexing" },
                    { vk12_features.shaderStorageBufferArrayNonUniformIndexing,     "shaderStorageBufferArrayNonUniformIndexing" },
                    { vk12_features.shaderUniformBufferArrayNonUniformIndexing,     "shaderUniformBufferArrayNonUniformIndexing" },
                };
                for (auto& [supported, name] : required)
                    if (!supported)
                    {
                        Log::get().crash_error(std::string("[Vulkan] required feature not supported: ") + name);
                        return;
                    }
            }

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

            fault_device = vk_device;
            if (has_device_fault && fault_features.deviceFault)
                fn_get_fault_info = reinterpret_cast<PFN_vkGetDeviceFaultInfoEXT>(
                    vkGetDeviceProcAddr(vk_device, "vkGetDeviceFaultInfoEXT"));
            Log::get() << "[Vulkan] device fault reporting: " << (fn_get_fault_info ? "on" : "unavailable") << Log::endl;

            // ---- DeviceProperties -------------------------------------------
            VkPhysicalDeviceVulkan12Properties vk12_props{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES };
            VkPhysicalDeviceProperties2 props2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
            props2.pNext = &vk12_props;
            vkGetPhysicalDeviceProperties2(vk_physical, &props2);

            auto& p = THIS->properties;
            p.name = props2.properties.deviceName;
            p.rtx           = false;  // Phase: VK_KHR_ray_tracing_pipeline check
            p.mesh_shader   = has_mesh_shader && (mesh_features.meshShader == VK_TRUE);
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

            // ---- Bindless descriptor layouts -------------------------------
            // Must match the DXC flags in HAL.Vulkan.ShaderReflection.cpp:
            //   set 0, binding 0        : ResourceDescriptorHeap -- MUTABLE[N]; every
            //                             CBV/SRV/UAV type aliases this one binding
            //   set 0, binding 2        : counter.var.ResourceDescriptorHeap -- the
            //                             Append/Consume counter of slot i is element i
            //   set 0, bindings 384..390: static samplers s0..s6 (s-shift 384)
            //   set 1, binding 0        : SamplerDescriptorHeap
            //   push constants          : one uint per SIG slot, offset = slot * 4
            {
                // DescriptorHeapFactory's shader-visible heap sizes; heaps clamp to
                // the capacities computed from them below.
                constexpr uint64_t REQUESTED_RESOURCES = 65536 * 8;
                constexpr uint64_t REQUESTED_SAMPLERS  = 2048;
                constexpr uint32_t SMP_BASE            = 384;

                // A mutable descriptor counts against the limit of every type in its
                // list, and the counter binding adds another N storage buffers.
                const auto& L = vk12_props;
                uint64_t cap = REQUESTED_RESOURCES;
                for (uint64_t limit : {
                         uint64_t(L.maxDescriptorSetUpdateAfterBindSampledImages),
                         uint64_t(L.maxDescriptorSetUpdateAfterBindStorageImages),
                         uint64_t(L.maxDescriptorSetUpdateAfterBindUniformBuffers),
                         uint64_t(L.maxDescriptorSetUpdateAfterBindStorageBuffers) / 2,
                         uint64_t(L.maxPerStageDescriptorUpdateAfterBindSampledImages),
                         uint64_t(L.maxPerStageDescriptorUpdateAfterBindStorageImages),
                         uint64_t(L.maxPerStageDescriptorUpdateAfterBindUniformBuffers),
                         uint64_t(L.maxPerStageDescriptorUpdateAfterBindStorageBuffers) / 2,
                         uint64_t(L.maxPerStageUpdateAfterBindResources) / 5,
                         uint64_t(L.maxUpdateAfterBindDescriptorsInAllPools) / 2 })
                    cap = std::min(cap, limit);
                resource_heap_capacity = static_cast<uint32_t>(cap);
                sampler_heap_capacity  = static_cast<uint32_t>(std::min<uint64_t>({ REQUESTED_SAMPLERS,
                    L.maxDescriptorSetUpdateAfterBindSamplers - NUM_INLINE_SMP,
                    L.maxPerStageDescriptorUpdateAfterBindSamplers - NUM_INLINE_SMP }));
                if (cap < REQUESTED_RESOURCES)
                    Log::get() << Log::LEVEL_WARNING << "[Vulkan] resource descriptor heap limited to "
                               << resource_heap_capacity << " of " << REQUESTED_RESOURCES << " by device limits" << Log::endl;

                // Static samplers, in Frame::FrameLayout order (register sN = binding
                // SMP_BASE + N): linearWrap, pointClamp, linearClamp, anisoBorder,
                // pointBorder, vsmShadow (comparison), linearBorderBlack.
                const SamplerDesc* descs[NUM_INLINE_SMP] = {
                    &Samplers::SamplerLinearWrapDesc,
                    &Samplers::SamplerPointClampDesc,
                    &Samplers::SamplerLinearClampDesc,
                    &Samplers::SamplerAnisoBorderDesc,
                    &Samplers::SamplerPointBorderDesc,
                    &Samplers::SamplerShadowComparisonDesc,
                    &Samplers::SamplerLinearBorderBlackDesc,
                };
                for (uint32_t i = 0; i < NUM_INLINE_SMP; ++i)
                {
                    auto ci = to_native_sampler_ci(*descs[i]);
                    vkCreateSampler(vk_device, &ci, nullptr, &inline_samplers[i]);
                }

                constexpr VkDescriptorBindingFlags bindless_flags =
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

                // ---- Set 0 ----
                constexpr uint32_t SET0_BINDINGS = 2 + NUM_INLINE_SMP;
                VkDescriptorSetLayoutBinding bindings[SET0_BINDINGS]{};
                VkDescriptorBindingFlags     flags[SET0_BINDINGS]{};
                VkMutableDescriptorTypeListEXT mutable_lists[SET0_BINDINGS]{};

                bindings[0] = { 0, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, resource_heap_capacity, VK_SHADER_STAGE_ALL, nullptr };
                flags[0]    = bindless_flags;
                mutable_lists[0].descriptorTypeCount = static_cast<uint32_t>(std::size(mutable_resource_types));
                mutable_lists[0].pDescriptorTypes    = mutable_resource_types;

                bindings[1] = { 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resource_heap_capacity, VK_SHADER_STAGE_ALL, nullptr };
                flags[1]    = bindless_flags;

                for (uint32_t i = 0; i < NUM_INLINE_SMP; ++i)
                    bindings[2 + i] = { SMP_BASE + i, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, &inline_samplers[i] };

                VkMutableDescriptorTypeCreateInfoEXT mutable_ci{ VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT };
                mutable_ci.mutableDescriptorTypeListCount = SET0_BINDINGS;
                mutable_ci.pMutableDescriptorTypeLists    = mutable_lists;

                VkDescriptorSetLayoutBindingFlagsCreateInfo flags_ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
                flags_ci.bindingCount  = SET0_BINDINGS;
                flags_ci.pBindingFlags = flags;
                flags_ci.pNext         = &mutable_ci;

                VkDescriptorSetLayoutCreateInfo set0_ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
                set0_ci.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
                set0_ci.bindingCount = SET0_BINDINGS;
                set0_ci.pBindings    = bindings;
                set0_ci.pNext        = &flags_ci;
                process_result(vkCreateDescriptorSetLayout(vk_device, &set0_ci, nullptr, &resource_set_layout), "resource set layout");

                // ---- Set 1 ----
                VkDescriptorSetLayoutBinding sampler_binding{ 0, VK_DESCRIPTOR_TYPE_SAMPLER, sampler_heap_capacity, VK_SHADER_STAGE_ALL, nullptr };
                VkDescriptorSetLayoutBindingFlagsCreateInfo sampler_flags_ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
                sampler_flags_ci.bindingCount  = 1;
                sampler_flags_ci.pBindingFlags = &bindless_flags;

                VkDescriptorSetLayoutCreateInfo set1_ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
                set1_ci.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
                set1_ci.bindingCount = 1;
                set1_ci.pBindings    = &sampler_binding;
                set1_ci.pNext        = &sampler_flags_ci;
                process_result(vkCreateDescriptorSetLayout(vk_device, &set1_ci, nullptr, &sampler_set_layout), "sampler set layout");

                // ---- Pipeline layout ----
                VkDescriptorSetLayout set_layouts[] = { resource_set_layout, sampler_set_layout };
                VkPushConstantRange push_range{ VK_SHADER_STAGE_ALL, 0, PUSH_CONSTANT_SIZE };

                VkPipelineLayoutCreateInfo pl_ci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
                pl_ci.setLayoutCount         = static_cast<uint32_t>(std::size(set_layouts));
                pl_ci.pSetLayouts            = set_layouts;
                pl_ci.pushConstantRangeCount = 1;
                pl_ci.pPushConstantRanges    = &push_range;
                process_result(vkCreatePipelineLayout(vk_device, &pl_ci, nullptr, &pipeline_layout), "pipeline layout");

                Log::get() << "[Vulkan] bindless heap: " << resource_heap_capacity << " resources, "
                           << sampler_heap_capacity << " samplers" << Log::endl;
            }

            Log::get() << "Vulkan device: " << p.name.c_str()
                       << "  VRAM: " << (vram / 1024 / 1024) << " MB" << Log::endl;
        }

        Device::~Device()
        {
            if (pipeline_layout)     vkDestroyPipelineLayout(vk_device, pipeline_layout, nullptr);
            if (resource_set_layout) vkDestroyDescriptorSetLayout(vk_device, resource_set_layout, nullptr);
            if (sampler_set_layout)  vkDestroyDescriptorSetLayout(vk_device, sampler_set_layout, nullptr);
            for (VkSampler s : inline_samplers)
                if (s) vkDestroySampler(vk_device, s, nullptr);
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
