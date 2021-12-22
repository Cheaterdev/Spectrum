module;


export module D3D.Shaders;

import Singleton;
import FileSystem;
import FileDepender;
import DXCompiler;
import serialization;
import Utils;

export
{

namespace D3D
{
 
    struct shader_macro
    {
            std::string name;
            std::string value;
            shader_macro() = default;
            shader_macro(std::string name, std::string value = "1");

            GEN_DEF_COMP(shader_macro)
        private:
            SERIALIZE()
            {
                ar& NVP(name) &NVP(value);
            }
    };

    struct shader_header
    {
            std::string file_name;
            std::string entry_point;
            DWORD flags;
            std::vector<shader_macro> macros;
            bool contains_text = false;

            GEN_DEF_COMP(shader_header)
        private:
            SERIALIZE()
            {
                ar& NVP(file_name);
                ar& NVP(entry_point);
                ar& NVP(flags);
                ar& NVP(macros);
                ar& NVP(contains_text);
            }
    };

	class shader_include
	{
		std::string dir;
		resource_file_depender& depender;
  
	public:
		std::vector<std::string> autogen;

		shader_include(std::string dir, resource_file_depender& _depender);
		std::unique_ptr<std::string> load_file(std::string filename);
	};
}




struct ShaderCompiler :public Singleton<ShaderCompiler>
{
    std::optional<binary> Compile_Shader(std::string shaderText, std::vector < D3D::shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", D3D::shader_include* includer = nullptr, std::string file_name="");
    std::optional<binary> Compile_Shader_File(std::string filename, std::vector < D3D::shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", D3D::shader_include* includer = nullptr);

private:

    ShaderCompiler();
    friend class Singleton<ShaderCompiler>;
    friend class shader_include_dxil;
    dxc::DxcDllSupport		DxcDllHelper;
    IDxcCompiler* compiler = nullptr;
    IDxcLibrary* library = nullptr;

};


}