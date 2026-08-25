module;
// Global module fragment: include Vulkan headers directly so that
// `static const VkPipelineStageFlagBits2` / `VkAccessFlagBits2` values
// (VK_PIPELINE_STAGE_2_*, VK_ACCESS_2_*) are visible as file-scope names.
// MSVC does not export `static const` namespace-scope variables from header
// units, so `import vulkan;` alone is not enough for these types.
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
module HAL:Utils;
import stl.core;
import Core;

// Vulkan conversion helpers: HAL abstract types → Vulkan native types.
// These mirror the to_native() / from_native() functions provided by the
// D3D12 Utils for D3D12 types.  Only the conversions needed through Phase 3
// (clear screen) are implemented here; pipeline/sampler conversions arrive
// in Phase 4.
//
// Defined at global scope with `using namespace HAL` to match the D3D12 Utils
// convention and the declarations in HAL.Vulkan.Utils.ixx.

using namespace HAL;

// ============================================================================
//  Format
// ============================================================================

VkFormat to_native(Format format)
{
    switch (format)
    {
    // ---- 8-bit ---------------------------------------------------------------
    case Format::R8_UNORM:              return VK_FORMAT_R8_UNORM;
    case Format::R8_UINT:               return VK_FORMAT_R8_UINT;
    case Format::R8_SNORM:              return VK_FORMAT_R8_SNORM;
    case Format::R8_SINT:               return VK_FORMAT_R8_SINT;
    case Format::R8G8_UNORM:            return VK_FORMAT_R8G8_UNORM;
    case Format::R8G8_UINT:             return VK_FORMAT_R8G8_UINT;
    case Format::R8G8_SNORM:            return VK_FORMAT_R8G8_SNORM;
    case Format::R8G8_SINT:             return VK_FORMAT_R8G8_SINT;
    case Format::R8G8B8A8_UNORM:        return VK_FORMAT_R8G8B8A8_UNORM;
    case Format::R8G8B8A8_UNORM_SRGB:   return VK_FORMAT_R8G8B8A8_SRGB;
    case Format::R8G8B8A8_UINT:         return VK_FORMAT_R8G8B8A8_UINT;
    case Format::R8G8B8A8_SNORM:        return VK_FORMAT_R8G8B8A8_SNORM;
    case Format::R8G8B8A8_SINT:         return VK_FORMAT_R8G8B8A8_SINT;
    case Format::B8G8R8A8_UNORM:        return VK_FORMAT_B8G8R8A8_UNORM;
    case Format::B8G8R8A8_UNORM_SRGB:   return VK_FORMAT_B8G8R8A8_SRGB;
    // Alpha-only (glyph/font atlases).  Exact match provided by VK_KHR_maintenance5.
    case Format::A8_UNORM:              return VK_FORMAT_A8_UNORM;
    // ---- 16-bit --------------------------------------------------------------
    case Format::R16_FLOAT:             return VK_FORMAT_R16_SFLOAT;
    case Format::R16_UNORM:             return VK_FORMAT_R16_UNORM;
    case Format::R16_UINT:              return VK_FORMAT_R16_UINT;
    case Format::R16_SNORM:             return VK_FORMAT_R16_SNORM;
    case Format::R16_SINT:              return VK_FORMAT_R16_SINT;
    case Format::R16G16_FLOAT:          return VK_FORMAT_R16G16_SFLOAT;
    case Format::R16G16_UNORM:          return VK_FORMAT_R16G16_UNORM;
    case Format::R16G16_UINT:           return VK_FORMAT_R16G16_UINT;
    case Format::R16G16_SNORM:          return VK_FORMAT_R16G16_SNORM;
    case Format::R16G16_SINT:           return VK_FORMAT_R16G16_SINT;
    case Format::R16G16B16A16_FLOAT:    return VK_FORMAT_R16G16B16A16_SFLOAT;
    case Format::R16G16B16A16_UNORM:    return VK_FORMAT_R16G16B16A16_UNORM;
    case Format::R16G16B16A16_UINT:     return VK_FORMAT_R16G16B16A16_UINT;
    case Format::R16G16B16A16_SNORM:    return VK_FORMAT_R16G16B16A16_SNORM;
    case Format::R16G16B16A16_SINT:     return VK_FORMAT_R16G16B16A16_SINT;
    // ---- 32-bit --------------------------------------------------------------
    case Format::R32_FLOAT:             return VK_FORMAT_R32_SFLOAT;
    case Format::R32_UINT:              return VK_FORMAT_R32_UINT;
    case Format::R32_SINT:              return VK_FORMAT_R32_SINT;
    case Format::R32G32_FLOAT:          return VK_FORMAT_R32G32_SFLOAT;
    case Format::R32G32_UINT:           return VK_FORMAT_R32G32_UINT;
    case Format::R32G32_SINT:           return VK_FORMAT_R32G32_SINT;
    case Format::R32G32B32_FLOAT:       return VK_FORMAT_R32G32B32_SFLOAT;
    case Format::R32G32B32_UINT:        return VK_FORMAT_R32G32B32_UINT;
    case Format::R32G32B32_SINT:        return VK_FORMAT_R32G32B32_SINT;
    case Format::R32G32B32A32_FLOAT:    return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Format::R32G32B32A32_UINT:     return VK_FORMAT_R32G32B32A32_UINT;
    case Format::R32G32B32A32_SINT:     return VK_FORMAT_R32G32B32A32_SINT;
    // ---- packed --------------------------------------------------------------
    case Format::R10G10B10A2_UNORM:     return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case Format::R10G10B10A2_UINT:      return VK_FORMAT_A2B10G10R10_UINT_PACK32;
    case Format::R11G11B10_FLOAT:       return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case Format::R9G9B9E5_SHAREDEXP:    return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    case Format::B5G6R5_UNORM:          return VK_FORMAT_R5G6B5_UNORM_PACK16;
    case Format::B5G5R5A1_UNORM:        return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
    // ---- depth/stencil -------------------------------------------------------
    case Format::D16_UNORM:             return VK_FORMAT_D16_UNORM;
    case Format::D24_UNORM_S8_UINT:     return VK_FORMAT_D24_UNORM_S8_UINT;
    case Format::D32_FLOAT:             return VK_FORMAT_D32_SFLOAT;
    case Format::D32_FLOAT_S8X24_UINT:  return VK_FORMAT_D32_SFLOAT_S8_UINT;
    // ---- block-compressed ----------------------------------------------------
    case Format::BC1_UNORM:             return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case Format::BC1_UNORM_SRGB:        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case Format::BC2_UNORM:             return VK_FORMAT_BC2_UNORM_BLOCK;
    case Format::BC2_UNORM_SRGB:        return VK_FORMAT_BC2_SRGB_BLOCK;
    case Format::BC3_UNORM:             return VK_FORMAT_BC3_UNORM_BLOCK;
    case Format::BC3_UNORM_SRGB:        return VK_FORMAT_BC3_SRGB_BLOCK;
    case Format::BC4_UNORM:             return VK_FORMAT_BC4_UNORM_BLOCK;
    case Format::BC4_SNORM:             return VK_FORMAT_BC4_SNORM_BLOCK;
    case Format::BC5_UNORM:             return VK_FORMAT_BC5_UNORM_BLOCK;
    case Format::BC5_SNORM:             return VK_FORMAT_BC5_SNORM_BLOCK;
    case Format::BC6H_UF16:             return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case Format::BC6H_SF16:             return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case Format::BC7_UNORM:             return VK_FORMAT_BC7_UNORM_BLOCK;
    case Format::BC7_UNORM_SRGB:        return VK_FORMAT_BC7_SRGB_BLOCK;
    // ---- TYPELESS → concrete -------------------------------------------------
    // Vulkan has no typeless formats; a typeless D3D12 resource maps to the
    // natural typed format and is reinterpreted per-view (images that need it
    // are created with VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT).
    case Format::R32G32B32A32_TYPELESS: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Format::R32G32B32_TYPELESS:    return VK_FORMAT_R32G32B32_SFLOAT;
    case Format::R16G16B16A16_TYPELESS: return VK_FORMAT_R16G16B16A16_SFLOAT;
    case Format::R32G32_TYPELESS:       return VK_FORMAT_R32G32_SFLOAT;
    case Format::R10G10B10A2_TYPELESS:  return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case Format::R8G8B8A8_TYPELESS:     return VK_FORMAT_R8G8B8A8_UNORM;
    case Format::B8G8R8A8_TYPELESS:     return VK_FORMAT_B8G8R8A8_UNORM;
    case Format::R16G16_TYPELESS:       return VK_FORMAT_R16G16_SFLOAT;
    case Format::R32_TYPELESS:          return VK_FORMAT_R32_SFLOAT;
    case Format::R8G8_TYPELESS:         return VK_FORMAT_R8G8_UNORM;
    case Format::R16_TYPELESS:          return VK_FORMAT_R16_SFLOAT;
    case Format::R8_TYPELESS:           return VK_FORMAT_R8_UNORM;
    case Format::R24G8_TYPELESS:        return VK_FORMAT_D24_UNORM_S8_UINT;
    case Format::R32G8X24_TYPELESS:     return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case Format::BC1_TYPELESS:          return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case Format::BC2_TYPELESS:          return VK_FORMAT_BC2_UNORM_BLOCK;
    case Format::BC3_TYPELESS:          return VK_FORMAT_BC3_UNORM_BLOCK;
    case Format::BC4_TYPELESS:          return VK_FORMAT_BC4_UNORM_BLOCK;
    case Format::BC5_TYPELESS:          return VK_FORMAT_BC5_UNORM_BLOCK;
    case Format::BC6H_TYPELESS:         return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case Format::BC7_TYPELESS:          return VK_FORMAT_BC7_UNORM_BLOCK;
    // ---- formats with no direct Vulkan equivalent ----------------------------
    // TYPELESS formats have no Vulkan analogue; callers should resolve to a
    // typed format before calling to_native().
    // A8_UNORM, R1_UNORM, subsampled YUV formats, etc. are also unsupported.
    default:
        Log::get() << "[Vulkan] to_native(Format): unhandled HAL format "
                   << static_cast<int>(static_cast<Format::Formats>(format))
                   << " — returning VK_FORMAT_UNDEFINED" << Log::endl;
        ASSERT(false && "to_native(Format): unhandled format — see log");
        return VK_FORMAT_UNDEFINED;
    }
}

