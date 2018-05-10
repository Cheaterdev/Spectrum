#include "pch.h"


namespace DX12
{

    const char* Shader<pixel_shader>::compile_code = "ps_5_1";
    const char* Shader<vertex_shader>::compile_code = "vs_5_1";
    const char* Shader<geometry_shader>::compile_code = "gs_5_1";
    const char* Shader<domain_shader>::compile_code = "ds_5_1";
    const char* Shader<hull_shader>::compile_code = "hs_5_1";
    const char* Shader<compute_shader>::compile_code = "cs_5_1";



    const vertex_shader::ptr vertex_shader::null(new vertex_shader());
    const pixel_shader::ptr pixel_shader::null(new pixel_shader());
    const geometry_shader::ptr geometry_shader::null(new geometry_shader());
    const domain_shader::ptr domain_shader::null(new domain_shader());
    const hull_shader::ptr hull_shader::null(new hull_shader());
    const compute_shader::ptr compute_shader::null(new compute_shader());




    void vertex_shader::compile()
    {
        Render::shader_inputs input_desc;

        for (auto& r : reflection.get_inputs())
        {
            input_desc.inputs.push_back({ r.SemanticName, r.SemanticIndex });
        }

        this->input_desc = Render::layout_id_generator::get().get_unique(input_desc);
    }

}