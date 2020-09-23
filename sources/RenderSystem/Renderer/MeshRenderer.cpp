#include "pch.h"

void mesh_renderer::render(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene)
{

	// return;
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;
	GBuffer* gbuffer = mesh_render_context->g_buffer;
	auto timer = list.start(L"mesh_renderer");

	instances_count = 0;
	bool current_cpu_culling = use_cpu_culling && mesh_render_context->render_type == RENDER_TYPE::PIXEL;
	mesh_render_context->transformer = transformer;

	Render::PipelineStateDesc &default_pipeline = mesh_render_context->pipeline;
	default_pipeline.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	default_pipeline.root_signature = get_Signature(Layouts::DefaultLayout);
	default_pipeline.rtv.enable_depth = true;
	default_pipeline.rasterizer.fill_mode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	default_pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	default_pipeline.blend.independ_blend = false;
	default_pipeline.blend.render_target[0].enabled = false;
	default_pipeline.vertex = shader;
	default_pipeline.rasterizer.conservative = GetAsyncKeyState('B') && mesh_render_context->render_type == RENDER_TYPE::VOXEL;

	if (mesh_render_context->render_type == RENDER_TYPE::VOXEL)
	{
		default_pipeline.geometry = voxel_geometry_shader;
		default_pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
		default_pipeline.rtv.enable_depth = false;
		default_pipeline.rasterizer.conservative = GetAsyncKeyState('B');

		scene->voxelization_compiled.set(graphics);
	}

	mesh_render_context->pipeline = default_pipeline;
	mesh_render_context->begin();

	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	Slots::FrameInfo::Compiled compiledFrame;

	{
		auto timer = Profiler::get().start(L"FrameInfo");
		Slots::FrameInfo frameInfo;

		auto camera = frameInfo.MapCamera();
		camera.cb = mesh_render_context->cam->camera_cb.current;
	
		auto prev = frameInfo.MapPrevCamera();
		prev.cb = mesh_render_context->cam->camera_cb.prev;


		if (best_fit_normals)
		{
			frameInfo.GetBestFitNormals() = best_fit_normals->get_texture()->texture_2d()->get_static_srv();
		}

		compiledFrame = frameInfo.compile(list);
	}


	UINT meshes_count = scene->command_ids[(int)mesh_render_context->render_mesh].size();
	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	std::map<size_t, materials::Pipeline::ptr> pipelines_local;
	std::map<size_t, materials::Pipeline::ptr>* pipelines_ptr = &pipelines_local;
	/*if (mesh_render_context->overrided_pipeline)
	{
		pipelines_local[0] = mesh_render_context->overrided_pipeline;
	}
	else*/
		pipelines_ptr = &scene->pipelines;

	std::map<size_t, materials::Pipeline::ptr>& pipelines= *pipelines_ptr;

	if (meshes_count == 0) return;

	commands_boxes->reserve(meshes_count);
	visible_boxes->reserve(meshes_count);
	meshes_ids->reserve(meshes_count);
	meshes_invisible_ids->reserve(meshes_count);
	for(int i =0;i<8;i++)
	commands_buffer[i]->reserve(meshes_count);

		compiledScene.set(compute);
		compiledFrame.set(compute);

		compiledScene.set(graphics);
		compiledFrame.set(graphics);


		init_dispatch(mesh_render_context, scene->compiledGather[(int)mesh_render_context->render_mesh]);

		if (!gbuffer)
		{
			render_meshes(mesh_render_context, scene, pipelines, scene->compiledGather[(int)mesh_render_context->render_mesh], compiledFrame, (mesh_render_context->render_type != RENDER_TYPE::VOXEL));
			return;
		}

		{

			auto timer = list.start(L"first stage");
			generate_boxes(mesh_render_context, scene, scene->compiledGather[(int)mesh_render_context->render_mesh], true);

			draw_boxes(mesh_render_context, scene, compiledFrame);
			gather_rendered_boxes(mesh_render_context, scene, compiledFrame, true);

			init_dispatch(mesh_render_context, gather_visible);
			render_meshes(mesh_render_context, scene, pipelines, gather_visible, compiledFrame, false);
			MipMapGenerator::get().downsample_depth(compute, gbuffer->depth, gbuffer->HalfBuffer.hiZ_depth_uav);
			MipMapGenerator::get().write_to_depth(graphics, gbuffer->HalfBuffer.hiZ_depth_uav, gbuffer->HalfBuffer.hiZ_depth);
		}
	
	{
		init_dispatch(mesh_render_context, gather_invisible);

		auto timer = list.start(L"second stage");
		generate_boxes(mesh_render_context, scene, gather_invisible, false);

		draw_boxes(mesh_render_context, scene, compiledFrame);
		gather_rendered_boxes(mesh_render_context, scene, compiledFrame, false);

		init_dispatch(mesh_render_context, gather_visible);
		render_meshes(mesh_render_context, scene, pipelines, gather_visible, compiledFrame, false);

		MipMapGenerator::get().downsample_depth(compute, gbuffer->depth, gbuffer->HalfBuffer.hiZ_depth_uav);
		MipMapGenerator::get().write_to_depth(graphics, gbuffer->HalfBuffer.hiZ_depth_uav, gbuffer->HalfBuffer.hiZ_depth);

	}
}