Format from_native(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8_UNORM:                      return Format::R8_UNORM;
    case VK_FORMAT_R8_UINT:                       return Format::R8_UINT;
    case VK_FORMAT_R8_SNORM:                      return Format::R8_SNORM;
    case VK_FORMAT_R8_SINT:                       return Format::R8_SINT;
    case VK_FORMAT_R8G8_UNORM:                    return Format::R8G8_UNORM;
    case VK_FORMAT_R8G8_UINT:                     return Format::R8G8_UINT;
    case VK_FORMAT_R8G8_SNORM:                    return Format::R8G8_SNORM;
    case VK_FORMAT_R8G8_SINT:                     return Format::R8G8_SINT;
    case VK_FORMAT_R8G8B8A8_UNORM:                return Format::R8G8B8A8_UNORM;
    case VK_FORMAT_R8G8B8A8_SRGB:                 return Format::R8G8B8A8_UNORM_SRGB;
    case VK_FORMAT_R8G8B8A8_UINT:                 return Format::R8G8B8A8_UINT;
    case VK_FORMAT_R8G8B8A8_SNORM:                return Format::R8G8B8A8_SNORM;
    case VK_FORMAT_R8G8B8A8_SINT:                 return Format::R8G8B8A8_SINT;
    case VK_FORMAT_B8G8R8A8_UNORM:                return Format::B8G8R8A8_UNORM;
    case VK_FORMAT_B8G8R8A8_SRGB:                 return Format::B8G8R8A8_UNORM_SRGB;
    case VK_FORMAT_A8_UNORM:                      return Format::A8_UNORM;
    case VK_FORMAT_R16_SFLOAT:                    return Format::R16_FLOAT;
    case VK_FORMAT_R16_UNORM:                     return Format::R16_UNORM;
    case VK_FORMAT_R16_UINT:                      return Format::R16_UINT;
    case VK_FORMAT_R16_SNORM:                     return Format::R16_SNORM;
    case VK_FORMAT_R16_SINT:                      return Format::R16_SINT;
    case VK_FORMAT_R16G16_SFLOAT:                 return Format::R16G16_FLOAT;
    case VK_FORMAT_R16G16_UNORM:                  return Format::R16G16_UNORM;
    case VK_FORMAT_R16G16_UINT:                   return Format::R16G16_UINT;
    case VK_FORMAT_R16G16_SNORM:                  return Format::R16G16_SNORM;
    case VK_FORMAT_R16G16_SINT:                   return Format::R16G16_SINT;
    case VK_FORMAT_R16G16B16A16_SFLOAT:           return Format::R16G16B16A16_FLOAT;
    case VK_FORMAT_R16G16B16A16_UNORM:            return Format::R16G16B16A16_UNORM;
    case VK_FORMAT_R16G16B16A16_UINT:             return Format::R16G16B16A16_UINT;
    case VK_FORMAT_R16G16B16A16_SNORM:            return Format::R16G16B16A16_SNORM;
    case VK_FORMAT_R16G16B16A16_SINT:             return Format::R16G16B16A16_SINT;
    case VK_FORMAT_R32_SFLOAT:                    return Format::R32_FLOAT;
    case VK_FORMAT_R32_UINT:                      return Format::R32_UINT;
    case VK_FORMAT_R32_SINT:                      return Format::R32_SINT;
    case VK_FORMAT_R32G32_SFLOAT:                 return Format::R32G32_FLOAT;
    case VK_FORMAT_R32G32_UINT:                   return Format::R32G32_UINT;
    case VK_FORMAT_R32G32_SINT:                   return Format::R32G32_SINT;
    case VK_FORMAT_R32G32B32_SFLOAT:              return Format::R32G32B32_FLOAT;
    case VK_FORMAT_R32G32B32_UINT:                return Format::R32G32B32_UINT;
    case VK_FORMAT_R32G32B32_SINT:                return Format::R32G32B32_SINT;
    case VK_FORMAT_R32G32B32A32_SFLOAT:           return Format::R32G32B32A32_FLOAT;
    case VK_FORMAT_R32G32B32A32_UINT:             return Format::R32G32B32A32_UINT;
    case VK_FORMAT_R32G32B32A32_SINT:             return Format::R32G32B32A32_SINT;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:      return Format::R10G10B10A2_UNORM;
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:       return Format::R10G10B10A2_UINT;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:       return Format::R11G11B10_FLOAT;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:        return Format::R9G9B9E5_SHAREDEXP;
    case VK_FORMAT_R5G6B5_UNORM_PACK16:           return Format::B5G6R5_UNORM;
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:         return Format::B5G5R5A1_UNORM;
    case VK_FORMAT_D16_UNORM:                     return Format::D16_UNORM;
    case VK_FORMAT_D32_SFLOAT:                    return Format::D32_FLOAT;
    case VK_FORMAT_D24_UNORM_S8_UINT:             return Format::D24_UNORM_S8_UINT;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:            return Format::D32_FLOAT_S8X24_UINT;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:          return Format::BC1_UNORM;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:           return Format::BC1_UNORM_SRGB;
    case VK_FORMAT_BC2_UNORM_BLOCK:               return Format::BC2_UNORM;
    case VK_FORMAT_BC2_SRGB_BLOCK:                return Format::BC2_UNORM_SRGB;
    case VK_FORMAT_BC3_UNORM_BLOCK:               return Format::BC3_UNORM;
    case VK_FORMAT_BC3_SRGB_BLOCK:                return Format::BC3_UNORM_SRGB;
    case VK_FORMAT_BC4_UNORM_BLOCK:               return Format::BC4_UNORM;
    case VK_FORMAT_BC4_SNORM_BLOCK:               return Format::BC4_SNORM;
    case VK_FORMAT_BC5_UNORM_BLOCK:               return Format::BC5_UNORM;
    case VK_FORMAT_BC5_SNORM_BLOCK:               return Format::BC5_SNORM;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:             return Format::BC6H_UF16;
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:             return Format::BC6H_SF16;
    case VK_FORMAT_BC7_UNORM_BLOCK:               return Format::BC7_UNORM;
    case VK_FORMAT_BC7_SRGB_BLOCK:                return Format::BC7_UNORM_SRGB;
    default:
        Log::get() << "[Vulkan] from_native(VkFormat): unhandled VkFormat "
                   << static_cast<int>(format)
                   << " — returning Format::UNKNOWN" << Log::endl;
        ASSERT(false && "from_native(VkFormat): unhandled format — see log");
        return Format::UNKNOWN;
    }
}

