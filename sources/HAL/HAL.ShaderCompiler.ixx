export module HAL:ShaderCompiler;


import :API.ShaderCompiler;
import :Types;
import Core;

import :Slots;

export namespace HAL
{


	struct CompiledFunction
	{
		std::string name;
		std::wstring wname;
		UsedSlots slots;

		SERIALIZE()
			{
				ar& NVP(name);
				ar& NVP(wname);

				ar& NVP(slots);
			}
	};
	struct CompiledShader
	{
		std::vector<CompiledFunction> functions;
		binary blob;
		
		SERIALIZE()
			{
				ar& NVP(functions);
				ar& NVP(blob);
			}
	};

	struct ShaderCompiler :public Singleton<ShaderCompiler>, public API::ShaderCompiler
	{
		std::optional<CompiledShader> Compile_Shader(std::string shaderText, std::vector < shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", ShaderOptions options = ShaderOptions::None, shader_include* includer = nullptr, std::string file_name = "");
		std::optional<CompiledShader> Compile_Shader_File(std::string filename, std::vector < shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", ShaderOptions options = ShaderOptions::None, shader_include* includer = nullptr);

	private:
		ShaderCompiler();
		friend class Singleton<ShaderCompiler>;
	};


}