void mesh_renderer::init_dispatch(MeshRenderContext::ptr mesh_render_context, Slots::GatherPipelineGlobal::Compiled & from)
{
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;

	{
		compute.set_pipeline(GetPSO<PSOS::InitDispatch>());

		init_dispatch_compiled.set(compute);
		from.set(compute);
		compute.dispach(1, 1, 1);
	}
}

void  mesh_renderer::gather_rendered_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, Slots::FrameInfo::Compiled& compiledFrame, bool invisibleToo)
{

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;
	UINT meshes_count = scene->command_ids[(int)mesh_render_context->render_mesh].size();
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	if (invisibleToo)
	{
		list.clear_counter(meshes_invisible_ids->buffer);
		list.clear_uav(meshes_invisible_ids->buffer, meshes_invisible_ids->buffer->get_raw_uav(), ivec4{ 999,999,999,999 });

}

	{

		compute.set_pipeline(GetPSO<PSOS::GatherMeshes>(PSOS::GatherMeshes::Invisible.Use(invisibleToo)));
		compiledScene.set(compute);
		compiledFrame.set(compute);
		scene->compiledGather[(int)mesh_render_context->render_mesh].set(compute);
		gather_neshes_boxes_compiled.set(compute);

		graphics.execute_indirect(
			dispatch_command,
			1,
			dispatch_buffer.get());
	}

	list.transition_uav(meshes_ids->buffer.get());
	list.transition_uav(meshes_ids->buffer->help_buffer.get());
	if (invisibleToo)
	{
		list.transition_uav(meshes_invisible_ids->buffer.get());
		list.transition_uav(meshes_invisible_ids->buffer->help_buffer.get());
	}
}


void  mesh_renderer::generate_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, Slots::GatherPipelineGlobal::Compiled& gatherData, bool needCulling)
{
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;
	UINT meshes_count = scene->command_ids[(int)mesh_render_context->render_mesh].size();
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	{
		list.clear_counter(commands_boxes->buffer);
		list.clear_counter(meshes_ids->buffer);
	
		compute.set_pipeline(GetPSO<PSOS::GatherBoxes>());
		gather_boxes_compiled.set(compute);
		gatherData.set(compute);

		{
			auto timer = list.start(L"dispach");
			graphics.execute_indirect(
				dispatch_command,
				1,
				dispatch_buffer.get());
		}
	}

	copy.copy_buffer(draw_boxes_first.get(), sizeof(UINT), commands_boxes->buffer->help_buffer.get(), 0, 4);

	init_dispatch(mesh_render_context, gather_boxes_commands);


}