// ============================================================================
//  TextureLayout → VkImageLayout
//  NOTE: TextureLayout is a sequential enum (not bit flags), so use a switch.
// ============================================================================

VkImageLayout to_native(TextureLayout layout)
{
    switch (layout)
    {
    case TextureLayout::UNDEFINED:           return VK_IMAGE_LAYOUT_UNDEFINED;
    case TextureLayout::PRESENT:             return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // == COMMON
   // case TextureLayout::GENERIC_READ:        return VK_IMAGE_LAYOUT_GENERAL;
    case TextureLayout::RENDER_TARGET:       return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case TextureLayout::UNORDERED_ACCESS:    return VK_IMAGE_LAYOUT_GENERAL;
    case TextureLayout::DEPTH_STENCIL_WRITE: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case TextureLayout::DEPTH_STENCIL_READ:  return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    // GENERAL instead of SHADER_READ_ONLY_OPTIMAL: D3D12 descriptors carry no
    // embedded layout, so the same slot is validly used as both SRV and UAV
    // without ever becoming "stale."  Vulkan's VkDescriptorImageInfo embeds a
    // layout, causing Warning 529 when a bindless STORAGE_IMAGE slot still
    // says GENERAL while the image has been transitioned back to
    // SHADER_READ_ONLY.  By keeping ALL shader-accessible images in GENERAL we
    // match D3D12 semantics: one stable layout for any shader access.
    // (GENERAL is a valid layout for VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE per spec.)
    case TextureLayout::SHADER_RESOURCE:     return VK_IMAGE_LAYOUT_GENERAL;
    case TextureLayout::COPY_SOURCE:         return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case TextureLayout::COPY_DEST:           return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
   // case TextureLayout::RESOLVE_SOURCE:      return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
   // case TextureLayout::RESOLVE_DEST:        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    default:                                 return VK_IMAGE_LAYOUT_GENERAL;
    }
}

