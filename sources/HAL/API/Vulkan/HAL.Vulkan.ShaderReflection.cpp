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
    // -fvk-use-dx-layout          : preserve cbuffer/structured-buffer memory layout
    // -fvk-b-shift 0 all          : CBVs keep their register number as the binding index
    // -fvk-t-shift 128 all        : SRVs start at binding 128
    // -fvk-u-shift 256 all        : UAVs start at binding 256
    // -fvk-s-shift 384 all        : Samplers start at binding 384
    // (Must match the VkDescriptorSetLayoutBinding layout in Device::init)
    // -fvk-bind-resource-heap 0 0 : SM6.6 ResourceDescriptorHeap → set 0, binding 0
    //                               matches: b0=SAMPLED_IMAGE, b1=STORAGE_IMAGE,
    //                                        b2=UNIFORM_BUFFER, b3=STORAGE_BUFFER
    // -fvk-bind-sampler-heap 0 1  : SM6.6 SamplerDescriptorHeap → set 1, binding 0
    //                               matches: b0=SAMPLER
    // (Requires DXC 1.9+; added alongside SM6.6 SPIRV bindless support)
    //
    // float16_tN aliases: -enable-16bit-types is intentionally suppressed for SPIRV
    // (Vulkan SPIRV spec requires image sampled types to be 32-bit).  Shaders that
    // declare float16_tN variables/params still need the types to exist, so we
    // define them as float aliases via the preprocessor — arithmetic stays correct,
    // image declarations become Texture2D<floatN> which is valid SPIRV.
    std::vector<std::wstring> get_extra_compile_args(const std::string& /*target*/)
    {
        return {
            L"-spirv",
            L"-fspv-target-env=vulkan1.3",
            L"-fvk-use-dx-layout",
            L"-fvk-b-shift", L"0",   L"all",
            L"-fvk-t-shift", L"128", L"all",
            L"-fvk-u-shift", L"256", L"all",
            L"-fvk-s-shift", L"384", L"all",
            L"-fvk-bind-resource-heap", L"0", L"0",
            L"-fvk-bind-sampler-heap",  L"0", L"1",
            // float16_t aliases — map 16-bit scalar/vector types to 32-bit
            // so shaders compile without -enable-16bit-types (blocked for SPIRV;
            // see DXC.ShaderCompiler.cpp).  Texture2D<float16_tN> → Texture2D<floatN>.
            L"-Dfloat16_t=float",
            L"-Dfloat16_t2=float2",
            L"-Dfloat16_t3=float3",
            L"-Dfloat16_t4=float4",
            L"-Dint16_t=int",
            L"-Dint16_t2=int2",
            L"-Dint16_t3=int3",
            L"-Dint16_t4=int4",
            L"-Duint16_t=uint",
            L"-Duint16_t2=uint2",
            L"-Duint16_t3=uint3",
            L"-Duint16_t4=uint4",
        };
    }
}
