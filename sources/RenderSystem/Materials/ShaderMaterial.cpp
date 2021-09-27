#include "pch.h"
#include "ShaderMaterial.h"
#include "Materials/universal_material.h"

BOOST_CLASS_EXPORT_IMPLEMENT(ShaderMaterial);

template<class Archive>
void ShaderMaterial::load(Archive& ar, const unsigned int)
{
    ar& NVP(boost::serialization::base_object<material>(*this));
    ar& NVP(boost::serialization::base_object<AssetHolder>(*this));
    //  ar& NVP(file);
    //ar& NVP(shader);
}

template<class Archive>
void ShaderMaterial::save(Archive& ar, const unsigned int) const
{
    ar& NVP(boost::serialization::base_object<material>(*this));
    ar& NVP(boost::serialization::base_object<AssetHolder>(*this));
    //  ar& NVP(file);
   // ar& NVP(shader);
}
template<class Archive>
void ShaderMaterial::serialize(Archive& ar, const unsigned int file_version)
{
    boost::serialization::split_member(ar, *this, file_version);
}

ShaderMaterial::ShaderMaterial(std::string file_name, std::string entry)//: file(this)
{

	if (!file_name.empty())
	{
		auto shader = Render::pixel_shader::get_resource({ file_name, entry,{} });

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
	//      shader = Render::pixel_shader::create_from_memory(file->get_data(), "PC", 0);
}
void ShaderMaterial::set(RENDER_TYPE render_type, MESH_TYPE type, Render::PipelineStateDesc &pipeline)
{
	//pipeline.pixel = shader;
}
void ShaderMaterial::set(MESH_TYPE type, MeshRenderContext::ptr& context)
{
    //context->pipeline.pixel = shader;
}

template void ShaderMaterial::serialize(serialization_oarchive& arch, const unsigned int version);
template void ShaderMaterial::serialize(serialization_iarchive& arch, const unsigned int version);