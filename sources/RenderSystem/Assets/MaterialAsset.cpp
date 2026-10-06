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
    AssetRenderer::get().draw(self, preview);

    // The first draw creates the material's pipeline; its shaders then compile
    // in the background, and until they land its draws are skipped -- the
    // preview would stay without the material. Wait (this is the preview
    // thread) and draw again once it can render.
    if (!is_ready_to_draw())
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
        while (!is_ready_to_draw() && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (is_ready_to_draw())
            AssetRenderer::get().draw(self, preview);
    }
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

