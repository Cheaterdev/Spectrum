module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
module HAL:Device;

import :Debug;
import :Utils;
import :Impl;    // get_vk_instance()

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

    // ---- HAL::API::Device --------------------------------------------------

    namespace API
    {
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

            VkPhysicalDeviceFeatures2 features2{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
            features2.pNext = &di_features;
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
            p.mesh_shader   = false;  // Phase: VK_EXT_mesh_shader check
            p.work_graph    = false;  // no Vulkan equivalent yet
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
            // Binding offsets MUST match the DXC SPIR-V shift flags in
            // HAL.Vulkan.ShaderReflection.cpp:
            //   t-registers (SRV)  → binding = register + 0
            //   b-registers (CBV)  → binding = register + 128
            //   u-registers (UAV)  → binding = register + 256
            //   s-registers (SMP)  → set 1, binding = register (separate layout)
            {
                constexpr uint32_t SRV_BASE  = 0;    constexpr uint32_t SRV_COUNT  = 128;
                constexpr uint32_t CBV_BASE  = 128;  constexpr uint32_t CBV_COUNT  = 128;
                constexpr uint32_t UAV_BASE  = 256;  constexpr uint32_t UAV_COUNT  = 128;
                constexpr uint32_t SMP_BASE  = 384;  constexpr uint32_t SMP_COUNT  = 128;

                constexpr VkDescriptorBindingFlags bind_flags =
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

                // ---- Set 0 layout: SRV + CBV + UAV -------------------------
                // 4 bindings at offsets 0, 128, 256, 384 within the same set.
                struct BindDef { uint32_t binding; VkDescriptorType type; uint32_t count; };
                const BindDef defs[4] = {
                    { SRV_BASE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  SRV_COUNT },
                    { CBV_BASE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, CBV_COUNT },
                    { UAV_BASE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, UAV_COUNT },
                    { SMP_BASE, VK_DESCRIPTOR_TYPE_SAMPLER,        SMP_COUNT }, // inline sampler fallback
                };

                VkDescriptorSetLayoutBinding bindings[4]{};
                VkDescriptorBindingFlags     bflags[4]{};
                for (uint32_t bi = 0; bi < 4; ++bi)
                {
                    bindings[bi].binding         = defs[bi].binding;
                    bindings[bi].descriptorType  = defs[bi].type;
                    bindings[bi].descriptorCount = defs[bi].count;
                    bindings[bi].stageFlags      = VK_SHADER_STAGE_ALL;
                    bflags[bi]                   = bind_flags;
                }

                VkDescriptorSetLayoutBindingFlagsCreateInfo ext_info{
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
                ext_info.bindingCount  = 4;
                ext_info.pBindingFlags = bflags;

                VkDescriptorSetLayoutCreateInfo layout_ci{
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
                layout_ci.bindingCount = 4;
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

            Log::get() << "Vulkan device: " << p.name.c_str()
                       << "  VRAM: " << (vram / 1024 / 1024) << " MB" << Log::endl;
        }

        Device::~Device()
        {
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
                ici.mipLevels   = t.MipLevels;
                ici.arrayLayers = t.ArraySize;
                ici.samples     = VK_SAMPLE_COUNT_1_BIT;
                ici.usage       = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

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
