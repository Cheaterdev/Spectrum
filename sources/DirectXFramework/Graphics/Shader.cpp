module;
#include "pch_dx.h"
module Graphics:Shader;

namespace Graphics
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

}