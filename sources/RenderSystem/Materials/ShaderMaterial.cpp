#include "pch_render.h"
#include "ShaderMaterial.h"
#include "Materials/universal_material.h"

ShaderMaterial::ShaderMaterial(std::string file_name, std::string entry)//: file(this)
{

	if (!file_name.empty())
	{
		auto shader = Graphics::pixel_shader::get_resource({ file_name, entry,{} });

		pipeline = std::make_shared<materials::PipelineSimple>(0, shader);

	}
	else
	{
		pipeline = std::make_shared<materials::PipelineSimple>(0, nullptr);
	}
	
}

void ShaderMaterial::on_asset_change(std::shared_ptr<Asset> asset)
{
	// if (asset == *file)
	//      shader = Graphics::pixel_shader::create_from_memory(file->get_data(), "PC", 0);
}
void ShaderMaterial::set(RENDER_TYPE render_type, MESH_TYPE type, Graphics::PipelineStateDesc &pipeline)
{
	//pipeline.pixel = shader;
}
void ShaderMaterial::set(MESH_TYPE type, MeshRenderContext::ptr& context)
{
    //context->pipeline.pixel = shader;
}
