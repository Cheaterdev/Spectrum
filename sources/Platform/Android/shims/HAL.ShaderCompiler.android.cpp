// Android stand-in for HAL/DXC/DXC.ShaderCompiler.cpp. There is no DXC on the phone and no
// shaders are shipped yet, so nothing can be compiled: every request is logged and answers
// "no result", which callers already treat as a failed compile. When precompiled SPIR-V blobs
// exist, this is where they get loaded instead.
module HAL:ShaderCompiler;

namespace HAL
{
    ShaderCompiler::ShaderCompiler() {}

    std::optional<CompiledShader> ShaderCompiler::Compile_Shader(
        std::string shaderText, std::vector<shader_macro> macros, std::string target, std::string entry_point,
        ShaderOptions options, shader_include* includer, std::string file_name, std::string* errors)
    {
        const std::string message = "no shader compiler on Android (entry '" + entry_point + "', file '" + file_name + "')";
        if (errors)
            *errors = message;
        else
            Log::get() << Log::LEVEL_WARNING << message << Log::endl;
        return std::nullopt;
    }

    std::optional<CompiledShader> ShaderCompiler::Compile_Shader_File(
        std::string filename, std::vector<shader_macro> macros, std::string target, std::string entry_point,
        ShaderOptions options, shader_include* includer)
    {
        Log::get() << Log::LEVEL_WARNING << "no shader compiler on Android (file '" << filename << "')" << Log::endl;
        return std::nullopt;
    }
}
