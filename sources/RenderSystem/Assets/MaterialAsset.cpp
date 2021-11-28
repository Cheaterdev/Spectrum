#include "pch_render.h"
#include "Assets/AssetRenderer.h"

// BOOST_CLASS_EXPORT_IMPLEMENT(AssetReference<MaterialAsset>);


// CEREAL_REGISTER_TYPE(MaterialAsset);

// BOOST_CLASS_EXPORT_IMPLEMENT(materials::material);


template void AssetReference<MaterialAsset>::serialize(serialization_oarchive& arch, const unsigned int version);
template void AssetReference<MaterialAsset>::serialize(serialization_iarchive& arch, const unsigned int version);

//////////////////////////////////////////////////////////////////////////
Asset_Type MaterialAsset::get_type()
{
    return Asset_Type::MATERIAL;
}

MaterialAsset::MaterialAsset(materials::material::ptr m)
{

    mark_changed();
}

void MaterialAsset::update_preview(Render::Texture::ptr preview)
{
    AssetRenderer::get().draw(get_ptr<MaterialAsset>(), preview);
    mark_changed();
}

