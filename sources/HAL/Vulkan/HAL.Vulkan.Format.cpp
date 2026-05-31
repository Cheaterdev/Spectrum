module HAL:Format;

import :Format;
import Core;

// Backend-specific Format query methods.
// Mirrors D3D12/HAL.Format.cpp.  These switch on the HAL::Format::Formats enum
// (whose ordering matches DXGI), so the logic is backend-portable — only the
// shader-component-mapping constant differs (it is a D3D12 concept; on Vulkan
// component swizzle is expressed per-view, so we return the same opaque value
// the rest of the HAL code treats as a token).

namespace HAL
{
    bool Format::is_shader_visible() const
    {
        switch (native_format)
        {
        case D32_FLOAT_S8X24_UINT:
        case R32_FLOAT_X8X24_TYPELESS:
        case X32_TYPELESS_G8X24_UINT:
        case D24_UNORM_S8_UINT:
        case R24_UNORM_X8_TYPELESS:
        case X24_TYPELESS_G8_UINT:
            return false;
        default:
            return true;
        }
    }

    bool Format::is_srgb() const
    {
        switch (native_format)
        {
        case R8G8B8A8_UNORM_SRGB:
        case BC1_UNORM_SRGB:
        case BC2_UNORM_SRGB:
        case BC3_UNORM_SRGB:
        case B8G8R8A8_UNORM_SRGB:
        case B8G8R8X8_UNORM_SRGB:
        case BC7_UNORM_SRGB:
            return true;
        default:
            return false;
        }
    }

    bool Format::is_blendable() const
    {
        switch (native_format)
        {
        case R32G32B32A32_FLOAT:
        case R32G32B32_FLOAT:
        case R16G16B16A16_FLOAT:
        case R16G16B16A16_UNORM:
        case R16G16B16A16_SNORM:
        case R32G32_FLOAT:
        case R10G10B10A2_UNORM:
        case R11G11B10_FLOAT:
        case R8G8B8A8_UNORM:
        case R8G8B8A8_UNORM_SRGB:
        case R8G8B8A8_SNORM:
        case R16G16_FLOAT:
        case R16G16_UNORM:
        case R16G16_SNORM:
        case R32_FLOAT:
        case R8G8_UNORM:
        case R8G8_SNORM:
        case R16_FLOAT:
        case R16_UNORM:
        case R16_SNORM:
        case R8_UNORM:
        case R8_SNORM:
        case A8_UNORM:
        case R8G8_B8G8_UNORM:
        case G8R8_G8B8_UNORM:
        case B5G6R5_UNORM:
        case B5G5R5A1_UNORM:
        case B8G8R8A8_UNORM:
        case B8G8R8X8_UNORM:
        case B8G8R8A8_UNORM_SRGB:
        case B8G8R8X8_UNORM_SRGB:
            return true;
        default:
            return false;
        }
    }

    Format Format::to_dsv() const
    {
        switch (native_format)
        {
        case R32_TYPELESS: return D32_FLOAT;
        case R16_TYPELESS: return D16_UNORM;
        case R8_TYPELESS:  return R8_TYPELESS; // oops! (matches D3D12 path)
        default:           return *this;
        }
    }

    Format Format::to_typeless() const
    {
        switch (native_format)
        {
        case R8G8B8A8_UNORM_SRGB:
        case R8G8B8A8_UNORM:
        case R8G8B8A8_UINT:
        case R8G8B8A8_SNORM:
        case R8G8B8A8_SINT:
            return R8G8B8A8_TYPELESS;
        default:
            return *this;
        }
    }

    Format Format::to_srv() const
    {
        switch (native_format)
        {
        case R8G8B8A8_TYPELESS: return R8G8B8A8_UNORM;
        case R32_TYPELESS:      return R32_FLOAT;
        case R16_TYPELESS:      return R16_FLOAT;
        case R8_TYPELESS:       return R8_UNORM;
        default:                return *this;
        }
    }