void  mesh_renderer::draw_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, Slots::FrameInfo::Compiled& compiledFrame)
{
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;

	GBuffer* gbuffer = mesh_render_context->g_buffer;
	UINT meshes_count = scene->command_ids[(int)mesh_render_context->render_mesh].size();
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	gbuffer->HalfBuffer.hiZ_table.set(graphics);
	gbuffer->HalfBuffer.hiZ_table.set_window(graphics);

	graphics.set_pipeline(GetPSO<PSOS::RenderBoxes>());
	graphics.set_index_buffer(index_buffer->get_index_buffer_view(true));

	draw_boxes_compiled.set(graphics);
		compiledScene.set(graphics);
		compiledFrame.set(graphics);

	list.clear_uav(visible_boxes->buffer, visible_boxes->buffer->get_raw_uav(), ivec4{ 999,999,999,999 });

	graphics.execute_indirect(
		boxes_command,
		1,
		draw_boxes_first.get());


	gbuffer->rtv_table.set(graphics);
	gbuffer->rtv_table.set_window(graphics);
}
void  mesh_renderer::render_meshes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene,std::map<size_t, materials::Pipeline::ptr>& pipelines, Slots::GatherPipelineGlobal::Compiled& gatherData, Slots::FrameInfo::Compiled& compiledFrame, bool needCulling)
{

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;

	UINT meshes_count = scene->command_ids[(int)mesh_render_context->render_mesh].size();

	Slots::GatherPipeline gather;
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	auto begin = pipelines.begin();
	auto end = begin;
	//	auto view = meshes_ids->buffer->help_buffer->create_view<Render::FormattedBufferView<UINT, DXGI_FORMAT::DXGI_FORMAT_R32_UINT>>(*list.frame_resources);

	graphics.set_index_buffer(universal_index_manager::get().buffer->get_index_buffer_view(true));
	while (end != pipelines.end())
	{

		int total = 0;
		while (total < 8)
		{
			((UINT*)gather.GetPip_ids())[total] = end->second->get_id();
			gather.GetCommands()[total] = commands_buffer[total]->buffer->get_uav()[0];

			list.transition(commands_buffer[total]->buffer, ResourceState::UNORDERED_ACCESS);
			list.transition(commands_buffer[total]->buffer->help_buffer, ResourceState::UNORDERED_ACCESS);
			end++; total++;
			if (end == pipelines.end()) break;
		}

		for (int i = total; i < 8; i++)
		{
			((UINT*)gather.GetPip_ids())[i] = std::numeric_limits<uint>::max();
			gather.GetCommands()[i] = commands_buffer[i]->buffer->get_uav()[0];
		}

		for (int i = 0; i < total; i++)
			list.clear_counter(commands_buffer[i]->buffer);


		{
			auto timer = list.start(L"GatherMats");

			compute.set_pipeline(GetPSO<PSOS::GatherPipeline>(PSOS::GatherPipeline::CheckFrustum.Use(needCulling)));
			gatherData.set(compute);
			gather.set(compute);
			compiledScene.set(compute);
			compiledFrame.set(compute);


			{
				auto timer = list.start(L"dispach");

				graphics.execute_indirect(
					dispatch_command,
					1,
					dispatch_buffer.get());
			}


			for (int i = 0; i < total; i++)
			{
				list.transition(commands_buffer[i]->buffer, ResourceState::INDIRECT_ARGUMENT);
				list.transition(commands_buffer[i]->buffer->help_buffer, ResourceState::INDIRECT_ARGUMENT);

			}

		}

		{
			auto timer = list.start(L"YO");
			int current = 0;


			for (auto it = begin; it != end; it++)
			{
				{
					auto timer = list.start(L"flush");
					it->second->set(mesh_render_context->render_type, mesh_render_context->render_mesh, mesh_render_context->pipeline);

					if (mesh_render_context->overrided_pipeline)
					{
						mesh_render_context->overrided_pipeline->set(mesh_render_context->render_type, mesh_render_context->render_mesh, mesh_render_context->pipeline);
					}
					mesh_render_context->flush_pipeline();
				}

				compiledScene.set(graphics);
				compiledFrame.set(graphics);
				scene->voxelization_compiled.set(graphics);

				{
					auto timer = list.start(L"execute_indirect");

					graphics.execute_indirect(
						indirect_command_signature,
						meshes_count,
						commands_buffer[current]->buffer.get(),
						0,
						commands_buffer[current]->buffer->help_buffer.get(),
						0
					);
				}
				current++;

			}
		}
	}
}
void mesh_renderer::iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)> f)
{

}


