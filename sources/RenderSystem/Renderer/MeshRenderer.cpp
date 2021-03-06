#include "pch.h"

void mesh_renderer::render(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene)
{

	// return;
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;
	GBuffer* gbuffer = mesh_render_context->g_buffer;
	PROFILE_GPU(L"mesh_renderer");

	instances_count = 0;

	Render::PipelineStateDesc& default_pipeline = mesh_render_context->pipeline;
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

		mesh_render_context->voxelization_compiled.set(graphics);
	}

	mesh_render_context->pipeline = default_pipeline;
	mesh_render_context->begin();

	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	UINT meshes_count = (UINT)scene->command_ids[(int)mesh_render_context->render_mesh].size();
	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	std::map<size_t, materials::Pipeline::ptr> pipelines_local;
	std::map<size_t, materials::Pipeline::ptr>* pipelines_ptr = &pipelines_local;
	/*if (mesh_render_context->overrided_pipeline)
	{
		pipelines_local[0] = mesh_render_context->overrided_pipeline;
	}
	else*/
	pipelines_ptr = &scene->pipelines;

	std::map<size_t, materials::Pipeline::ptr>& pipelines = *pipelines_ptr;

	if (meshes_count == 0) return;

	commands_boxes->reserve(list, meshes_count);
	visible_boxes->reserve(list, meshes_count);
	meshes_ids->reserve(list, meshes_count);
	meshes_invisible_ids->reserve(list, meshes_count);
	for (int i = 0; i < 8; i++)
		commands_buffer[i]->reserve(list, meshes_count);

	compiledScene.set(compute);
	compiledScene.set(graphics);

	init_dispatch(mesh_render_context, scene->compiledGather[(int)mesh_render_context->render_mesh]);

	if (!gbuffer || !use_gpu_occlusion)
	{
		render_meshes(mesh_render_context, scene, pipelines, scene->compiledGather[(int)mesh_render_context->render_mesh], (mesh_render_context->render_type != RENDER_TYPE::VOXEL));
		return;
	}

	{

		PROFILE_GPU(L"first stage");
		generate_boxes(mesh_render_context, scene, scene->compiledGather[(int)mesh_render_context->render_mesh], true);

		draw_boxes(mesh_render_context, scene);
		gather_rendered_boxes(mesh_render_context, scene, true);

		init_dispatch(mesh_render_context, gather_visible);

		render_meshes(mesh_render_context, scene, pipelines, gather_visible, false);
		MipMapGenerator::get().downsample_depth(compute, gbuffer->depth, gbuffer->HalfBuffer.hiZ_depth_uav);
		MipMapGenerator::get().write_to_depth(graphics, gbuffer->HalfBuffer.hiZ_depth_uav, gbuffer->HalfBuffer.hiZ_depth);
	}

	


	//if (false)// !GetAsyncKeyState(VK_F7))
	{
		init_dispatch(mesh_render_context, gather_invisible);

		PROFILE_GPU(L"second stage");
		generate_boxes(mesh_render_context, scene, gather_invisible, false);


		//	list.clear_counter(commands_boxes->buffer);
		//	list.clear_counter(meshes_ids->buffer);


		draw_boxes(mesh_render_context, scene);
		gather_rendered_boxes(mesh_render_context, scene, false);

		init_dispatch(mesh_render_context, gather_visible);
		render_meshes(mesh_render_context, scene, pipelines, gather_visible, false);

		MipMapGenerator::get().downsample_depth(compute, gbuffer->depth, gbuffer->HalfBuffer.hiZ_depth_uav);
		MipMapGenerator::get().write_to_depth(graphics, gbuffer->HalfBuffer.hiZ_depth_uav, gbuffer->HalfBuffer.hiZ_depth);

	}


}

void mesh_renderer::init_dispatch(MeshRenderContext::ptr mesh_render_context, Slots::GatherPipelineGlobal::Compiled& from)
{

	PROFILE_GPU(L"init_dispatch");
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;

	{
		compute.set_pipeline(GetPSO<PSOS::InitDispatch>());

		init_dispatch_compiled.set(compute);
		from.set(compute);

		
		compute.dispach(1, 1, 1);

		/*graphics.execute_indirect(
			dispatch_command,
			1,
			dispatch_buffer111.get());
		
		*/
		//		list.print_debug();
	}

	//	list.transition_uav(dispatch_buffer.get());
}

