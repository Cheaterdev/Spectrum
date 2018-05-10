#include "pch.h"




BOOST_CLASS_EXPORT_IMPLEMENT(materials::universal_material);

template<class Archive>
void materials::universal_material::load(Archive& ar, const unsigned int)
{
    ar& NVP(boost::serialization::base_object<material>(*this));
    ar& NVP(boost::serialization::base_object<AssetHolder>(*this));
    ar& NVP(shader);
    ar& NVP(graph);
    compile();
}

template<class Archive>
void materials::universal_material::save(Archive& ar, const unsigned int) const
{
    ar& NVP(boost::serialization::base_object<material>(*this));
    ar& NVP(boost::serialization::base_object<AssetHolder>(*this));
    ar& NVP(shader);
    ar& NVP(graph);
}
template<class Archive>
void materials::universal_material::serialize(Archive& ar, const unsigned int file_version)
{
    boost::serialization::split_member(ar, *this, file_version);
}
void materials::universal_material::set(MeshRenderContext::ptr context)
{
    context->pipeline.pixel = shader;
    context->set_material_textures(texture_handles);
}
void materials::universal_material::compile()
{
    texture_handles = Render::DescriptorHeapManager::get().get_csu()->create_table(textures.size());

    for (int i = 0; i < textures.size(); i++)
        textures[i]->get_texture()->place_srv(texture_handles[i]);
}
template void materials::universal_material::serialize(serialization_oarchive& arch, const unsigned int version);
template void materials::universal_material::serialize(serialization_iarchive& arch, const unsigned int version);