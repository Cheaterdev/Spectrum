module Graphics:PreSceneSystem;
import <RenderSystem.h>;

using namespace HAL;
using namespace FrameGraph;

void PreSceneSystem::generate(FrameGraph::Graph& graph)
{
	graph.add_library_pass<Passes::PreScene>(
		[](auto& data, TaskBuilder& builder) -> bool
		{
			builder.create(data.scene, { 1 }, ResourceFlags::UnorderedAccess);
			return true;
		},
		[this](auto& data, FrameContext& context)
		{
			auto& command_list = context.get_list();

			SceneFrameManager::get().prepare(command_list, *scene);

			if (HAL::Device::get().is_rtx_supported())
			{
				scene->raytrace_scene->update(command_list,
					(UINT)scene->raytrace->max_size(),
					scene->raytrace->buffer.get_resource_address(),
					false);
				RTX::get().prepare(command_list);
			}
		});
}
