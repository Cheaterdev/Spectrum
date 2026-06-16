module HAL:ShaderCompiler;

import wrl;
import Core;
import d3d12;        // ID3D12ShaderReflection / ID3D12LibraryReflection
import DXCompiler;
import :Slots;       // get_slot

// D3D12 implementation of the reflect_shader() seam declared in
// DXC/DXC.ShaderCompiler.cpp.  Extracts per-pass constant-buffer slot usage from
// the DXIL reflection blob.  This is the only D3D12-coupled part of shader
// compilation, kept here so the DXC compile path itself stays backend-neutral.

namespace HAL
{
    void reflect_shader(IDxcUtils* library, const DxcBuffer& reflectionBuffer,
                        const std::string& entry_point, CompiledShader& blob_str)
    {
        if (entry_point.size())
        {
            blob_str.functions.emplace_back();
            auto& f = blob_str.functions.back();

            ComPtr<ID3D12ShaderReflection> shaderReflection{};
            library->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));
            D3D12_SHADER_DESC shaderDesc{};
            shaderReflection->GetDesc(&shaderDesc);
            f.name = entry_point;
            f.wname = convert(f.name);
            for (uint i = 0; i < shaderDesc.ConstantBuffers; i++)
            {
                ID3D12ShaderReflectionConstantBuffer* cb = shaderReflection->GetConstantBufferByIndex(i);
                D3D12_SHADER_BUFFER_DESC shaderBufferDesc{};
                cb->GetDesc(&shaderBufferDesc);

                std::string cb_name = shaderBufferDesc.Name;
                if (cb_name.starts_with("pass_"))
                {
                    cb_name = cb_name.substr(5);
                    auto slot_id = get_slot(cb_name);
                    if (slot_id)
                        f.slots.merge(slot_id.value());
                }
            }
        }
        else
        {
            ComPtr<ID3D12LibraryReflection> libraryReflection{};
            auto hr3 = library->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&libraryReflection));

            D3D12_LIBRARY_DESC shaderDesc{};
            libraryReflection->GetDesc(&shaderDesc);

            for (uint i = 0; i < shaderDesc.FunctionCount; i++)
            {
                ID3D12FunctionReflection* f = libraryReflection->GetFunctionByIndex(i);
                D3D12_FUNCTION_DESC functionDesc{};
                f->GetDesc(&functionDesc);

                blob_str.functions.emplace_back();
                auto& rf = blob_str.functions.back();
                rf.name = functionDesc.Name;
                rf.wname = convert(rf.name);

                for (uint i = 0; i < functionDesc.ConstantBuffers; i++)
                {
                    ID3D12ShaderReflectionConstantBuffer* cb = f->GetConstantBufferByIndex(i);
                    D3D12_SHADER_BUFFER_DESC shaderBufferDesc{};
                    cb->GetDesc(&shaderBufferDesc);

                    std::string cb_name = shaderBufferDesc.Name;
                    if (cb_name.starts_with("pass_"))
                    {
                        cb_name = cb_name.substr(5);
                        auto slot_id = get_slot(cb_name);
                        rf.slots.merge(slot_id.value());
                    }
                }
            }
        }
    }

    // D3D12: no extra compile flags (produces DXIL, not SPIR-V).
    std::vector<std::wstring> get_extra_compile_args(const std::string& /*target*/)
    {
        return {};
    }
}