uint32_t to_native_resource_state(TextureLayout layout)
{
    return static_cast<uint32_t>(to_native(layout));
}

// ============================================================================
//  BarrierSync → VkPipelineStageFlags2 (synchronization2)
// ============================================================================

VkPipelineStageFlags2 to_native_stage(BarrierSync sync)
{
    if (sync == BarrierSync::NONE) return VK_PIPELINE_STAGE_2_NONE;
    if (check(sync & BarrierSync::ALL)) return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkPipelineStageFlags2 result = VK_PIPELINE_STAGE_2_NONE;
    if (check(sync & BarrierSync::DRAW))             result |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    //if (check(sync & BarrierSync::INPUT_ASSEMBLER))  result |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    if (check(sync & BarrierSync::VERTEX_SHADING))   result |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
    if (check(sync & BarrierSync::PIXEL_SHADING))    result |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    if (check(sync & BarrierSync::DEPTH_STENCIL))    result |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    if (check(sync & BarrierSync::RENDER_TARGET))    result |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (check(sync & BarrierSync::COMPUTE_SHADING))  result |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    if (check(sync & BarrierSync::RAYTRACING))       result |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    if (check(sync & BarrierSync::COPY))             result |= VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    if (check(sync & BarrierSync::RESOLVE))          result |= VK_PIPELINE_STAGE_2_RESOLVE_BIT;
    if (check(sync & BarrierSync::EXECUTE_INDIRECT)) result |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    if (check(sync & BarrierSync::ALL_SHADING))      result |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    if (check(sync & BarrierSync::BUILD_RAYTRACING_ACCELERATION_STRUCTURE)) result |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    return result;
}

