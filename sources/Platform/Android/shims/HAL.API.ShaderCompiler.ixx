// Android stand-in for HAL/DXC/DXC.ShaderCompiler.ixx. DXC is a Windows DLL; on
// Android shaders ship precompiled to SPIR-V, so the API-specific state is empty.
// (HAL::ShaderCompiler's Compile_* entry points still need an Android
// implementation that loads those blobs.)
export module HAL:API.ShaderCompiler;

export namespace HAL
{
    namespace API
    {
        struct ShaderCompiler
        {
        };
    }
}
