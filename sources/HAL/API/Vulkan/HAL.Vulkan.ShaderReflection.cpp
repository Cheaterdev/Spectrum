module HAL:ShaderCompiler;

import Core;
import DXCompiler;

// Vulkan stub of the reflect_shader() seam declared in
// DXC/DXC.ShaderCompiler.cpp.  D3D12 uses ID3D12ShaderReflection to recover
// per-pass constant-buffer slot usage from the DXIL reflection blob; on Vulkan
// (SPIR-V) reflection is deferred to Phase 4 (SPIR-V-Reflect or DXC's own
// SPIR-V reflection path).  For now we record the function name only — slot
// usage stays empty, which is sufficient until pipelines/bindless land.

namespace HAL
{
    void reflect_shader(IDxcUtils* /*library*/, const DxcBuffer& /*reflectionBuffer*/,
                        const std::string& entry_point, CompiledShader& blob_str)
    {
        if (entry_point.size())
        {
            blob_str.functions.emplace_back();
            auto& f = blob_str.functions.back();
            f.name  = entry_point;
            f.wname = convert(f.name);
            // Phase 5: populate f.slots from SPIR-V reflection.
        }
    }

    // Vulkan: compile HLSL to SPIR-V via DXC.
    // -fvk-use-dx-layout     : preserve cbuffer/structured-buffer memory layout
    // -fvk-b-shift 0 all     : CBVs keep their register number as the binding index
    // -fvk-t-shift 128 all   : SRVs start at binding 128
    // -fvk-u-shift 256 all   : UAVs start at binding 256
    // -fvk-s-shift 384 all   : Samplers start at binding 384
    // (Must match the VkDescriptorSetLayoutBinding layout in Device::init)
    std::vector<std::wstring> get_extra_compile_args(const std::string& /*target*/)
    {
        return {
            L"-spirv",
            L"-fvk-use-dx-layout",
            L"-fvk-b-shift", L"0",   L"all",
            L"-fvk-t-shift", L"128", L"all",
            L"-fvk-u-shift", L"256", L"all",
            L"-fvk-s-shift", L"384", L"all",
        };
    }
}