// ============================================================================
//  BarrierAccess → VkAccessFlags2
// ============================================================================

VkAccessFlags2 to_native_access(BarrierAccess access)
{
    if (check(access & BarrierAccess::NO_ACCESS)) return VK_ACCESS_2_NONE;

    VkAccessFlags2 result = VK_ACCESS_2_NONE;
    if (check(access & BarrierAccess::VERTEX_BUFFER))       result |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
    if (check(access & BarrierAccess::CONSTANT_BUFFER))     result |= VK_ACCESS_2_UNIFORM_READ_BIT;
    if (check(access & BarrierAccess::INDEX_BUFFER))        result |= VK_ACCESS_2_INDEX_READ_BIT;
    if (check(access & BarrierAccess::RENDER_TARGET))       result |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    if (check(access & BarrierAccess::UNORDERED_ACCESS))    result |= VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
    if (check(access & BarrierAccess::DEPTH_STENCIL_WRITE)) result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if (check(access & BarrierAccess::DEPTH_STENCIL_READ))  result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    if (check(access & BarrierAccess::SHADER_RESOURCE))     result |= VK_ACCESS_2_SHADER_READ_BIT;
    if (check(access & BarrierAccess::INDIRECT_ARGUMENT))   result |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    if (check(access & BarrierAccess::COPY_DEST))           result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
    if (check(access & BarrierAccess::COPY_SOURCE))         result |= VK_ACCESS_2_TRANSFER_READ_BIT;
    if (check(access & BarrierAccess::RESOLVE_DEST))        result |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    if (check(access & BarrierAccess::RESOLVE_SOURCE))      result |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    if (check(access & BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ))  result |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    if (check(access & BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE)) result |= VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    return result;
}

