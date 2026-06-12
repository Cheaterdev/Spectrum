export module HAL:Utils;

import stl.core;
import vulkan;
import Core;

import :Types;
import :Sampler;
using namespace HAL;

export namespace HAL
{
    // Backend identifier used to segregate per-backend caches (shaders, PSOs).
    // Textures are backend-agnostic and stay in the cache root.
    inline std::string get_backend_name() { return "vulkan"; }
}

// ============================================================================
// Compatibility stubs for D3D12 types used in common HAL files.
// In D3D12 builds, these come from d3d12.h/dxgi.h via the D3D12 Utils
// partition.  In Vulkan builds, we supply minimal-compatible definitions that
// share the same field names so common code compiles without any #ifdefs.
// ============================================================================

export
{
    // --- D3D12 descriptor handle stubs ---
    // Used by HAL::Handle::get_cpu() / get_gpu() in HAL.DescriptorHeap.ixx.
    struct D3D12_CPU_DESCRIPTOR_HANDLE { size_t ptr = 0; };
    struct D3D12_GPU_DESCRIPTOR_HANDLE { uint64_t ptr = 0; };

    // --- DXGI adapter description stub ---
    // Used by HAL::Device::create_singleton() in HAL.Device.cpp.
    struct DXGI_ADAPTER_DESC
    {
        wchar_t  Description[128] = {};
        unsigned VendorId            = 0;
        unsigned DeviceId            = 0;
        unsigned SubSysId            = 0;
        unsigned Revision            = 0;
        size_t   DedicatedVideoMemory   = 0;
        size_t   DedicatedSystemMemory  = 0;
        size_t   SharedSystemMemory     = 0;
    };

    // --- D3D12 dispatch / draw argument stubs ---
    // Used by IndirectCommand template machinery.
    struct D3D12_DISPATCH_ARGUMENTS
    {
        unsigned ThreadGroupCountX = 0;
        unsigned ThreadGroupCountY = 0;
        unsigned ThreadGroupCountZ = 0;
    };
    struct D3D12_DRAW_INDEXED_ARGUMENTS
    {
        unsigned IndexCountPerInstance  = 0;
        unsigned InstanceCount          = 0;
        unsigned StartIndexLocation     = 0;
        int      BaseVertexLocation     = 0;
        unsigned StartInstanceLocation  = 0;
    };
    // D3D12_DISPATCH_MESH_ARGUMENTS — used in IndirectCommand stubs
    struct D3D12_DISPATCH_MESH_ARGUMENTS
    {
        unsigned ThreadGroupCountX = 0;
        unsigned ThreadGroupCountY = 0;
        unsigned ThreadGroupCountZ = 0;
    };

    // --- D3D12_PROGRAM_IDENTIFIER stub (used by API::StateObject) ---
    struct D3D12_PROGRAM_IDENTIFIER
    {
        uint64_t OpaqueData[4] = {};
    };

    // --- D3D12CalcSubresource stub ---
    // Utility from d3dx12.h: computes a flat subresource index from
    // (MipSlice, ArraySlice, PlaneSlice, MipLevels, ArraySize).
    // Used in RenderSystem/Font/TextSystem.cpp.
    // TODO (REFACTOR_TODO §2): replace call-site with
    // TextureDesc::CalcSubresource() which already exists in HAL::TextureDesc.
    inline constexpr unsigned D3D12CalcSubresource(
        unsigned MipSlice, unsigned ArraySlice, unsigned PlaneSlice,
        unsigned MipLevels, unsigned ArraySize) noexcept
    {
        return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
    }

    // --- D3D_PRIMITIVE_TOPOLOGY stub ---
    // Used by RenderSystem (universal_material.ixx return type, Canvas.cpp,
    // FlowGraph) as a topology token.  Values match d3dcommon.h exactly so
    // that any serialized/cached topology tokens remain compatible.
    // TODO (REFACTOR_TODO §2): replace with HAL::PrimitiveTopologyType usage
    // throughout the RenderSystem.
    enum D3D_PRIMITIVE_TOPOLOGY : int
    {
        D3D_PRIMITIVE_TOPOLOGY_UNDEFINED                   =  0,
        D3D_PRIMITIVE_TOPOLOGY_POINTLIST                   =  1,
        D3D_PRIMITIVE_TOPOLOGY_LINELIST                    =  2,
        D3D_PRIMITIVE_TOPOLOGY_LINESTRIP                   =  3,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST                =  4,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP               =  5,
        D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ                = 10,
        D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ               = 11,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ            = 12,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ           = 13,
        D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST   = 33,
        D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST   = 34,
        D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST   = 35,
        D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST   = 36,
        D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST   = 37,
        D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST   = 38,
        D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST   = 39,
        D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST   = 40,
        D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST   = 41,
        D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST  = 42,
        D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST  = 64,
    };

    // --- Raytracing instance descriptor stub ---
    // Used by RenderSystem/Scene/Scene.ixx and MeshAsset as a gpu_buffer element
    // type.  The real struct is in d3d12.h; for Vulkan it maps to
    // VkAccelerationStructureInstanceKHR (same binary layout by design, but the
    // type name appears in template arguments so a stub is needed to compile).
    // TODO (REFACTOR_TODO §2): replace usages with HAL::InstanceDesc once the
    // RenderSystem is decoupled from D3D12 types.
    struct D3D12_RAYTRACING_INSTANCE_DESC
    {
        float    Transform[3][4]                      = {};
        unsigned InstanceID                           : 24 = 0;
        unsigned InstanceMask                         :  8 = 0;
        unsigned InstanceContributionToHitGroupIndex  : 24 = 0;
        unsigned Flags                                :  8 = 0;
        uint64_t AccelerationStructure                     = 0;
    };

    namespace cereal
    {
        // Empty serialization — matches the D3D12 Utils stub exactly.
        // Cereal requires a serialize() to be findable for any type it touches.
        template<class Archive>
        void serialize(Archive& /*ar*/, D3D12_RAYTRACING_INSTANCE_DESC& /*g*/) {}
    }

    // --- IndirectCommand / Work-Graph stubs ---
    // Used by DataHolder::create_indirect() and EntryPoints::compile() in
    // SIG/Slots.ixx.  The functions are never actually called in the Vulkan
    // backend (IndirectCommand is stubbed), but the types must exist for
    // the common template code to compile.

    enum D3D12_INDIRECT_ARGUMENT_TYPE : unsigned
    {
        D3D12_INDIRECT_ARGUMENT_TYPE_DRAW            = 0,
        D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED    = 1,
        D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH        = 2,
        D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT        = 5,
        D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH   = 10,
    };

    struct D3D12_INDIRECT_ARGUMENT_DESC
    {
        D3D12_INDIRECT_ARGUMENT_TYPE Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
        struct { unsigned RootParameterIndex = 0; unsigned DestOffsetIn32BitValues = 0; unsigned Num32BitValuesToSet = 0; } Constant;
    };

    // Work-graph node input types (D3D12 work-graphs, post-MVP on Vulkan).
    struct D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE
    {
        uint64_t StartAddress  = 0;
        uint64_t StrideInBytes = 0;
    };

    struct D3D12_NODE_GPU_INPUT
    {
        unsigned                          EntrypointIndex = 0;
        unsigned                          NumRecords      = 0;
        D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE Records;
    };

    struct D3D12_MULTI_NODE_GPU_INPUT
    {
        unsigned                          NumNodeInputs = 0;
        D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE NodeInputs;
    };

    // --- HANDLE stub (Win32 HANDLE used by API::Fence Event) ---
    // In Vulkan builds the Windows headers are still included (for HWND etc.),
    // so HANDLE is already defined.  No stub needed here.

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

    VkImageLayout        to_native(TextureLayout layout);
    VkPipelineStageFlags2 to_native_stage(BarrierSync sync);
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
    struct RaytracingDescNative {};
    RaytracingDescNative to_native(const RaytracingBuildDescBottomInputs& inputs);
    VkAccelerationStructureBuildGeometryInfoKHR
                         to_native(const RaytracingBuildDescTopInputs& inputs);

    // ResourceAddress → raw GPU virtual address.
    // Mirrors the global ::to_native(ResourceAddress) in the D3D12 Utils.
    // Defined in HAL.Vulkan.Resource.Buffer.cpp.
    GPUAddressPtr to_native(const HAL::ResourceAddress& address);

} // export
