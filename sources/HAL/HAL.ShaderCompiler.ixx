export module HAL:ShaderCompiler;

import :API.ShaderCompiler;
import :Types;
import Core;

export namespace HAL
{


	struct ShaderCompiler :public Singleton<ShaderCompiler>, public API::ShaderCompiler
	{
		std::optional<binary> Compile_Shader(std::string shaderText, std::vector < shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", ShaderOptions options = ShaderOptions::None, shader_include* includer = nullptr, std::string file_name = "");
		std::optional<binary> Compile_Shader_File(std::string filename, std::vector < shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", ShaderOptions options = ShaderOptions::None, shader_include* includer = nullptr);

	private:
		ShaderCompiler();
		friend class Singleton<ShaderCompiler>;
	};


}