// ============================================================================
//  CommandListType → Vulkan queue family flags
// ============================================================================

VkQueueFlagBits to_native_queue(CommandListType type)
{
    switch (type)
    {
    case CommandListType::DIRECT:  return VK_QUEUE_GRAPHICS_BIT;
    case CommandListType::COMPUTE: return VK_QUEUE_COMPUTE_BIT;
    case CommandListType::COPY:    return VK_QUEUE_TRANSFER_BIT;
    default:                       return VK_QUEUE_GRAPHICS_BIT;
    }
}

// ============================================================================
//  Sampler conversion helpers
// ============================================================================

VkFilter to_native_filter(Filter f)
{
    switch (f)
    {
    case Filter::POINT:       return VK_FILTER_NEAREST;
    case Filter::ANISOTROPIC: return VK_FILTER_LINEAR;
    default:                  return VK_FILTER_LINEAR;
    }
}

VkSamplerMipmapMode to_native_mipmap(Filter f)
{
    return (f == Filter::POINT) ? VK_SAMPLER_MIPMAP_MODE_NEAREST
                                : VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

VkSamplerAddressMode to_native(TextureAddressMode mode)
{
    switch (mode)
    {
    case TextureAddressMode::WRAP:        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case TextureAddressMode::MIRROR:      return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case TextureAddressMode::CLAMP:       return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TextureAddressMode::BORDER:      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case TextureAddressMode::MIRROR_ONCE: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    default:                              return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

VkCompareOp to_native(ComparisonFunc func)
{
    switch (func)
    {
    case ComparisonFunc::NONE:          return VK_COMPARE_OP_ALWAYS;
    case ComparisonFunc::NEVER:         return VK_COMPARE_OP_NEVER;
    case ComparisonFunc::LESS:          return VK_COMPARE_OP_LESS;
    case ComparisonFunc::EQUAL:         return VK_COMPARE_OP_EQUAL;
    case ComparisonFunc::LESS_EQUAL:    return VK_COMPARE_OP_LESS_OR_EQUAL;
    case ComparisonFunc::GREATER:       return VK_COMPARE_OP_GREATER;
    case ComparisonFunc::NOT_EQUAL:     return VK_COMPARE_OP_NOT_EQUAL;
    case ComparisonFunc::GREATER_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case ComparisonFunc::ALWAYS:        return VK_COMPARE_OP_ALWAYS;
    default:                            return VK_COMPARE_OP_ALWAYS;
    }
}

VkSamplerCreateInfo to_native_sampler_ci(const SamplerDesc& desc)
{
    VkSamplerCreateInfo ci{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    ci.magFilter    = to_native_filter(desc.MagFilter);
    ci.minFilter    = to_native_filter(desc.MinFilter);
    ci.mipmapMode   = to_native_mipmap(desc.MipFilter);
    ci.addressModeU = to_native(desc.AddressU);
    ci.addressModeV = to_native(desc.AddressV);
    ci.addressModeW = to_native(desc.AddressW);
    ci.mipLodBias   = desc.MipLODBias;
    ci.anisotropyEnable = (desc.MinFilter == Filter::ANISOTROPIC ||
                           desc.MagFilter == Filter::ANISOTROPIC) ? VK_TRUE : VK_FALSE;
    ci.maxAnisotropy    = static_cast<float>(desc.MaxAnisotropy);
    ci.compareEnable    = (desc.ComparisonFunc != ComparisonFunc::NONE) ? VK_TRUE : VK_FALSE;
    ci.compareOp        = to_native(desc.ComparisonFunc);
    ci.minLod           = desc.MinLOD;
    ci.maxLod           = desc.MaxLOD;
    // Border color: Vulkan only supports a fixed set.
    // All HAL SamplerDesc border colors are (1,1,1,1) → opaque white.
    const auto& bc = desc.BorderColor;
    if (bc.x == 0.f && bc.y == 0.f && bc.z == 0.f && bc.w == 0.f)
        ci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    else if (bc.x == 0.f && bc.y == 0.f && bc.z == 0.f)
        ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    else
        ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    ci.unnormalizedCoordinates = VK_FALSE;
    return ci;
}
