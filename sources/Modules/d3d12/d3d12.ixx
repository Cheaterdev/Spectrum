module;

#include <directx/d3d12.h>
#include <directx/d3d12shader.h>
#include <directx/d3dx12.h>
#include <directx/dxgiformat.h>
#include <dxgi1_6.h>
#include <DirectXTex.h>
#include <dstorage.h>
#include <dxgidebug.h>
 #include <wincodec.h>
export module d3d12;

export
{
    // ---- D3D12 structs -------------------------------------------------------
    using D3D12_RESOURCE_DESC1                              = ::D3D12_RESOURCE_DESC1;
    using D3D12_INDIRECT_ARGUMENT_DESC                      = ::D3D12_INDIRECT_ARGUMENT_DESC;
    using D3D12_COMMAND_SIGNATURE_DESC                      = ::D3D12_COMMAND_SIGNATURE_DESC;
    using D3D12_NODE_GPU_INPUT                              = ::D3D12_NODE_GPU_INPUT;
    using D3D12_MULTI_NODE_GPU_INPUT                        = ::D3D12_MULTI_NODE_GPU_INPUT;
    using D3D12_AUTO_BREADCRUMB_NODE                        = ::D3D12_AUTO_BREADCRUMB_NODE;
    using D3D12_AUTO_BREADCRUMB_OP                          = ::D3D12_AUTO_BREADCRUMB_OP;
    using D3D12_SAMPLER_DESC                                = ::D3D12_SAMPLER_DESC;
    using D3D12_DEPTH_STENCILOP_DESC                        = ::D3D12_DEPTH_STENCILOP_DESC;
    using D3D12_VIEWPORT                                    = ::D3D12_VIEWPORT;
    using D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS = ::D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS;
    using D3D12_RAYTRACING_GEOMETRY_DESC                    = ::D3D12_RAYTRACING_GEOMETRY_DESC;
    using D3D12_RAYTRACING_INSTANCE_DESC                    = ::D3D12_RAYTRACING_INSTANCE_DESC;
    using D3D12_PROGRAM_IDENTIFIER                          = ::D3D12_PROGRAM_IDENTIFIER;
    using D3D12_PLACED_SUBRESOURCE_FOOTPRINT                  = ::D3D12_PLACED_SUBRESOURCE_FOOTPRINT;
    using D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT                  = ::D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT;
    using D3D12_DRED_PAGE_FAULT_OUTPUT                        = ::D3D12_DRED_PAGE_FAULT_OUTPUT;
    using D3D12_STATIC_SAMPLER_DESC                           = ::D3D12_STATIC_SAMPLER_DESC;
    using D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO = ::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO;
    using D3D12_INFO_QUEUE_FILTER                             = ::D3D12_INFO_QUEUE_FILTER;
    using D3D12_SHADER_RESOURCE_VIEW_DESC                     = ::D3D12_SHADER_RESOURCE_VIEW_DESC;
    using D3D12_CLEAR_VALUE                                   = ::D3D12_CLEAR_VALUE;
    using D3D12_SHADER_BYTECODE                               = ::D3D12_SHADER_BYTECODE;
    using D3D12_RASTERIZER_DESC                               = ::D3D12_RASTERIZER_DESC;
    using D3D12_BLEND_DESC                                    = ::D3D12_BLEND_DESC;
    using D3D12_DEPTH_STENCIL_DESC                            = ::D3D12_DEPTH_STENCIL_DESC;
    using D3D12_RT_FORMAT_ARRAY                               = ::D3D12_RT_FORMAT_ARRAY;
    using D3D12_CACHED_PIPELINE_STATE                         = ::D3D12_CACHED_PIPELINE_STATE;
    using D3D12_PIPELINE_STATE_STREAM_DESC                    = ::D3D12_PIPELINE_STATE_STREAM_DESC;
    using D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS                = ::D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS;
    using D3D12_TEXTURE_BARRIER                               = ::D3D12_TEXTURE_BARRIER;
    using D3D12_BUFFER_BARRIER                                = ::D3D12_BUFFER_BARRIER;
    using D3D12_GLOBAL_BARRIER                                = ::D3D12_GLOBAL_BARRIER;
    using D3D12_HEAP_DESC                                     = ::D3D12_HEAP_DESC;
    using D3D12_TILED_RESOURCE_COORDINATE                     = ::D3D12_TILED_RESOURCE_COORDINATE;
    using D3D12_TILE_REGION_SIZE                              = ::D3D12_TILE_REGION_SIZE;
    using D3D12_PACKED_MIP_INFO                               = ::D3D12_PACKED_MIP_INFO;
    using D3D12_TILE_SHAPE                                    = ::D3D12_TILE_SHAPE;
    using D3D12_SUBRESOURCE_TILING                            = ::D3D12_SUBRESOURCE_TILING;
    using D3D12_SHADER_DESC                                   = ::D3D12_SHADER_DESC;
    using D3D12_SHADER_BUFFER_DESC                            = ::D3D12_SHADER_BUFFER_DESC;
    using D3D12_LIBRARY_DESC                                  = ::D3D12_LIBRARY_DESC;
    using D3D12_FUNCTION_DESC                                 = ::D3D12_FUNCTION_DESC;
    using DSTORAGE_CONFIGURATION                              = ::DSTORAGE_CONFIGURATION;
    using D3D12_QUERY_HEAP_DESC                               = ::D3D12_QUERY_HEAP_DESC;
    using D3D12_FEATURE_DATA_D3D12_OPTIONS                    = ::D3D12_FEATURE_DATA_D3D12_OPTIONS;
    using D3D12_FEATURE_DATA_D3D12_OPTIONS5                   = ::D3D12_FEATURE_DATA_D3D12_OPTIONS5;
    using D3D12_FEATURE_DATA_D3D12_OPTIONS7                   = ::D3D12_FEATURE_DATA_D3D12_OPTIONS7;
    using D3D12_FEATURE_DATA_D3D12_OPTIONS12                  = ::D3D12_FEATURE_DATA_D3D12_OPTIONS12;
    using D3D12_FEATURE_DATA_D3D12_OPTIONS16                  = ::D3D12_FEATURE_DATA_D3D12_OPTIONS16;
    using D3D12_FEATURE_DATA_D3D12_OPTIONS21                  = ::D3D12_FEATURE_DATA_D3D12_OPTIONS21;
    using D3D12_FEATURE_DATA_SHADER_MODEL                     = ::D3D12_FEATURE_DATA_SHADER_MODEL;

    // ---- DXGI structs (extended) ---------------------------------------------
    using DXGI_QUERY_VIDEO_MEMORY_INFO                        = ::DXGI_QUERY_VIDEO_MEMORY_INFO;

    // ---- D3D12 descriptor handles --------------------------------------------
    using D3D12_CPU_DESCRIPTOR_HANDLE                       = ::D3D12_CPU_DESCRIPTOR_HANDLE;
    using D3D12_GPU_DESCRIPTOR_HANDLE                       = ::D3D12_GPU_DESCRIPTOR_HANDLE;

    // ---- D3DX12 helpers ------------------------------------------------------
    using CD3DX12_RESOURCE_DESC                             = ::CD3DX12_RESOURCE_DESC;
    using CD3DX12_RESOURCE_DESC1                            = ::CD3DX12_RESOURCE_DESC1;
    using CD3DX12_CPU_DESCRIPTOR_HANDLE                     = ::CD3DX12_CPU_DESCRIPTOR_HANDLE;
    using CD3DX12_GPU_DESCRIPTOR_HANDLE                     = ::CD3DX12_GPU_DESCRIPTOR_HANDLE;
    using CD3DX12_ROOT_PARAMETER1                           = ::CD3DX12_ROOT_PARAMETER1;
    using CD3DX12_DESCRIPTOR_RANGE1                         = ::CD3DX12_DESCRIPTOR_RANGE1;
    using CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC             = ::CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC;
    using CD3DX12_DEFAULT                                   = ::CD3DX12_DEFAULT;
    using CD3DX12_RASTERIZER_DESC                           = ::CD3DX12_RASTERIZER_DESC;
    using CD3DX12_BLEND_DESC                                = ::CD3DX12_BLEND_DESC;
    using CD3DX12_DEPTH_STENCIL_DESC                        = ::CD3DX12_DEPTH_STENCIL_DESC;
    using CD3DX12_STATE_OBJECT_DESC                         = ::CD3DX12_STATE_OBJECT_DESC;
    using CD3DX12_DXIL_LIBRARY_SUBOBJECT                    = ::CD3DX12_DXIL_LIBRARY_SUBOBJECT;
    using CD3DX12_SHADER_BYTECODE                           = ::CD3DX12_SHADER_BYTECODE;
    using CD3DX12_TEXTURE_COPY_LOCATION                     = ::CD3DX12_TEXTURE_COPY_LOCATION;

    // ---- D3D12 enums ---------------------------------------------------------
    using D3D12_TEXTURE_ADDRESS_MODE                        = ::D3D12_TEXTURE_ADDRESS_MODE;
    using D3D12_PRIMITIVE_TOPOLOGY_TYPE                     = ::D3D12_PRIMITIVE_TOPOLOGY_TYPE;
    using D3D12_COMPARISON_FUNC                             = ::D3D12_COMPARISON_FUNC;
    using D3D12_FILTER                                      = ::D3D12_FILTER;
    using D3D12_COMMAND_LIST_TYPE                           = ::D3D12_COMMAND_LIST_TYPE;
    using D3D12_HEAP_TYPE                                   = ::D3D12_HEAP_TYPE;
    using D3D12_HEAP_FLAGS                                  = ::D3D12_HEAP_FLAGS;
    using D3D12_DESCRIPTOR_HEAP_FLAGS                       = ::D3D12_DESCRIPTOR_HEAP_FLAGS;
    using D3D12_DESCRIPTOR_HEAP_TYPE                        = ::D3D12_DESCRIPTOR_HEAP_TYPE;
    using D3D12_CULL_MODE                                   = ::D3D12_CULL_MODE;
    using D3D12_FILL_MODE                                   = ::D3D12_FILL_MODE;
    using D3D12_BLEND                                       = ::D3D12_BLEND;
    using D3D12_STENCIL_OP                                  = ::D3D12_STENCIL_OP;
    using D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS = ::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS;
    using D3D12_DESCRIPTOR_RANGE_TYPE                       = ::D3D12_DESCRIPTOR_RANGE_TYPE;
    using D3D12_SHADER_VISIBILITY                           = ::D3D12_SHADER_VISIBILITY;
    using D3D12_BARRIER_LAYOUT                              = ::D3D12_BARRIER_LAYOUT;
    using D3D12_BARRIER_SYNC                                = ::D3D12_BARRIER_SYNC;
    using D3D12_BARRIER_ACCESS                              = ::D3D12_BARRIER_ACCESS;
    using D3D12_RESOURCE_FLAGS                              = ::D3D12_RESOURCE_FLAGS;
    using D3D12_HIT_GROUP_TYPE                              = ::D3D12_HIT_GROUP_TYPE;
    using D3D12_RAYTRACING_GEOMETRY_TYPE                    = ::D3D12_RAYTRACING_GEOMETRY_TYPE;
    using D3D12_RAYTRACING_GEOMETRY_FLAGS                   = ::D3D12_RAYTRACING_GEOMETRY_FLAGS;
    using D3D12_DRED_ENABLEMENT                             = ::D3D12_DRED_ENABLEMENT;
    using D3D12_DESCRIPTOR_RANGE_FLAGS                      = ::D3D12_DESCRIPTOR_RANGE_FLAGS;
    using D3D12_ROOT_DESCRIPTOR_FLAGS                       = ::D3D12_ROOT_DESCRIPTOR_FLAGS;
    using D3D12_ROOT_SIGNATURE_FLAGS                        = ::D3D12_ROOT_SIGNATURE_FLAGS;
    using D3D12_STATIC_BORDER_COLOR                         = ::D3D12_STATIC_BORDER_COLOR;
    using D3D12_RAYTRACING_TIER                             = ::D3D12_RAYTRACING_TIER;
    using D3D12_MESH_SHADER_TIER                            = ::D3D12_MESH_SHADER_TIER;
    using D3D12_WORK_GRAPHS_TIER                            = ::D3D12_WORK_GRAPHS_TIER;
    using DXGI_DEBUG_RLO_FLAGS                              = ::DXGI_DEBUG_RLO_FLAGS;
    using DSTORAGE_COMPRESSION_FORMAT                       = ::DSTORAGE_COMPRESSION_FORMAT;
    using DSTORAGE_DEBUG                                    = ::DSTORAGE_DEBUG;
    using D3D12_SRV_DIMENSION                               = ::D3D12_SRV_DIMENSION;
    using D3D12_BUFFER_SRV_FLAGS                            = ::D3D12_BUFFER_SRV_FLAGS;
    using D3D12_PIPELINE_STATE_SUBOBJECT_TYPE               = ::D3D12_PIPELINE_STATE_SUBOBJECT_TYPE;
    using D3D12_STATE_OBJECT_TYPE                           = ::D3D12_STATE_OBJECT_TYPE;
    using D3D12_TEXTURE_COPY_TYPE                           = ::D3D12_TEXTURE_COPY_TYPE;
    using D3D12_CPU_PAGE_PROPERTY                           = ::D3D12_CPU_PAGE_PROPERTY;
    using D3D12_MEMORY_POOL                                 = ::D3D12_MEMORY_POOL;
    using D3D12_TILE_RANGE_FLAGS                            = ::D3D12_TILE_RANGE_FLAGS;
    using D3D12_COMMAND_QUEUE_PRIORITY                      = ::D3D12_COMMAND_QUEUE_PRIORITY;
    using D3D12_CONSERVATIVE_RASTERIZATION_MODE             = ::D3D12_CONSERVATIVE_RASTERIZATION_MODE;
    using D3D12_DEPTH_WRITE_MASK                            = ::D3D12_DEPTH_WRITE_MASK;

    // ---- D3D common ----------------------------------------------------------
    using D3D_FEATURE_LEVEL                                 = ::D3D_FEATURE_LEVEL;
    using D3D_SHADER_MODEL                                  = ::D3D_SHADER_MODEL;
    using D3D_PRIMITIVE_TOPOLOGY                            = ::D3D_PRIMITIVE_TOPOLOGY;

    // ---- DXGI structs --------------------------------------------------------
    using DXGI_FORMAT                                       = ::DXGI_FORMAT;
    using DXGI_SAMPLE_DESC                                  = ::DXGI_SAMPLE_DESC;
    using DXGI_SWAP_CHAIN_DESC                              = ::DXGI_SWAP_CHAIN_DESC;
    using DXGI_SWAP_CHAIN_DESC1                             = ::DXGI_SWAP_CHAIN_DESC1;
    using DXGI_ADAPTER_DESC                                 = ::DXGI_ADAPTER_DESC;
    using DXGI_OUTPUT_DESC                                  = ::DXGI_OUTPUT_DESC;

    // ---- D3D12 interfaces ----------------------------------------------------
    using ID3D12CommandList             = ::ID3D12CommandList;
    using ID3D12Heap                    = ::ID3D12Heap;
    using ID3D12Resource                = ::ID3D12Resource;
    using ID3D12Device14                = ::ID3D12Device14;
    using ID3D12Fence                   = ::ID3D12Fence;
    using ID3D12GraphicsCommandList10   = ::ID3D12GraphicsCommandList10;
    using ID3D12CommandAllocator        = ::ID3D12CommandAllocator;
    using ID3D12CommandSignature        = ::ID3D12CommandSignature;
    using ID3D12PipelineState           = ::ID3D12PipelineState;
    using ID3D12StateObject             = ::ID3D12StateObject;
    using ID3D12StateObjectProperties   = ::ID3D12StateObjectProperties;
    using ID3D12StateObjectProperties1  = ::ID3D12StateObjectProperties1;
    using ID3DBlob                      = ::ID3DBlob;
    using ID3D12DescriptorHeap          = ::ID3D12DescriptorHeap;
    using ID3D12QueryHeap               = ::ID3D12QueryHeap;
    using ID3D12RootSignature           = ::ID3D12RootSignature;
    using ID3D12CommandQueue            = ::ID3D12CommandQueue;
    using ID3D12Debug                   = ::ID3D12Debug;
    using ID3D12Debug1                  = ::ID3D12Debug1;
    using ID3D12InfoQueue               = ::ID3D12InfoQueue;
    using ID3D12InfoQueue1              = ::ID3D12InfoQueue1;   // RegisterMessageCallback
    using ID3D12DeviceRemovedExtendedDataSettings = ::ID3D12DeviceRemovedExtendedDataSettings;
    using ID3D12DeviceRemovedExtendedData         = ::ID3D12DeviceRemovedExtendedData;
    using ID3D12WorkGraphProperties               = ::ID3D12WorkGraphProperties;

    // ---- D3D12 shader reflection interfaces ----------------------------------
    using ID3D12ShaderReflection                  = ::ID3D12ShaderReflection;
    using ID3D12ShaderReflectionConstantBuffer    = ::ID3D12ShaderReflectionConstantBuffer;
    using ID3D12LibraryReflection                 = ::ID3D12LibraryReflection;
    using ID3D12FunctionReflection                = ::ID3D12FunctionReflection;

    // ---- DirectStorage interfaces --------------------------------------------
    using IDStorageFactory              = ::IDStorageFactory;
    using IDStorageFile                 = ::IDStorageFile;
    using IDStorageQueue2               = ::IDStorageQueue2;
    using IDStorageCompressionCodec     = ::IDStorageCompressionCodec;

    // ---- DXGI interfaces -----------------------------------------------------
    using IDXGIAdapter4                 = ::IDXGIAdapter4;
    using IDXGISwapChain3               = ::IDXGISwapChain3;
    using IDXGISwapChain4               = ::IDXGISwapChain4;
    using IDXGISurface2                 = ::IDXGISurface2;
    using IDXGIDevice2                  = ::IDXGIDevice2;
    using IDXGIFactory7                 = ::IDXGIFactory7;
    using IDXGIOutput                   = ::IDXGIOutput;
    using IDXGIOutput6                  = ::IDXGIOutput6;
    using IDXGIDebug                    = ::IDXGIDebug;

    // ---- Free functions -------------------------------------------------------
    using ::D3D12GetDebugInterface;
    using ::D3D12GetFormatPlaneCount;
    using ::D3D12SerializeVersionedRootSignature;
    using ::CreateDXGIFactory2;
    using ::DXGIGetDebugInterface1;
    using ::D3D12CreateDevice;
    using ::DStorageCreateCompressionCodec;
    using ::DStorageSetConfiguration;
    using ::DStorageGetFactory;
    using ::D3D12_DEFAULT;
}

