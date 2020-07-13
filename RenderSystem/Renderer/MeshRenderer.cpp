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
	graphics.use_dynamic = false;

	instances_count = 0;
	bool current_cpu_culling = use_cpu_culling && mesh_render_context->render_type == RENDER_TYPE::PIXEL;
	mesh_render_context->transformer = transformer;

	Render::PipelineStateDesc default_pipeline = mesh_render_context->pipeline;
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
		graphics.set_signature(get_Signature(Layouts::DefaultLayout));


		voxel_info.voxel_min = { mesh_render_context->voxel_min,5 };
		voxel_info.voxel_size = { mesh_render_context->voxel_size,6 };
		voxel_info.voxel_map_size = { mesh_render_context->voxel_target_size ,0 };// ivec3(512, 512, 512);
		voxel_info.voxel_tiles_count = ivec4(mesh_render_context->voxel_tiles_count, 0);// ivec3(512, 512, 512);
		voxel_info.voxels_per_tile = ivec4(mesh_render_context->voxels_per_tile, 0);// ivec3(512, 512, 512);
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
	
		if (best_fit_normals)
		{
			frameInfo.GetBestFitNormals() = best_fit_normals->get_texture()->texture_2d()->get_static_srv();
		}

		compiledFrame = frameInfo.compile(list);
	}


	UINT meshes_count = scene->command_ids.size();
	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	std::map<size_t, materials::Pipeline::ptr> pipelines_local;
	std::map<size_t, materials::Pipeline::ptr>* pipelines_ptr = &pipelines_local;
	if (mesh_render_context->overrided_pipeline)
	{
		pipelines_local[0] = mesh_render_context->overrided_pipeline;
	}
	else
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


		init_dispatch(mesh_render_context, scene->compiledGather);

		if (!gbuffer)
		{
			render_meshes(mesh_render_context, scene, pipelines, scene->compiledGather, compiledFrame,  true);
			return;
		}

		{

			auto timer = list.start(L"first stage");
			generate_boxes(mesh_render_context, scene, scene->compiledGather, true);

			draw_boxes(mesh_render_context, scene, compiledFrame);
			gather_rendered_boxes(mesh_render_context, scene, compiledFrame, true);

			init_dispatch(mesh_render_context, gather_visible);
			render_meshes(mesh_render_context, scene, pipelines, gather_visible, compiledFrame, true);
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
	graphics.use_dynamic = true;
}

