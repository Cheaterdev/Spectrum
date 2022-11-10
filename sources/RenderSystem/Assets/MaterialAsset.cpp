module Graphics:MaterialAsset;

import :AssetRenderer;

REGISTER_TYPE(MaterialAsset);

import Graphics;

//////////////////////////////////////////////////////////////////////////
Asset_Type MaterialAsset::get_type()
{
    return Asset_Type::MATERIAL;
}

MaterialAsset::MaterialAsset(materials::material::ptr m)
{

    mark_changed();
}

void MaterialAsset::update_preview(HAL::Texture::ptr preview)
{
    AssetRenderer::get().draw(get_ptr<MaterialAsset>(), preview);
    mark_changed();
}