mesh_renderer::mesh_renderer()
{
	shader = Render::vertex_shader::get_resource({ "shaders/triangle.hlsl", "VS", 0, {} });
	voxel_geometry_shader = Render::geometry_shader::get_resource({ "shaders/voxelization.hlsl", "GS", 0, {} });

	best_fit_normals = EngineAssets::best_fit_normals.get_asset();


	indirect_command_signature = Render::IndirectCommand::create_command<Slots::MeshInfo, Slots::MaterialInfo, DrawIndexedArguments>(sizeof(command), get_Signature(Layouts::DefaultLayout));
	
	UINT max_meshes = 1024 * 1024;

	commands_boxes = std::make_shared<virtual_gpu_buffer<Table::BoxInfo::CB>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	visible_boxes = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	meshes_ids = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	meshes_invisible_ids = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	for (int i = 0; i < 8; i++)
		commands_buffer[i] = std::make_shared<virtual_gpu_buffer<command>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);



	{

		std::vector<unsigned int> data = { 3, 1, 0,
		2, 1, 3,
		0, 5, 4,
		1, 5, 0,
		3, 4, 7,
		0, 4, 3,
		1, 6, 5,
		2, 6, 1,
		2, 7, 6,
		3, 7, 2,
		6, 4, 5,
		7, 4, 6,
		};



		std::vector<vec4> verts(8);
		vec3 v0(-0.5, -0.5, 0.5);
		vec3 v1(0.5, 0.5, 0.5);
		verts[0] = vec3(-1.0f, 1.0f, -1.0f);
		verts[1] = vec3(1.0f, 1.0f, -1.0f);
		verts[2] = vec3(1.0f, 1.0f, 1.0f);
		verts[3] = vec3(-1.0f, 1.0f, 1.0f);
		verts[4] = vec3(-1.0f, -1.0f, -1.0f);
		verts[5] = vec3(1.0f, -1.0f, -1.0f);
		verts[6] = vec3(1.0f, -1.0f, 1.0f);
		verts[7] = vec3(-1.0f, -1.0f, 1.0f);
		index_buffer.reset(new Render::IndexBuffer(data));

		vertex_buffer.reset(new Render::StructuredBuffer<vec4>(8));
		vertex_buffer->set_raw_data(verts);

		draw_boxes_first = std::make_shared<Render::StructuredBuffer<DrawIndexedArguments>>(1);

		DrawIndexedArguments args;

		args.BaseVertexLocation = 0;
		args.IndexCountPerInstance = data.size();
		args.InstanceCount = 0;
		args.StartIndexLocation = 0;
		args.StartInstanceLocation = 0;

		draw_boxes_first->set_data(args);


		{

			boxes_command = Render::IndirectCommand::create_command<DrawIndexedArguments>(sizeof(command));

		}
	}


	{
		dispatch_buffer = std::make_shared<Render::StructuredBuffer<DispatchArguments>>(1, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		dispatch_command = Render::IndirectCommand::create_command<DispatchArguments>(sizeof(command));
	}


	dispatch_info = std::make_shared<Render::StructuredBuffer<Table::GatherPipelineGlobal::CB>>(1);

	{
		Slots::GatherPipelineGlobal gather;
		gather.GetCommands() = meshes_ids->buffer->get_srv()[0];
		gather_visible = gather.compile(StaticCompiledGPUData::get());
		gather_visible.cb = meshes_ids->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::GatherPipelineGlobal gather;
		gather.GetCommands() = meshes_invisible_ids->buffer->get_srv()[0];
		gather_invisible = gather.compile(StaticCompiledGPUData::get());
		gather_invisible.cb = meshes_invisible_ids->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::GatherPipelineGlobal gather;
		//gather.GetCommands() = meshes_invisible_ids->buffer->get_srv()[0];
		gather_boxes_commands = gather.compile(StaticCompiledGPUData::get());
		gather_boxes_commands.cb = commands_boxes->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::InitDispatch init_dispatch;
		init_dispatch.GetDispatch_data() = dispatch_buffer->get_uav()[0];
		init_dispatch_compiled = init_dispatch.compile(StaticCompiledGPUData::get());
	}

	{
		Slots::GatherMeshesBoxes gather_neshes_boxes;
		gather_neshes_boxes.GetInput_meshes() = commands_boxes->buffer->get_srv()[0];
		gather_neshes_boxes.GetVisible_boxes() = visible_boxes->buffer->get_srv()[0];
		gather_neshes_boxes.GetVisibleMeshes() = meshes_ids->buffer->get_uav()[0];
		gather_neshes_boxes.GetInvisibleMeshes() = meshes_invisible_ids->buffer->get_uav()[0];

		gather_neshes_boxes_compiled = gather_neshes_boxes.compile(StaticCompiledGPUData::get());
	}

	{
		Slots::DrawBoxes draw_boxes;
		draw_boxes.GetInput_meshes() = commands_boxes->buffer->get_srv()[0];
		draw_boxes.GetVisible_meshes() = visible_boxes->buffer->get_uav()[0];
		draw_boxes.GetVertices() = vertex_buffer->get_srv()[0];\

		draw_boxes_compiled = draw_boxes.compile(StaticCompiledGPUData::get());
	}
	{
		Slots::GatherBoxes gather;
		gather.GetCulledMeshes() = commands_boxes->buffer->get_uav()[0];
		gather.GetVisibleMeshes() = meshes_ids->buffer->get_uav()[0];

		gather_boxes_compiled = gather.compile(StaticCompiledGPUData::get());
	}
}