void mesh_renderer::init_dispatch(MeshRenderContext::ptr mesh_render_context, Slots::GatherPipelineGlobal::Compiled & from)
{
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;

	{
		compute.set_pipeline(init_dispatch_pipeline);
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
	UINT meshes_count = scene->command_ids.size();
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	if (invisibleToo)
	{
		list.clear_counter(meshes_invisible_ids->buffer);
		list.clear_uav(meshes_invisible_ids->buffer, meshes_invisible_ids->buffer->get_uav()[0], ivec4{ 999,999,999,999 });

}

	{
		if (invisibleToo)
			compute.set_pipeline(gather_meshes_from_boxes_pipeline_invisible);
		else
			compute.set_pipeline(gather_meshes_from_boxes_pipeline);

		compiledScene.set(compute);
		compiledFrame.set(compute);
		scene->compiledGather.set(compute);
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
	UINT meshes_count = scene->command_ids.size();
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	{
		list.clear_counter(commands_boxes->buffer);
		list.clear_counter(meshes_ids->buffer);
	
		compute.set_pipeline(gather_boxes);
	
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
	UINT meshes_count = scene->command_ids.size();
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	gbuffer->HalfBuffer.hiZ_table.set(graphics);
	gbuffer->HalfBuffer.hiZ_table.set_window(graphics);

	graphics.set_pipeline(pipeline_boxes);
	graphics.set_index_buffer(index_buffer->get_index_buffer_view(true));

	draw_boxes_compiled.set(graphics);
		compiledScene.set(graphics);
		compiledFrame.set(graphics);

	list.clear_uav(visible_boxes->buffer, visible_boxes->buffer->get_uav()[0], ivec4{ 999,999,999,999 });

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

	UINT meshes_count = scene->command_ids.size();

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
			gather.GetPip_ids()[total] = end->second->get_id();
			gather.GetCommands()[total] = commands_buffer[total]->buffer->get_uav()[0];

			list.transition(commands_buffer[total]->buffer, ResourceState::UNORDERED_ACCESS);
			list.transition(commands_buffer[total]->buffer->help_buffer, ResourceState::UNORDERED_ACCESS);
			end++; total++;
			if (end == pipelines.end()) break;
		}

		for (int i = total; i < 8; i++)
		{
			gather.GetPip_ids()[i] = std::numeric_limits<uint>::max();
			gather.GetCommands()[i] = commands_buffer[i]->buffer->get_uav()[0];
		}

		for (int i = 0; i < total; i++)
			list.clear_counter(commands_buffer[i]->buffer);


		{
			auto timer = list.start(L"GatherMats");


			if (!needCulling)
			{
				compute.set_pipeline(gather_pipeline);
			}
			else
			{
				compute.set_pipeline(gather_pipeline_frustum);
			}

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
					mesh_render_context->flush_pipeline();
				}

				compiledScene.set(graphics);
				compiledFrame.set(graphics);

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
void mesh_renderer::iterate(MESH_TYPE mesh_type, std::function<void(Scene::ptr&)> f)
{

}


mesh_renderer::mesh_renderer()
{
	shader = Render::vertex_shader::get_resource({ "shaders/triangle.hlsl", "VS", 0, {} });
	voxel_geometry_shader = Render::geometry_shader::get_resource({ "shaders/voxelization.hlsl", "GS", 0, {} });

	best_fit_normals = EngineAssets::best_fit_normals.get_asset();




	{
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[] = {
			Slots::MaterialInfo::create_indirect(),
			Slots::MeshInfo::create_indirect(),
			Descriptors::DrawIndirect::create_indirect()
		};

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(command);
		TEST(Render::Device::get().get_native_device()->CreateCommandSignature(&commandSignatureDesc, get_Signature(Layouts::DefaultLayout)->get_native().Get(), IID_PPV_ARGS(&indirect_command_signature)));
	}



	{
		Render::ComputePipelineStateDesc compute_desc;
		compute_desc.root_signature = get_Signature(Layouts::DefaultLayout);
		compute_desc.shader = Render::compute_shader::get_resource({ "shaders/gather_pipeline.hlsl", "CS", D3DCOMPILE_OPTIMIZATION_LEVEL3, {} });
		gather_pipeline = std::make_shared<Render::ComputePipelineState>(compute_desc);

		compute_desc.shader = Render::compute_shader::get_resource({ "shaders/gather_pipeline.hlsl", "CS", D3DCOMPILE_OPTIMIZATION_LEVEL3, {{"CHECK_FRUSTUM",""}} });
		gather_pipeline_frustum = std::make_shared<Render::ComputePipelineState>(compute_desc);

	}

	{
		Render::ComputePipelineStateDesc compute_desc;
		compute_desc.root_signature = get_Signature(Layouts::DefaultLayout);
		compute_desc.shader = Render::compute_shader::get_resource({ "shaders/gather_pipeline.hlsl", "CS_boxes", D3DCOMPILE_OPTIMIZATION_LEVEL3, {{"CHECK_FRUSTUM",""}} });
		gather_boxes = std::make_shared<Render::ComputePipelineState>(compute_desc);
	}

	
	UINT max_meshes = 1024 * 1024;

	commands_boxes = std::make_shared<virtual_gpu_buffer<Table::BoxInfo::CB>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	visible_boxes = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	meshes_ids = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	meshes_invisible_ids = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	for (int i = 0; i < 8; i++)
		commands_buffer[i] = std::make_shared<virtual_gpu_buffer<command>>(max_meshes, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);


	{
		Render::PipelineStateDesc desc;
		desc.root_signature = get_Signature(Layouts::DefaultLayout);
		desc.vertex = Render::vertex_shader::get_resource({ "shaders/occluder.hlsl", "VS", 0,{} });
		desc.pixel = Render::pixel_shader::get_resource({ "shaders/occluder.hlsl", "PS", 0,{} });
		desc.rtv.rtv_formats = {};
		desc.rtv.enable_depth = true;
		desc.rtv.enable_depth_write = false;
		desc.rtv.ds_format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
		desc.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
		desc.rtv.func = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
		desc.rasterizer.conservative = true;
		//	desc.blend.render_target[0].enabled = true;
		pipeline_boxes = Render::PipelineStateCache::get().get_cache(desc);
	}


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
			D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[] = {
				Descriptors::DrawIndirect::create_indirect()
			};

			D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
			commandSignatureDesc.pArgumentDescs = argumentDescs;
			commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
			commandSignatureDesc.ByteStride = sizeof(command);
			TEST(Render::Device::get().get_native_device()->CreateCommandSignature(&commandSignatureDesc, nullptr, IID_PPV_ARGS(&boxes_command)));
		}
	}


	{
		dispatch_buffer = std::make_shared<Render::StructuredBuffer<DispatchArguments>>(1, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);


		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[] = {
					Descriptors::Dispatch::create_indirect()
		};

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(command);
		TEST(Render::Device::get().get_native_device()->CreateCommandSignature(&commandSignatureDesc, nullptr, IID_PPV_ARGS(&dispatch_command)));

	}


	dispatch_info = std::make_shared<Render::StructuredBuffer<Table::GatherPipelineGlobal::CB>>(1);
	{

		{
			Render::ComputePipelineStateDesc compute_desc;
			compute_desc.root_signature = get_Signature(Layouts::DefaultLayout);
			compute_desc.shader = Render::compute_shader::get_resource({ "shaders/occluder_cs_dispatch_init.hlsl", "CS", D3DCOMPILE_OPTIMIZATION_LEVEL3, {} });
			init_dispatch_pipeline = std::make_shared<Render::ComputePipelineState>(compute_desc);
		}
	} 
	{

		{
			Render::ComputePipelineStateDesc compute_desc;
			compute_desc.root_signature = get_Signature(Layouts::DefaultLayout);
			compute_desc.shader = Render::compute_shader::get_resource({ "shaders/gather_pipeline.hlsl", "CS_meshes_from_boxes", D3DCOMPILE_OPTIMIZATION_LEVEL3, {} });
			gather_meshes_from_boxes_pipeline = std::make_shared<Render::ComputePipelineState>(compute_desc);

			compute_desc.shader = Render::compute_shader::get_resource({ "shaders/gather_pipeline.hlsl", "CS_meshes_from_boxes", D3DCOMPILE_OPTIMIZATION_LEVEL3, {{"INVISIBLE",""}} });
			gather_meshes_from_boxes_pipeline_invisible = std::make_shared<Render::ComputePipelineState>(compute_desc);

			
		}
	}

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





DynamicSizeUAVBuffer::DynamicSizeUAVBuffer()
{
	clear_buffer = std::make_shared<Render::GPUBuffer>(sizeof(UINT));
	clear_buffer->set_data(UINT(0));
	handles = Render::DescriptorHeapManager::get().get_csu_static()->create_table(command_buffers.size());
	//   handles[1] = Render::DescriptorHeapManager::get().get_csu_static()->create_table(command_buffers.size());
	null_uav = DescriptorHeapManager::get().get_csu_static()->create_table(1);

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
	desc.Texture2D.PlaneSlice = 0;
	desc.Texture2D.MipSlice = 0;

	Device::get().get_native_device()->CreateUnorderedAccessView(nullptr, nullptr, &desc, null_uav[0].cpu);


	for (UINT i = 0; i < command_buffers.size(); i++)
	{
		handles[i].place(null_uav[0]);
	}


}


void DynamicSizeUAVBuffer::start_write(Render::CommandList& list, int binded_count)
{
	{
		//    auto timer = graphics.start(L"preparing buffers");
	//	for (int i = 0; i < binded_count; i++)
	//		list.transition(command_buffers[i]->help_buffer, Render::ResourceState::COPY_DEST);


		for (int i = 0; i < binded_count; i++)
			list.transition(command_buffers[i]->help_buffer, Render::ResourceState::UNORDERED_ACCESS);

		for (int i = 0; i < binded_count; i++)
			list.transition(command_buffers[i], Render::ResourceState::UNORDERED_ACCESS);


		for (int i = 0; i < binded_count; i++)
		{
			list.clear_counter(command_buffers[i]);
			//	list.copy_buffer(command_buffers[i].get(), command_buffers[i]->get_counter_offset(), clear_buffer.get(), 0, 4);
		}

	}
}

Render::HandleTable DynamicSizeUAVBuffer::get_handles()
{
	return handles;
}
void DynamicSizeUAVBuffer::start_indirect(Render::CommandList& list, int binded_count)
{
	{

		for (int i = 0; i < binded_count; i++)
			list.transition(command_buffers[i]->help_buffer, Render::ResourceState::INDIRECT_ARGUMENT);

		for (int i = 0; i < binded_count; i++)
			list.transition(command_buffers[i], Render::ResourceState::INDIRECT_ARGUMENT);


		//     auto timer = graphics.start(L"preparing buffers 2");
	}
}




