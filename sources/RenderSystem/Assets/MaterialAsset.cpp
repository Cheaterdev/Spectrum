module Graphics:MaterialAsset;


import :AssetRenderer;

REGISTER_TYPE(MaterialAsset);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Asset, MaterialAsset);
CEREAL_REGISTER_DYNAMIC_INIT(MaterialAsset);
CEREAL_FORCE_REGISTER(MaterialAsset);
CEREAL_FORCE_REGISTER_RELATION(Asset, MaterialAsset);

CEREAL_FORCE_DYNAMIC_INIT(universal_material);

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

namespace materials
{

Pipeline::Pipeline(uint id) : id(id)
{
}

uint Pipeline::get_id()
{
    return id;
}

void material::set(MESH_TYPE type, MeshRenderContext::ptr&)
{
}

void material::set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion)
{
}

void material::compile()
{
}

void material::update()
{
}

size_t material::get_pipeline_id()
{
    return pipeline_id;
}

}

