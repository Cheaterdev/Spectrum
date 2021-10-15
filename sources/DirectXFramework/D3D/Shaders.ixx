module;

#include "utils/utils_macros.h"
export module D3D.Shaders;

import Singleton;
import FileSystem;
import FileDepender;
import DXCompiler;
import serialization;

export
{

namespace D3D
{
    enum class D3D_VERSION
    {
        V11,
        V12
    };
    struct shader_macro
    {
            std::string name;
            std::string value;
            shader_macro() = default;
            shader_macro(std::string name, std::string value = "1");

            bool operator==(const shader_macro&) const = default;
            auto operator<=>(const  shader_macro& r)  const = default;
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

            bool operator==(const shader_header&) const = default;
            auto operator<=>(const  shader_header & r)  const = default;


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

   // bool operator<(const shader_header& l, const shader_header& r);

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




struct D3D12ShaderCompilerInfo :public Singleton<D3D12ShaderCompilerInfo>
{
	dxc::DxcDllSupport		DxcDllHelper;
	IDxcCompiler* compiler = nullptr;
	IDxcLibrary* library = nullptr;
	
    std::unique_ptr<std::string> Compile_Shader(std::string shaderText, std::vector < D3D::shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", D3D::shader_include* includer = nullptr, std::string file_name="");
	std::unique_ptr<std::string> Compile_Shader_File(std::string filename, std::vector < D3D::shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", D3D::shader_include* includer = nullptr);

	D3D12ShaderCompilerInfo()
	{
		DxcDllHelper.Initialize();
		//	Utils::Validate(hr, L"Failed to initialize DxCDllSupport!");

		DxcDllHelper.CreateInstance(CLSID_DxcCompiler, &compiler);
		//	Utils::Validate(hr, L"Failed to create DxcCompiler!");

		DxcDllHelper.CreateInstance(CLSID_DxcLibrary, &library);
	//	Utils::Validate(hr, L"Failed to create DxcLibrary!");
	}
};


}