export namespace D3D12
{

   // Simple integer macros — safe to use as constexpr initializers
    inline constexpr unsigned SHADER_IDENTIFIER_SIZE_IN_BYTES      = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    inline constexpr unsigned TEXTURE_DATA_PLACEMENT_ALIGNMENT     = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
    inline constexpr unsigned TEXTURE_DATA_PITCH_ALIGNMENT         = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
    inline constexpr unsigned SMALL_RESOURCE_PLACEMENT_ALIGNMENT   = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
    inline constexpr unsigned DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    // D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING is a bitwise expression — use literal
    inline constexpr unsigned DEFAULT_SHADER_4_COMPONENT_MAPPING            = 0x1688u;
    // D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_N are macros — expose as constants
    inline constexpr unsigned SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0 = 0;
    inline constexpr unsigned SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1 = 1;
    inline constexpr unsigned SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2 = 2;
    inline constexpr unsigned SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3 = 3;
}

#undef ERROR_NOT_FOUND   // winerror.h defines this as Win32 error code 1168 — would mangle the identifier
export namespace DXGI
{
    // _HRESULT_TYPEDEF_ and (1L<<n) macros are not constexpr-safe — use literals
    inline constexpr HRESULT  ERROR_NOT_FOUND            = static_cast<HRESULT>(0x887A0002L);
    inline constexpr unsigned USAGE_RENDER_TARGET_OUTPUT = 0x00000020u;
    inline constexpr unsigned USAGE_BACK_BUFFER          = 0x00000040u;
    // GUIDs — defined by value to avoid external symbol dependency on dxguid.lib
    // Values from dxgidebug.h: DEFINE_GUID(DXGI_DEBUG_ALL, 0xe48ae283,0xda80,0x490b,0x87,0xe6,0x43,0xe9,0xa9,0xcf,0xda,0x08)
    inline constexpr GUID    DEBUG_ALL     = { 0xe48ae283, 0xda80, 0x490b, { 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x08 } };
    inline constexpr DXGI_DEBUG_RLO_FLAGS DEBUG_RLO_ALL = DXGI_DEBUG_RLO_ALL;
}

