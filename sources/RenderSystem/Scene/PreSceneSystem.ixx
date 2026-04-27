export module Graphics:PreSceneSystem;

import <RenderSystem.h>;
import :Scene;
import :RTX;
import :MeshAsset;
import FrameGraph;


export class PreSceneSystem : public FrameGraph::GraphGenerator
{
	Scene::ptr scene;

public:
	// Template constructor: wires setup/render funcs onto the preScene pass
	// of any pipeline that exposes a 'preScene' member of type Passes::PreScene.
	template<typename TPipeline>
	explicit PreSceneSystem(TPipeline& pipeline, Scene::ptr _scene) : scene(std::move(_scene))
	{
		pipeline.preScene.setup_func = [](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			builder.create(data.scene, { 1 }, FrameGraph::ResourceFlags::UnorderedAccess);
			return true;
		};

		pipeline.preScene.render_func = [this](auto& data, FrameGraph::FrameContext& context)
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
		};
	}

	// Kept for GraphGenerator backward compatibility.
	virtual void generate(FrameGraph::Graph& graph) override;
};
