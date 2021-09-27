#pragma once
#include "FileSystem/FileSystem.h"

#ifndef D3DCOMPILE_DEBUG
#define D3DCOMPILE_DEBUG                          (1 << 0)
#define D3DCOMPILE_SKIP_VALIDATION                (1 << 1)
#define D3DCOMPILE_SKIP_OPTIMIZATION              (1 << 2)
#define D3DCOMPILE_PACK_MATRIX_ROW_MAJOR          (1 << 3)
#define D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR       (1 << 4)
#define D3DCOMPILE_PARTIAL_PRECISION              (1 << 5)
#define D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT       (1 << 6)
#define D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT       (1 << 7)
#define D3DCOMPILE_NO_PRESHADER                   (1 << 8)
#define D3DCOMPILE_AVOID_FLOW_CONTROL             (1 << 9)
#define D3DCOMPILE_PREFER_FLOW_CONTROL            (1 << 10)
#define D3DCOMPILE_ENABLE_STRICTNESS              (1 << 11)
#define D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY (1 << 12)
#define D3DCOMPILE_IEEE_STRICTNESS                (1 << 13)
#define D3DCOMPILE_OPTIMIZATION_LEVEL0            (1 << 14)
#define D3DCOMPILE_OPTIMIZATION_LEVEL1            0
#define D3DCOMPILE_OPTIMIZATION_LEVEL2            ((1 << 14) | (1 << 15))
#define D3DCOMPILE_OPTIMIZATION_LEVEL3            (1 << 15)
#define D3DCOMPILE_RESERVED16                     (1 << 16)
#define D3DCOMPILE_RESERVED17                     (1 << 17)
#define D3DCOMPILE_WARNINGS_ARE_ERRORS            (1 << 18)
#endif

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
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
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
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
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

