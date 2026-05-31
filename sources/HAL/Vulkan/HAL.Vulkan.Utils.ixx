export module HAL:Utils;

import stl.core;
import vulkan;
import Core;

import :Types;
import :Sampler;
using namespace HAL;

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

    // ResourceDesc → VkBufferCreateInfo / VkImageCreateInfo helpers are
    // provided in HAL.Vulkan.Resource.ixx rather than here.

} // export
