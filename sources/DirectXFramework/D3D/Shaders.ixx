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



 const CLSID _CLSID_DxcCompiler = {
    0x73e22d93,
    0xe6ce,
    0x47f3,
    {0xb5, 0xbf, 0xf0, 0x66, 0x4f, 0x39, 0xc1, 0xb0} };

// {EF6A8087-B0EA-4D56-9E45-D07E1A8B7806}
 const GUID _CLSID_DxcLibrary = {
    0x6245d6af,
    0x66e0,
    0x48fd,
    {0x80, 0xb4, 0x4d, 0x27, 0x17, 0x96, 0x74, 0x8c} };


struct D3D12ShaderCompilerInfo :public Singleton<D3D12ShaderCompilerInfo>
{
	dxc::DxcDllSupport		DxcDllHelper;
	IDxcCompiler* compiler = nullptr;
	IDxcLibrary* library = nullptr;
	
    std::unique_ptr<std::string> Compile_Shader(std::string shaderText, std::vector < D3D::shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", D3D::shader_include* includer = nullptr, std::string file_name="");
	std::unique_ptr<std::string> Compile_Shader_File(std::string filename, std::vector < D3D::shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", D3D::shader_include* includer = nullptr);

	D3D12ShaderCompilerInfo()
	{
		auto hr = DxcDllHelper.Initialize();
		//	Utils::Validate(hr, L"Failed to initialize DxCDllSupport!");

		DxcDllHelper.CreateInstance(_CLSID_DxcCompiler, &compiler);
		//	Utils::Validate(hr, L"Failed to create DxcCompiler!");

		DxcDllHelper.CreateInstance(_CLSID_DxcLibrary, &library);
	//	Utils::Validate(hr, L"Failed to create DxcLibrary!");
	}
};


}