module Graphics:MaterialAsset;
import <RenderSystem.h>;

import :AssetRenderer;

REGISTER_TYPE(MaterialAsset);
CEREAL_FORCE_REGISTER(MaterialAsset);

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

void material::set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics)
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