void  mesh_renderer::gather_rendered_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, bool invisibleToo)
{
	PROFILE_GPU(L"gather_rendered_boxes");

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;
	UINT meshes_count = (UINT)scene->command_ids[(int)mesh_render_context->render_mesh].size();
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	if (invisibleToo)
	{
		list.clear_counter(meshes_invisible_ids->buffer);
		list.clear_uav(meshes_invisible_ids->buffer->get_raw_uav(), ivec4{ 999,999,999,999 });

	}

	{

		compute.set_pipeline(GetPSO<PSOS::GatherMeshes>(PSOS::GatherMeshes::Invisible.Use(invisibleToo)));
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

//	meshes_ids->debug_print(list);

}


void  mesh_renderer::generate_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, Slots::GatherPipelineGlobal::Compiled& gatherData, bool needCulling)
{
	PROFILE_GPU(L"generate_boxes");

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;
	UINT meshes_count = (UINT)scene->command_ids[(int)mesh_render_context->render_mesh].size();
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;


	{

		list.clear_counter(commands_boxes->buffer);
		list.clear_counter(meshes_ids->buffer);


		compute.set_pipeline(GetPSO<PSOS::GatherBoxes>());
		gather_boxes_compiled.set(compute);
		gatherData.set(compute);

		{
			PROFILE_GPU(L"dispach");
			graphics.execute_indirect(
				dispatch_command,
				1,
				dispatch_buffer.get());
		}



	}

	copy.copy_buffer(draw_boxes_first.get(), sizeof(UINT), commands_boxes->buffer->help_buffer.get(), 0, 4);

	init_dispatch(mesh_render_context, gather_boxes_commands);


}

