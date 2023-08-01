module Graphics:MeshRenderer;

import <RenderSystem.h>;
import :EngineAssets;
import :MipMapGenerator;

import HAL;

using namespace HAL;
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
		
	mesh_render_context->begin();

	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	UINT meshes_count = (UINT)scene->command_ids[(int)mesh_render_context->render_mesh].size();
	graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);


	std::map<size_t, materials::Pipeline::ptr> pipelines_local;
	std::map<size_t, materials::Pipeline::ptr>* pipelines_ptr = &pipelines_local;

	pipelines_ptr = &scene->pipelines;

	std::map<size_t, materials::Pipeline::ptr>& pipelines = *pipelines_ptr;

	if (meshes_count == 0) return;

	commands_boxes->reserve(list, meshes_count);
	visible_boxes->reserve(list, meshes_count);
	meshes_ids->reserve(list, meshes_count);
	meshes_invisible_ids->reserve(list, meshes_count);
	for (int i = 0; i < 8; i++)
		commands_buffer[i]->reserve(list, meshes_count);

	graphics.set(compiledScene);
	compute.set(compiledScene);

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
		compute.set_pipeline<PSOS::InitDispatch>();

		compute.set(init_dispatch_compiled);
		compute.set(from);

		compute.dispatch(1, 1, 1);
	}
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

		list.clear_uav(meshes_invisible_ids->buffer.counter_view.rwRAW);
		list.clear_uav(meshes_invisible_ids->buffer.rwRAW, ivec4{ 999,999,999,999 });

	}

	{

		compute.set_pipeline<PSOS::GatherMeshes>(PSOS::GatherMeshes::Invisible.Use(invisibleToo));
		compute.set(scene->compiledGather[(int)mesh_render_context->render_mesh]);
		compute.set(gather_neshes_boxes_compiled);

		graphics.execute_indirect(
			dispatch_command,
			1,
			dispatch_buffer.resource.get());
	}


	/*list.transition_uav(meshes_ids->buffer->resource.get());
	list.transition_uav(meshes_ids->buffer->help_buffer->resource.get());
	if (invisibleToo)
	{
		list.transition_uav(meshes_invisible_ids->buffer->resource.get());
		list.transition_uav(meshes_invisible_ids->buffer->help_buffer->resource.get());
	}*/

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
		list.clear_uav(commands_boxes->buffer.counter_view.rwRAW);
		list.clear_uav(meshes_ids->buffer.counter_view.rwRAW);


		compute.set_pipeline<PSOS::GatherBoxes>();
		compute.set(gather_boxes_compiled);
		compute.set(gatherData);

		{
			PROFILE_GPU(L"dispatch");
			graphics.execute_indirect(
				dispatch_command,
				1,
				dispatch_buffer.resource.get());
		}



	}

	copy.copy_buffer(draw_boxes_first.resource.get(), sizeof(UINT), commands_boxes->buffer.get_counter_buffer().get(),commands_boxes->buffer.get_counter_offset(), 4);

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

	graphics.set_rtv(gbuffer->HalfBuffer.compiled);

	graphics.set_pipeline<PSOS::RenderBoxes>();
	graphics.set_index_buffer(index_buffer->get_index_buffer_view());
	graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);


	list.clear_uav(visible_boxes->buffer.rwRAW, ivec4{ 999,999,999,999 });

	graphics.set(draw_boxes_compiled);

	graphics.execute_indirect(
		boxes_command,
		1,
		draw_boxes_first.resource.get());


	graphics.set_rtv(gbuffer->compiled);
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




	graphics.set_index_buffer(HAL::Views::IndexBuffer());// universal_index_manager::get().buffer->get_index_buffer_view(true));
	while (end != pipelines.end())
	{

		int total = 0;
		while (total < 8)
		{
			((UINT*)gather.GetPip_ids())[total] = end->second->get_id();
			gather.GetCommands()[total] = commands_buffer[total]->buffer.appendStructuredBuffer;

			/*		list.transition(commands_buffer[total]->buffer, ResourceState::UNORDERED_ACCESS);
					list.transition(commands_buffer[total]->buffer->help_buffer, ResourceState::UNORDERED_ACCESS);*/
			end++; total++;
			if (end == pipelines.end()) break;
		}

		for (int i = total; i < 8; i++)
		{
			((UINT*)gather.GetPip_ids())[i] = std::numeric_limits<uint>::max();
			gather.GetCommands()[i] = commands_buffer[i]->buffer.appendStructuredBuffer;
		}

		for (int i = 0; i < total; i++)
					list.clear_uav(commands_buffer[i]->buffer.counter_view.rwRAW);



		{
			PROFILE_GPU(L"GatherMats");

			compute.set_pipeline<PSOS::GatherPipeline>(PSOS::GatherPipeline::CheckFrustum.Use(needCulling));
			compute.set(gatherData);
			compute.set(gather);

			{
				PROFILE_GPU(L"dispatch");

				compute.execute_indirect(
					dispatch_command,
					1,
					dispatch_buffer.resource.get());
			}

		}

		//if(0)
		{
			PROFILE_GPU(L"YO");
			int current = 0;

{
					PROFILE_GPU(L"compile");

					if (mesh_render_context->render_type == RENDER_TYPE::VOXEL)
						graphics.set(mesh_render_context->voxelization_compiled);
				}

			for (auto it = begin; it != end; it++)
			{
				{
					PROFILE_GPU(L"flush");
			
					it->second->set(mesh_render_context->render_type, mesh_render_context->render_mesh, graphics);
				}

				{
					PROFILE_GPU(L"execute_indirect");

					auto t=commands_buffer[current]->buffer.get_data_offset_in_bytes();
					graphics.execute_indirect(
						indirect_command_signature,
						meshes_count,
						commands_buffer[current]->buffer.resource.get(),
						commands_buffer[current]->buffer.get_data_offset() +  t,
						commands_buffer[current]->buffer.get_counter_buffer().get(),
						commands_buffer[current]->buffer.get_counter_offset()
					);

				}

				//list.get_copy().read_buffer(commands_buffer[current]->buffer.get(),0, sizeof(Table::CommandData), [](const char* data, UINT64 size){

				//	std::vector<Table::CommandData> v; v.resize(size/sizeof(Table::CommandData));
				//memcpy(v.data(),data,size);

				//auto &heap = *Device::get().get_descriptor_heap_factory().get_cbv_srv_uav_heap();
				//	
				//auto mesh = heap.get_resource_info(v[0].mesh_cb);
				//auto material =  heap.get_resource_info(v[0].material_cb);


				//	});
				current++;

			}
		}
	}
}
void mesh_renderer::iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)> f)
{

}


