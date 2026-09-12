module Graphics:PreSceneSystem;
import RenderSystem;


import :Scene;
import :RTX;
import :MeshAsset;
import :FrameGraphContext;
import FrameGraph;

using namespace FrameGraph;

// setup() is fully generated (scene.sig's own [RunAlways]).

void PassDefault<Passes::PreScene>::pre_setup(FrameGraph::Graph& graph)
{
    if (RenderSystem::get().device().is_rtx_supported())
    {
        auto& scene = *graph.get_context<SceneInfo>().scene;
        scene.raytrace_scene->new_frame();
    }
}

void PassDefault<Passes::PreScene>::render(
    Passes::PreScene::Context& data, FrameGraph::FrameContext& context)
{
    auto& command_list = context.get_list();
    auto& scene = *context.graph->get_context<SceneInfo>().scene;

    SceneFrameManager::get().prepare(command_list, scene);

    if (RenderSystem::get().device().is_rtx_supported())
    {
        scene.raytrace_scene->update(command_list,
            (UINT)scene.raytrace->max_size(),
            scene.raytrace->buffer.get_resource_address(),
            false);
        RTX::get().prepare(command_list);
    }
}