void  mesh_renderer::draw_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene)
{
	PROFILE_GPU(L"draw_boxes");

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;

	GBuffer* gbuffer = mesh_render_context->g_buffer;
	UINT meshes_count = (UINT)scene->command_ids[(int)mesh_render_context->render_mesh].size();

	gbuffer->HalfBuffer.hiZ_table.set(graphics);
	gbuffer->HalfBuffer.hiZ_table.set_window(graphics);

	graphics.set_pipeline(GetPSO<PSOS::RenderBoxes>());
	graphics.set_index_buffer(index_buffer->get_index_buffer_view(true));
	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	list.clear_uav(visible_boxes->buffer->get_raw_uav(), ivec4{ 999,999,999,999 });

	draw_boxes_compiled.set(graphics);

	graphics.execute_indirect(
		boxes_command,
		1,
		draw_boxes_first.get());


	gbuffer->rtv_table.set(graphics);
	gbuffer->rtv_table.set_window(graphics);
}
void  mesh_renderer::render_meshes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, std::map<size_t, materials::Pipeline::ptr>& pipelines, Slots::GatherPipelineGlobal::Compiled& gatherData, bool needCulling)
{
	PROFILE_GPU(L"render_meshes");

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;

	UINT meshes_count = (UINT)scene->command_ids[(int)mesh_render_context->render_mesh].size();

	Slots::GatherPipeline gather;

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
			gather.GetCommands()[total] = commands_buffer[total]->buffer->appendStructuredBuffer;

	/*		list.transition(commands_buffer[total]->buffer, ResourceState::UNORDERED_ACCESS);
			list.transition(commands_buffer[total]->buffer->help_buffer, ResourceState::UNORDERED_ACCESS);*/
			end++; total++;
			if (end == pipelines.end()) break;
		}

		for (int i = total; i < 8; i++)
		{
			((UINT*)gather.GetPip_ids())[i] = std::numeric_limits<uint>::max();
			gather.GetCommands()[i] = commands_buffer[i]->buffer->appendStructuredBuffer;
		}

		for (int i = 0; i < total; i++)
			list.clear_counter(commands_buffer[i]->buffer);


		{
			PROFILE_GPU(L"GatherMats");

			compute.set_pipeline(GetPSO<PSOS::GatherPipeline>(PSOS::GatherPipeline::CheckFrustum.Use(needCulling)));
			gatherData.set(compute);
			gather.set(compute);

			{
				PROFILE_GPU(L"dispach");

				compute.execute_indirect(
					dispatch_command,
					1,
					dispatch_buffer.get());
			}

		}


		{
			PROFILE_GPU(L"YO");
			int current = 0;


			for (auto it = begin; it != end; it++)
			{
				{
					PROFILE_GPU(L"flush");
					it->second->set(mesh_render_context->render_type, mesh_render_context->render_mesh, mesh_render_context->pipeline);

					if (mesh_render_context->overrided_pipeline)
					{
						mesh_render_context->overrided_pipeline->set(mesh_render_context->render_type, mesh_render_context->render_mesh, mesh_render_context->pipeline);
					}
					mesh_render_context->flush_pipeline();
				}

				{
					PROFILE_GPU(L"compile");

					if (mesh_render_context->render_type == RENDER_TYPE::VOXEL)
					mesh_render_context->voxelization_compiled.set(graphics);
				}
				{
					PROFILE_GPU(L"execute_indirect");

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


mesh_renderer::mesh_renderer():VariableContext(L"mesh_renderer")
{
	shader = Render::vertex_shader::get_resource({ "shaders/triangle.hlsl", "VS", 0, {} });
	voxel_geometry_shader = Render::geometry_shader::get_resource({ "shaders/voxelization.hlsl", "GS", 0, {} });

	best_fit_normals = EngineAssets::best_fit_normals.get_asset();


	indirect_command_signature = Render::IndirectCommand::create_command<Slots::MeshInfo, Slots::MaterialInfo, DrawIndexedArguments>(sizeof(Underlying<command>), get_Signature(Layouts::DefaultLayout));

	UINT max_meshes = 1024 * 1024;

	commands_boxes = std::make_shared<virtual_gpu_buffer<Table::BoxInfo>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	visible_boxes = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	meshes_ids = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	meshes_invisible_ids = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	for (int i = 0; i < 8; i++)
		commands_buffer[i] = std::make_shared<virtual_gpu_buffer<Table::CommandData>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);


	meshes_ids->buffer->get_native()->SetName(L"meshes_ids");
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
		args.IndexCountPerInstance = (UINT)data.size();
		args.InstanceCount = 0;
		args.StartIndexLocation = 0;
		args.StartInstanceLocation = 0;

		draw_boxes_first->set_data(args);


		{

			boxes_command = Render::IndirectCommand::create_command<DrawIndexedArguments>(sizeof(Underlying<command>));

		}
	}


	{
		dispatch_buffer = std::make_shared<Render::StructuredBuffer<DispatchArguments>>(1, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		dispatch_command = Render::IndirectCommand::create_command<DispatchArguments>(sizeof(Underlying<command>));
	}

	{
		dispatch_buffer111 = std::make_shared<Render::StructuredBuffer<DispatchArguments>>(1, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		DispatchArguments args;
		args.ThreadGroupCountX = 1;
		args.ThreadGroupCountY = 1;
		args.ThreadGroupCountZ = 1;
		dispatch_buffer111->set_data(args);
	}
	{
		Slots::GatherPipelineGlobal gather;
		gather.GetCommands() = meshes_ids->buffer->create_view<FormattedBufferView<UINT, DXGI_FORMAT::DXGI_FORMAT_R32_UINT>>(StaticCompiledGPUData::get()).srv_handle;

		gather.GetMeshes_count() = meshes_ids->buffer->structuredBufferCount;

		gather_visible = gather.compile(StaticCompiledGPUData::get());
		//	gather_visible.cb = meshes_ids->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::GatherPipelineGlobal gather;
		gather.GetCommands() = meshes_invisible_ids->buffer->create_view<FormattedBufferView<UINT, DXGI_FORMAT::DXGI_FORMAT_R32_UINT>>(StaticCompiledGPUData::get()).srv_handle;
		gather.GetMeshes_count() = meshes_invisible_ids->buffer->structuredBufferCount;
		gather_invisible = gather.compile(StaticCompiledGPUData::get());
		//	gather_invisible.cb = meshes_invisible_ids->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::GatherPipelineGlobal gather;
		gather.GetMeshes_count() = commands_boxes->buffer->structuredBufferCount;

		//gather.GetCommands() = // supposed to be null
		gather_boxes_commands = gather.compile(StaticCompiledGPUData::get());
		//gather_boxes_commands.cb = commands_boxes->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::InitDispatch init_dispatch;
		init_dispatch.GetDispatch_data() = dispatch_buffer->rwStructuredBuffer;
		init_dispatch_compiled = init_dispatch.compile(StaticCompiledGPUData::get());
	}

	{
		Slots::GatherMeshesBoxes gather_neshes_boxes;
		gather_neshes_boxes.GetInput_meshes() = commands_boxes->buffer->structuredBuffer;
		gather_neshes_boxes.GetVisible_boxes() = visible_boxes->buffer->structuredBuffer;
		gather_neshes_boxes.GetVisibleMeshes() = meshes_ids->buffer->appendStructuredBuffer;
		gather_neshes_boxes.GetInvisibleMeshes() = meshes_invisible_ids->buffer->appendStructuredBuffer;

		gather_neshes_boxes_compiled = gather_neshes_boxes.compile(StaticCompiledGPUData::get());
	}

	{
		Slots::DrawBoxes draw_boxes;
		draw_boxes.GetInput_meshes() = commands_boxes->buffer->structuredBuffer;
		draw_boxes.GetVisible_meshes() = visible_boxes->buffer->rwStructuredBuffer;
		draw_boxes.GetVertices() = vertex_buffer->structuredBuffer;

		draw_boxes_compiled = draw_boxes.compile(StaticCompiledGPUData::get());
	}
	{
		Slots::GatherBoxes gather;
		gather.GetCulledMeshes() = commands_boxes->buffer->appendStructuredBuffer;
		gather.GetVisibleMeshes() = meshes_ids->buffer->appendStructuredBuffer;

		gather_boxes_compiled = gather.compile(StaticCompiledGPUData::get());
	}
}

