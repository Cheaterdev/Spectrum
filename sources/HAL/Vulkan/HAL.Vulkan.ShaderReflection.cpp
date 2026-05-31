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
            // Phase 4: populate f.slots from SPIR-V reflection.
        }
        // For library targets (entry_point empty), per-function reflection is
        // also deferred to Phase 4.
    }
}
