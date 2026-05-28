module Graphics:PreSceneSystem;

import <RenderSystem.h>;
import :Scene;
import :RTX;
import :MeshAsset;
import :FrameGraphContext;
import FrameGraph;

using namespace FrameGraph;

bool PassDefault<Passes::PreScene>::setup(
    Passes::PreScene::Context& data, FrameGraph::TaskBuilder& builder)
{
    builder.create(data.scene, { 1 }, ResourceFlags::UnorderedAccess);
    return true;
}

void PassDefault<Passes::PreScene>::render(
    Passes::PreScene::Context& data, FrameGraph::FrameContext& context)
{
    auto& command_list = context.get_list();
    auto& scene = *context.graph->get_context<SceneInfo>().scene;

    SceneFrameManager::get().prepare(command_list, scene);

    if (HAL::Device::get().is_rtx_supported())
    {
        scene.raytrace_scene->update(command_list,
            (UINT)scene.raytrace->max_size(),
            scene.raytrace->buffer.get_resource_address(),
            false);
        RTX::get().prepare(command_list);
    }
}
