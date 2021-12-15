#include "pch_render.h"
#include "Assets/AssetRenderer.h"
import FlowGraph;

CEREAL_REGISTER_TYPE(MaterialAsset);
CEREAL_REGISTER_TYPE(AABB)

CEREAL_REGISTER_TYPE(Frustum)
CEREAL_REGISTER_TYPE(Primitive)
CEREAL_REGISTER_TYPE(Sphere)

CEREAL_REGISTER_TYPE(FlowGraph::window);
CEREAL_REGISTER_TYPE(FlowGraph::Node);
CEREAL_REGISTER_TYPE(FlowGraph::input);
CEREAL_REGISTER_TYPE(FlowGraph::output);
CEREAL_REGISTER_TYPE(FlowGraph::graph);
CEREAL_REGISTER_TYPE(FlowGraph::graph_input);
CEREAL_REGISTER_TYPE(FlowGraph::graph_output);
CEREAL_REGISTER_TYPE(FlowGraph::parameter_type);
CEREAL_REGISTER_TYPE(FlowGraph::strict_parameter);

CEREAL_REGISTER_TYPE(texture_data);
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

