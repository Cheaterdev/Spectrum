module HAL:Shader;

namespace HAL
{

    const char* Shader<pixel_shader>::compile_code = "ps_5_1";
    const char* Shader<vertex_shader>::compile_code = "vs_5_1";
    const char* Shader<geometry_shader>::compile_code = "gs_5_1";
    const char* Shader<domain_shader>::compile_code = "ds_5_1";
    const char* Shader<hull_shader>::compile_code = "hs_5_1";
    const char* Shader<compute_shader>::compile_code = "cs_5_1";
	
	const char* Shader<library_shader>::compile_code = "";
    const char* Shader<mesh_shader>::compile_code = "";
    const char* Shader<amplification_shader>::compile_code = "";

	

	const char* Shader<pixel_shader>::compile_code_dxil = "ps_6_6";
	const char* Shader<vertex_shader>::compile_code_dxil = "vs_6_6";
	const char* Shader<geometry_shader>::compile_code_dxil = "gs_6_6";
	const char* Shader<domain_shader>::compile_code_dxil = "ds_6_6";
	const char* Shader<hull_shader>::compile_code_dxil = "hs_6_6";
	const char* Shader<compute_shader>::compile_code_dxil = "cs_6_6";

    const char* Shader<mesh_shader>::compile_code_dxil = "ms_6_6";
    const char* Shader<amplification_shader>::compile_code_dxil = "as_6_6";

	
	const char* Shader<library_shader>::compile_code_dxil = "lib_6_6";


    const vertex_shader::ptr vertex_shader::null(new vertex_shader());
    const pixel_shader::ptr pixel_shader::null(new pixel_shader());
    const geometry_shader::ptr geometry_shader::null(new geometry_shader());
    const domain_shader::ptr domain_shader::null(new domain_shader());
    const hull_shader::ptr hull_shader::null(new hull_shader());
    const compute_shader::ptr compute_shader::null(new compute_shader());
	const library_shader::ptr library_shader::null(new library_shader());
    const mesh_shader::ptr mesh_shader::null(new mesh_shader());
    const amplification_shader::ptr amplification_shader::null(new amplification_shader());



	shader_macro::shader_macro(std::string name, std::string value /*= "1"*/)
	{
		this->name = name;
		this->value = value;
	}




	shader_include::shader_include(std::string dir, resource_file_depender& _depender) : depender(_depender)
	{

		std::transform(dir.begin(), dir.end(), dir.begin(), [](char s) {
			if (s == '/') return '\\';
			return s;
			});
		this->dir = dir.substr(0, dir.find_last_of('\\')) + '\\';
	}

	std::unique_ptr<std::string> shader_include::load_file(std::string pFileName)
	{
		auto sig = pFileName.find("autogen");
		if (sig != std::string::npos)
		{
			autogen.emplace_back(pFileName);
		}

		std::string file_name = dir + pFileName;
		auto file = FileSystem::get().get_file(convert(file_name));


		if (!file) file = FileSystem::get().get_file(convert(pFileName));
		if (!file)
		{
			return nullptr;
		}

		auto data = file->load_all();
		depender.add_depend(file);

		return std::make_unique<std::string>(std::move(data));

	}

}