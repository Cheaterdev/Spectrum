export module HAL:Utils;

import stl.core;
import vulkan;
import Core;

import :Types;
import :Sampler;
using namespace HAL;

export namespace HAL
{
    // Backend identifier used to segregate per-backend caches (shaders, PSOs, textures).
    inline std::string get_backend_name() { return "vulkan"; }
}

export
{
    // ========================================================================
    // Vulkan backend namespace aliases (mirrors the D3D:: / DXGI:: aliases
    // the D3D12 Utils exports so that backend-specific code has a uniform
    // convention, though common code must not use these).
    // ========================================================================
    namespace VK
    {
        // Placeholder — populated by backend implementation files.
    }

    // ========================================================================
    // Conversion helpers: HAL abstract types → Vulkan native types.
    // The D3D12 Utils exports to_native() for every HAL enum.  We mirror the
    // same function names so any code (currently none in common files) that
    // calls to_native() still compiles.
    // ========================================================================

    VkFormat             to_native(Format format);
    Format               from_native(VkFormat format);

    // D24_UNORM_S8_UINT is optional in Vulkan (AMD has none). The device reports
    // it at init; without it, to_native() maps the D24S8 formats to
    // D32_SFLOAT_S8_UINT, which images, views and pipelines then agree on.
    void                 set_d24s8_supported(bool supported);

    VkImageLayout        to_native(TextureLayout layout);

    // Backend-native resource-state value as a plain uint32_t, mirroring
    // D3D12's to_native_resource_state (HAL.Utils.ixx).
    uint32_t             to_native_resource_state(TextureLayout layout);
    VkPipelineStageFlags2 to_native_stage(BarrierSync sync);
    void                  set_ray_tracing_pipeline_stage(bool available);   // device init
    VkAccessFlags2       to_native_access(BarrierAccess access);

    VkFilter             to_native_filter(Filter f);
    VkSamplerAddressMode to_native(TextureAddressMode mode);
    VkCompareOp          to_native(ComparisonFunc func);
    VkSamplerCreateInfo  to_native_sampler_ci(const SamplerDesc& desc);
    VkPrimitiveTopology  to_native_topology(PrimitiveTopologyType t);
    VkCullModeFlagBits   to_native(CullMode mode);
    VkPolygonMode        to_native(FillMode mode);
    VkBlendFactor        to_native(Blend b);
    VkStencilOp          to_native(StencilOp op);
    VkStencilOpState     to_native(StencilDesc desc);
    VkImageType          to_native(ResourceType t);

    VkCommandBufferLevel to_native(CommandListType type);
    VkQueueFlagBits      to_native_queue(CommandListType type);

    // Raytracing stubs — declared but not used in Vulkan for Phase 0.
    // A BLAS/TLAS build's inputs in Vulkan form: the geometries the build info
    // points at, and each one's primitive count (triangles, or instances for a
    // TLAS). info() points into `geometries`, so build it after any copy/move.
    struct AccelerationStructureInputs
    {
        VkAccelerationStructureTypeKHR                  type  = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        VkBuildAccelerationStructureFlagsKHR            flags = 0;
        std::vector<VkAccelerationStructureGeometryKHR> geometries;
        std::vector<uint32_t>                           primitive_counts;

        VkAccelerationStructureBuildGeometryInfoKHR info(bool update) const
        {
            VkAccelerationStructureBuildGeometryInfoKHR i{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
            i.type          = type;
            i.flags         = flags;
            i.mode          = update ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
            i.geometryCount = static_cast<uint32_t>(geometries.size());
            i.pGeometries   = geometries.data();
            return i;
        }
    };
    AccelerationStructureInputs to_native(const RaytracingBuildDescBottomInputs& inputs);
    AccelerationStructureInputs to_native(const RaytracingBuildDescTopInputs& inputs);

    // ResourceAddress → raw GPU virtual address.
    // Mirrors the global ::to_native(ResourceAddress) in the D3D12 Utils.
    // Defined in HAL.Vulkan.Resource.Buffer.cpp.
    GPUAddressPtr to_native(const HAL::ResourceAddress& address);

} // export