export namespace DStorage
{
    inline constexpr unsigned MAX_QUEUE_CAPACITY = DSTORAGE_MAX_QUEUE_CAPACITY;
}

export namespace DirectXTex
{
    using TexMetadata        = ::DirectX::TexMetadata;
    using ScratchImage       = ::DirectX::ScratchImage;
    using Image              = ::DirectX::Image;
    using Blob               = ::DirectX::Blob;
    using WIC_FLAGS          = ::DirectX::WIC_FLAGS;
    using DDS_FLAGS          = ::DirectX::DDS_FLAGS;
    using TEX_COMPRESS_FLAGS = ::DirectX::TEX_COMPRESS_FLAGS;
    using TEX_FILTER_FLAGS   = ::DirectX::TEX_FILTER_FLAGS;
    using TEX_DIMENSION      = ::DirectX::TEX_DIMENSION;

    // Unscoped enum values promoted into this namespace for convenience
    inline constexpr WIC_FLAGS WIC_FLAGS_NONE = ::DirectX::WIC_FLAGS_NONE;
    
    // -------------------------------------------------------------------------
    // Free functions — thin inline wrappers forwarding to DirectX::
    // -------------------------------------------------------------------------
    inline HRESULT GetMetadataFromTGAFile(const wchar_t* szFile, DirectX::TexMetadata& metadata) noexcept { return DirectX::GetMetadataFromTGAFile(szFile, metadata); }
    inline HRESULT GetMetadataFromWICFile(const wchar_t* szFile, DirectX::WIC_FLAGS flags, DirectX::TexMetadata& metadata) noexcept { return DirectX::GetMetadataFromWICFile(szFile, flags, metadata); }
    inline HRESULT LoadFromTGAMemory(const void* pSource, size_t size, DirectX::TexMetadata* metadata, DirectX::ScratchImage& image) noexcept { return DirectX::LoadFromTGAMemory(static_cast<const uint8_t*>(pSource), size, metadata, image); }
    inline HRESULT LoadFromDDSMemory(const void* pSource, size_t size, DirectX::DDS_FLAGS flags, DirectX::TexMetadata* metadata, DirectX::ScratchImage& image) noexcept { return DirectX::LoadFromDDSMemory(static_cast<const uint8_t*>(pSource), size, flags, metadata, image); }
    inline HRESULT LoadFromWICMemory(const void* pSource, size_t size, DirectX::WIC_FLAGS flags, DirectX::TexMetadata* metadata, DirectX::ScratchImage& image) noexcept { return DirectX::LoadFromWICMemory(static_cast<const uint8_t*>(pSource), size, flags, metadata, image); }
    inline HRESULT GenerateMipMaps(const DirectX::Image* srcImages, size_t nimages, const DirectX::TexMetadata& metadata, DirectX::TEX_FILTER_FLAGS filter, size_t levels, DirectX::ScratchImage& mipChain) noexcept { return DirectX::GenerateMipMaps(srcImages, nimages, metadata, filter, levels, mipChain); }
    inline HRESULT SaveToWICMemory(const DirectX::Image& image, DirectX::WIC_FLAGS flags, REFGUID guidContainerFormat, DirectX::Blob& blob) noexcept { return DirectX::SaveToWICMemory(image, flags, guidContainerFormat, blob); }
}


export DEFINE_GUID(GUID_ContainerFormatPng,  0x1b7cfaf4, 0x713f, 0x473c, 0xbb, 0xcd, 0x61, 0x37, 0x42, 0x5f, 0xae, 0xaf);