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
    // get() would construct an AssetRenderer on a backend where GraphicsSystem
    // deliberately doesn't create one (Vulkan: no material pipelines yet).
    if (!AssetRenderer::is_good()) return;
    auto self = get_ptr<MaterialAsset>();

    // Previews are drawn on the render thread, which must not wait: a material whose shaders are still
    // compiling draws nothing, so say so and let the caller come back. A loaded material only makes its
    // pipeline on its first draw, so that draw is made (its image is thrown away) to get it started.
    if (!is_ready_to_draw())
    {
        if (!has_pipeline())
            AssetRenderer::get().draw(self, preview);

        AssetRenderer::defer_preview();
        return;
    }

    AssetRenderer::get().draw(self, preview);
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

