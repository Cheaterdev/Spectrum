#include "pch.h"

void mesh_renderer::render(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene)
{

	// return;
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& list = *mesh_render_context->list;

	auto timer = list.start(L"mesh_renderer");



	graphics.use_dynamic = false;
	//  std::list<MeshAssetInstance*> meshes;
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
	/*
	auto info = list.place_raw(scene->command_ids);
	auto srv = info.resource->create_view<FormattedBufferView<UINT, DXGI_FORMAT::DXGI_FORMAT_R32_UINT>>(*list.frame_resources, info.offset, info.size).get_srv();
	
	{
		auto timer = list.start(L"SceneData");
		Slots::SceneData sceneData;
		sceneData.GetNodes() = universal_nodes_manager::get().buffer->get_srv()[0];
		sceneData.GetMaterial_textures() = materials::universal_material_manager::get().get_textures();
		sceneData.GetVertexes() = universal_vertex_manager::get().buffer->get_srv()[0];
		sceneData.GetCommands() = srv;
		compiledScene = sceneData.compile(list);
	}
	*/
	{
		auto timer = list.start(L"set_index_buffer");
		graphics.set_index_buffer(universal_index_manager::get().buffer->get_index_buffer_view(true));
	}

	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	using render_list = std::map<size_t, std::vector<const MeshAssetInstance::render_info*>>;


	std::map<size_t, materials::Pipeline::ptr> pipelines;

	if (mesh_render_context->overrided_pipeline)
	{
		pipelines[0] = mesh_render_context->overrided_pipeline;
	}
	else
		for (auto mat : scene->mats)
		{
			pipelines[mat->get_id()] = mat->get_pipeline();
		}


	if (meshes_count == 0) return;

	//	auto timer = list.start(L"GatherMat");
	Slots::GatherPipelineGlobal gather_global;
	gather_global.GetMeshes_count() = meshes_count;
	gather_global.GetMaterials() = universal_material_info_part_manager::get().buffer->get_srv()[0];
	gather_global.GetMeshes() = scene->mesh_infos->buffer->get_srv()[0];


	auto gather_compiled = gather_global.compile(list);

	Slots::GatherPipeline gather;

	auto begin = pipelines.begin();
	auto end = begin;

	while (end != pipelines.end())
	{

		int total = 0;
		while (total < 8)
		{
			gather.GetPip_ids()[total] = end->second->get_id();
			gather.GetCommands()[total] = commands_buffer[total]->buffer->get_uav()[0];
			commands_buffer[total]->reserve(gather_global.GetMeshes_count());

			list.transition(commands_buffer[total]->buffer, ResourceState::UNORDERED_ACCESS);
			list.transition(commands_buffer[total]->buffer->help_buffer, ResourceState::UNORDERED_ACCESS);
			end++; total++;
			if (end == pipelines.end()) break;
		}

		for(int i = total;i<8;i++)
		{
			gather.GetPip_ids()[i] = std::numeric_limits<uint>::max();
			gather.GetCommands()[i] = commands_buffer[i]->buffer->get_uav()[0];
		}

		for (int i = 0; i < total; i++)
			list.clear_counter(commands_buffer[i]->buffer);


		{
			auto timer = list.start(L"GatherMats");
			compute.set_pipeline(gather_pipeline);

			gather_compiled.set(compute);
			gather.set(compute);


			compiledScene.set(compute);
			compiledFrame.set(compute);

			{
				auto timer = list.start(L"dispach");
				compute.dispach(ivec3{ meshes_count, 1 ,1 }, ivec3{ 64,1,1 });
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

	graphics.use_dynamic = true;
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
		compute_desc.shader = Render::compute_shader::get_resource({ "shaders/gather_pipeline.hlsl", "CS", D3DCOMPILE_SKIP_OPTIMIZATION, {} });
		gather_pipeline = std::make_shared<Render::ComputePipelineState>(compute_desc);
	}

	for (int i = 0; i < 8; i++)
		commands_buffer[i] = std::make_shared<virtual_gpu_buffer<command>>(1024, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
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




