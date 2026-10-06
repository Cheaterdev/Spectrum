module Graphics:PreSceneSystem;
import RenderSystem;


import :Scene;
import :RTX;
import :MeshAsset;
import :FrameGraphContext;
import FrameGraph;

using namespace FrameGraph;

// setup() is fully generated (scene.prism's own [RunAlways]).

void PassDefault<Passes::Frame::PreScene>::pre_setup(FrameGraph::Graph& graph)
{
    if (RenderSystem::get().device().is_ray_query_supported())
    {
        auto& scene = *graph.get_context<SceneInfo>().scene;
        scene.raytrace_scene->new_frame();
    }
}

void PassDefault<Passes::Frame::PreScene>::render(
    Passes::Frame::PreScene::Context& data, FrameGraph::FrameContext& context)
{
    auto& command_list = context.get_list();
    auto& scene = *context.graph->get_context<SceneInfo>().scene;

    SceneFrameManager::get().prepare(command_list, scene);

    // The TLAS serves inline ray queries too; the RTX singleton's shader tables
    // only ray tracing pipelines.
    if (RenderSystem::get().device().is_ray_query_supported())
        scene.raytrace_scene->update(command_list,
            (UINT)scene.raytrace->max_size(),
            scene.raytrace->buffer.get_resource_address(),
            false);
    if (RenderSystem::get().device().is_rtx_supported())
        RTX::get().prepare(command_list);
}