mesh_renderer::mesh_renderer() :VariableContext(L"mesh_renderer")
{
	best_fit_normals = EngineAssets::best_fit_normals.get_asset();


	indirect_command_signature = HAL::IndirectCommand::create_command_layout<Slots::MeshInfo,Slots::MeshInstanceInfo, Slots::MaterialInfo, DispatchMeshArguments>(HAL::Device::get(), Layouts::DefaultLayout);

	UINT max_meshes = 1024 * 1024;

	commands_boxes = std::make_shared<virtual_gpu_buffer<Table::BoxInfo>>(max_meshes, counterType::HELP_BUFFER, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	visible_boxes = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	meshes_ids = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::HELP_BUFFER, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	meshes_invisible_ids = std::make_shared<virtual_gpu_buffer<UINT>>(max_meshes, counterType::HELP_BUFFER, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	for (int i = 0; i < 8; i++)
		commands_buffer[i] = std::make_shared<virtual_gpu_buffer<Table::CommandData>>(max_meshes, counterType::HELP_BUFFER, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);

		auto list = (HAL::Device::get().get_upload_list());
			
	//meshes_ids->buffer->get_native()->SetName(L"meshes_ids");
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
		verts[0] = vec4(-1.0f, 1.0f, -1.0f, 0);
		verts[1] = vec4(1.0f, 1.0f, -1.0f, 0);
		verts[2] = vec4(1.0f, 1.0f, 1.0f, 0);
		verts[3] = vec4(-1.0f, 1.0f, 1.0f, 0);
		verts[4] = vec4(-1.0f, -1.0f, -1.0f, 0);
		verts[5] = vec4(1.0f, -1.0f, -1.0f, 0);
		verts[6] = vec4(1.0f, -1.0f, 1.0f, 0);
		verts[7] = vec4(-1.0f, -1.0f, 1.0f, 0);
		index_buffer = HAL::IndexBuffer::make_buffer(data);

		vertex_buffer = HAL::StructuredBufferView<vec4>(8);
		list->get_copy().update(vertex_buffer, 0, verts);
	
		draw_boxes_first = HAL::StructuredBufferView<DrawIndexedArguments>(1);

		DrawIndexedArguments args;

		args.BaseVertexLocation = 0;
		args.IndexCountPerInstance = (UINT)data.size();
		args.InstanceCount = 0;
		args.StartIndexLocation = 0;
		args.StartInstanceLocation = 0;

		list->get_copy().update(draw_boxes_first, 0, std::span{static_cast<D3D12_DRAW_INDEXED_ARGUMENTS*>(&args),1});
	

		{

			boxes_command = HAL::IndirectCommand::create_command<DrawIndexedArguments>(HAL::Device::get());

		}
	}


	{
		dispatch_buffer = HAL::StructuredBufferView<DispatchArguments>(1, counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
		dispatch_command = HAL::IndirectCommand::create_command<DispatchArguments>(HAL::Device::get());
	}

	{
		dispatch_buffer111 = HAL::StructuredBufferView<DispatchArguments>(1, counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
		DispatchArguments args;
		args.ThreadGroupCountX = 1;
		args.ThreadGroupCountY = 1;
		args.ThreadGroupCountZ = 1;
		
		list->get_copy().update(dispatch_buffer111,0,std::span{static_cast<D3D12_DISPATCH_ARGUMENTS*>(&args),1});
	}
	{
		Slots::GatherPipelineGlobal gather;
		gather.GetCommands() = meshes_ids->buffer.resource->create_view<HAL::FormattedBufferView<UINT, HAL::Format::R32_UINT>>(HAL::Device::get().get_static_gpu_data()).buffer;

		gather.GetMeshes_count() = meshes_ids->buffer.counter_view.structuredBuffer;

		gather_visible = gather.compile(HAL::Device::get().get_static_gpu_data());
		//	gather_visible = meshes_ids->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::GatherPipelineGlobal gather;
		gather.GetCommands() = meshes_invisible_ids->buffer.resource->create_view<HAL::FormattedBufferView<UINT, HAL::Format::R32_UINT>>(HAL::Device::get().get_static_gpu_data()).buffer;
		gather.GetMeshes_count() = meshes_invisible_ids->buffer.counter_view.structuredBuffer;
		gather_invisible = gather.compile(HAL::Device::get().get_static_gpu_data());
		//	gather_invisible = meshes_invisible_ids->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::GatherPipelineGlobal gather;
		gather.GetMeshes_count() = commands_boxes->buffer.counter_view.structuredBuffer;

		//gather.GetCommands() = // supposed to be null
		gather_boxes_commands = gather.compile(HAL::Device::get().get_static_gpu_data());
		//gather_boxes_commands = commands_boxes->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::InitDispatch init_dispatch;
		init_dispatch.GetDispatch_data() = dispatch_buffer.rwStructuredBuffer;
		init_dispatch_compiled = init_dispatch.compile(HAL::Device::get().get_static_gpu_data());
	}

	{
		Slots::GatherMeshesBoxes gather_neshes_boxes;
		gather_neshes_boxes.GetInput_meshes() = commands_boxes->buffer.structuredBuffer;
		gather_neshes_boxes.GetVisible_boxes() = visible_boxes->buffer.structuredBuffer;
		gather_neshes_boxes.GetVisibleMeshes() = meshes_ids->buffer.appendStructuredBuffer;
		gather_neshes_boxes.GetInvisibleMeshes() = meshes_invisible_ids->buffer.appendStructuredBuffer;

		gather_neshes_boxes_compiled = gather_neshes_boxes.compile(HAL::Device::get().get_static_gpu_data());
	}

	{
		Slots::DrawBoxes draw_boxes;
		draw_boxes.GetInput_meshes() = commands_boxes->buffer.structuredBuffer;
		draw_boxes.GetVisible_meshes() = visible_boxes->buffer.rwStructuredBuffer;
		draw_boxes.GetVertices() = vertex_buffer.structuredBuffer;

		draw_boxes_compiled = draw_boxes.compile(HAL::Device::get().get_static_gpu_data());
	}
	{
		Slots::GatherBoxes gather;
		gather.GetCulledMeshes() = commands_boxes->buffer.appendStructuredBuffer;
		gather.GetVisibleMeshes() = meshes_ids->buffer.appendStructuredBuffer;

		gather_boxes_compiled = gather.compile(HAL::Device::get().get_static_gpu_data());
	}


			list->execute_and_wait();

}