    uint Format::size() const
    {
        switch (native_format)
        {
        case R32G32B32A32_TYPELESS:
        case R32G32B32A32_FLOAT:
        case R32G32B32A32_UINT:
        case R32G32B32A32_SINT:
            return 128;

        case R32G32B32_TYPELESS:
        case R32G32B32_FLOAT:
        case R32G32B32_UINT:
        case R32G32B32_SINT:
            return 96;

        case R16G16B16A16_TYPELESS:
        case R16G16B16A16_FLOAT:
        case R16G16B16A16_UNORM:
        case R16G16B16A16_UINT:
        case R16G16B16A16_SNORM:
        case R16G16B16A16_SINT:
        case R32G32_TYPELESS:
        case R32G32_FLOAT:
        case R32G32_UINT:
        case R32G32_SINT:
        case R32G8X24_TYPELESS:
        case D32_FLOAT_S8X24_UINT:
        case R32_FLOAT_X8X24_TYPELESS:
        case X32_TYPELESS_G8X24_UINT:
            return 64;

        case R10G10B10A2_TYPELESS:
        case R10G10B10A2_UNORM:
        case R10G10B10A2_UINT:
        case R11G11B10_FLOAT:
        case R8G8B8A8_TYPELESS:
        case R8G8B8A8_UNORM:
        case R8G8B8A8_UNORM_SRGB:
        case R8G8B8A8_UINT:
        case R8G8B8A8_SNORM:
        case R8G8B8A8_SINT:
        case R16G16_TYPELESS:
        case R16G16_FLOAT:
        case R16G16_UNORM:
        case R16G16_UINT:
        case R16G16_SNORM:
        case R16G16_SINT:
        case R32_TYPELESS:
        case D32_FLOAT:
        case R32_FLOAT:
        case R32_UINT:
        case R32_SINT:
        case R24G8_TYPELESS:
        case D24_UNORM_S8_UINT:
        case R24_UNORM_X8_TYPELESS:
        case X24_TYPELESS_G8_UINT:
        case R9G9B9E5_SHAREDEXP:
        case R8G8_B8G8_UNORM:
        case G8R8_G8B8_UNORM:
        case B8G8R8A8_UNORM:
        case B8G8R8X8_UNORM:
        case R10G10B10_XR_BIAS_A2_UNORM:
        case B8G8R8A8_TYPELESS:
        case B8G8R8A8_UNORM_SRGB:
        case B8G8R8X8_TYPELESS:
        case B8G8R8X8_UNORM_SRGB:
            return 32;

        case R8G8_TYPELESS:
        case R8G8_UNORM:
        case R8G8_UINT:
        case R8G8_SNORM:
        case R8G8_SINT:
        case R16_TYPELESS:
        case R16_FLOAT:
        case D16_UNORM:
        case R16_UNORM:
        case R16_UINT:
        case R16_SNORM:
        case R16_SINT:
        case B5G6R5_UNORM:
        case B5G5R5A1_UNORM:
            return 16;

        case R8_TYPELESS:
        case R8_UNORM:
        case R8_UINT:
        case R8_SNORM:
        case R8_SINT:
        case A8_UNORM:
            return 8;

        case R1_UNORM:
            return 1;

        case BC1_TYPELESS:
        case BC1_UNORM:
        case BC1_UNORM_SRGB:
            return 4;

        case BC2_TYPELESS:
        case BC2_UNORM:
        case BC2_UNORM_SRGB:
        case BC3_TYPELESS:
        case BC3_UNORM:
        case BC3_UNORM_SRGB:
        case BC4_TYPELESS:
        case BC4_UNORM:
        case BC4_SNORM:
        case BC5_TYPELESS:
        case BC5_UNORM:
        case BC5_SNORM:
        case BC6H_TYPELESS:
        case BC6H_UF16:
        case BC6H_SF16:
        case BC7_TYPELESS:
        case BC7_UNORM:
        case BC7_UNORM_SRGB:
            return 8;

        default:
            ASSERT(FALSE);
            return 0;
        }
    }

    uint Format::get_default_mapping() const
    {
        // D3D12's DEFAULT_SHADER_4_COMPONENT_MAPPING literal (0x1688). On Vulkan
        // component swizzle is per-view; this token is consumed by view code,
        // which the Vulkan backend reinterprets in Phase 4.
        return 0x1688u;
    }

    SurfaceInfo Format::surface_info(uint2 size) const
    {
        uint64 numBytes = 0;
        uint rowBytes = 0;
        uint numRows = 0;

        bool bc = false;
        bool packed = false;
        uint bcnumBytesPerBlock = 0;

        switch (native_format)
        {
        case BC1_TYPELESS:
        case BC1_UNORM:
        case BC1_UNORM_SRGB:
        case BC4_TYPELESS:
        case BC4_UNORM:
        case BC4_SNORM:
            bc = true; bcnumBytesPerBlock = 8; break;

        case BC2_TYPELESS:
        case BC2_UNORM:
        case BC2_UNORM_SRGB:
        case BC3_TYPELESS:
        case BC3_UNORM:
        case BC3_UNORM_SRGB:
        case BC5_TYPELESS:
        case BC5_UNORM:
        case BC5_SNORM:
        case BC6H_TYPELESS:
        case BC6H_UF16:
        case BC6H_SF16:
        case BC7_TYPELESS:
        case BC7_UNORM:
        case BC7_UNORM_SRGB:
            bc = true; bcnumBytesPerBlock = 16; break;

        case R8G8_B8G8_UNORM:
        case G8R8_G8B8_UNORM:
            packed = true; break;
        default:
            break;
        }

        if (bc)
        {
            uint numBlocksWide = 0;
            if (size.x > 0) numBlocksWide = std::max<uint>(1, (size.x + 3) / 4);
            uint numBlocksHigh = 0;
            if (size.y > 0) numBlocksHigh = std::max<uint>(1, (size.y + 3) / 4);
            rowBytes = numBlocksWide * bcnumBytesPerBlock;
            numRows = numBlocksHigh;
        }
        else if (packed)
        {
            rowBytes = ((size.x + 1) >> 1) * 4;
            numRows = size.y;
        }
        else
        {
            uint bpp = this->size();
            rowBytes = (size.x * bpp + 7) / 8;
            numRows = size.y;
        }

        numBytes = rowBytes * numRows;
        return { numBytes, rowBytes, numRows };
    }
}
