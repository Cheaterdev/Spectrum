#include "pch.h"
#include <array>

void gpu_cached_renderer::init_pipeline(global_pipeline& infos, Render::PipelineStateDesc default_pipeline, MaterialAsset* m, std::tuple<RENDER_TYPE, MESH_TYPE, std::shared_ptr<materials::material>> pass_id)
{
	materials::universal_material* mat = dynamic_cast<materials::universal_material*>(m);

	auto mat_id = mat->get_id();
	auto& info = infos.pipeline_infos[mat_id];

	if (!info.pipeline)
	{
		auto pass = mat->get_pass(std::get<0>(pass_id), std::get<1>(pass_id));

		m->set(std::get<0>(pass_id), std::get<1>(pass_id), default_pipeline);
		if (std::get<2>(pass_id))
		{
			std::get<2>(pass_id)->set(std::get<0>(pass_id), std::get<1>(pass_id), default_pipeline);
		}
		info.pipeline = Render::PipelineStateCache::get_cache(default_pipeline);
		info.topology = pass.get_topology();
	}
}

void gpu_cached_renderer::update(MeshRenderContext::ptr mesh_render_context)
{
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& list = *mesh_render_context->list;
for (auto& p : material_offsets)
	{
		materials::universal_material* mat = dynamic_cast<materials::universal_material*>(p.first);
		mat->update(mesh_render_context);

		//dafuq, mat_ids can be changed here
	}

	process_tasks();

	if (!boxes_instances.size())
		return;

	
	{
	//	auto timer = list.start(L"transmitting data");

		gpu_nodes.update_gpu(mesh_render_context->list);
		gpu_commands.update_gpu(mesh_render_context->list);
		boxes_instances.update_gpu(mesh_render_context->list);

	}

	buffers.new_frame();
	buffers.process_pipeline(direction);


	if (boxes_instances.size() > visible_id_buffer->get_count())
	{
		visible_id_buffer = std::make_shared<Render::StructuredBuffer<UINT>>(boxes_instances.size(), Render::counterType::NONE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		invisible_commands = std::make_shared<Render::StructuredBuffer<instance>>(boxes_instances.size(), Render::counterType::HELP_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		//	frustum_culled_commands = std::make_shared<Render::StructuredBuffer<instance>>(boxes_instances.size(), true, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		frustum_culled_commands = std::make_shared<Render::StructuredBuffer<unsigned int>>(boxes_instances.size(), Render::counterType::HELP_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		second_commands = std::make_shared<Render::StructuredBuffer<unsigned int>>(boxes_instances.size(), Render::counterType::HELP_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	}

}

void gpu_cached_renderer::render(MeshRenderContext::ptr mesh_render_context, scene_object::ptr obj)
{
	if ((mesh_render_context->render_mesh & mesh_type) == 0) return;
	if (!boxes_instances.size())
		return;

	auto& list = *mesh_render_context->list;
	auto& copy = mesh_render_context->list->get_copy();
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();


	//GPUMeshSignature signature;

	GPUMeshSignature<Signature> signature(&graphics);

//	signature.context = &graphics;


	instances_count = 0;
	//if (mesh_render_context->render_type == RENDER_TYPE::VOXEL)
	//	return;

	mesh_render_context->transformer = transformer;
	Render::PipelineStateDesc default_pipeline = mesh_render_context->pipeline;
	default_pipeline.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	default_pipeline.root_signature = my_signature;
	default_pipeline.rtv.enable_depth = true;
	default_pipeline.rasterizer.fill_mode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	default_pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	default_pipeline.blend.independ_blend = false;
	default_pipeline.blend.render_target[0].enabled = false;
	default_pipeline.vertex = shader;
	default_pipeline.rasterizer.conservative = false;


	if (mesh_render_context->render_type == RENDER_TYPE::VOXEL)
	{
		default_pipeline.geometry = voxel_geometry_shader;
		default_pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
		default_pipeline.rtv.enable_depth = false;
		default_pipeline.rasterizer.conservative = GetAsyncKeyState('B');

		voxel_info.voxel_min = { mesh_render_context->voxel_min,5 };
		voxel_info.voxel_size = { mesh_render_context->voxel_size,6 };
		voxel_info.voxel_map_size = { mesh_render_context->voxel_target_size ,0 };// ivec3(512, 512, 512);
		voxel_info.voxel_tiles_count = ivec4(mesh_render_context->voxel_tiles_count, 0);// ivec3(512, 512, 512);
		voxel_info.voxels_per_tile = ivec4(mesh_render_context->voxels_per_tile, 0);// ivec3(512, 512, 512);

	}



	mesh_render_context->pipeline = default_pipeline;

	if (mesh_render_context->override_material) {
		mesh_render_context->override_material->set(mesh_type, mesh_render_context);
		default_pipeline = mesh_render_context->pipeline;

	}
	mesh_render_context->begin();

//////////////////////////////////////////////////////////////	mesh_render_context->set_frame_data(mesh_render_context->cam->get_const_buffer());

	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (best_fit_normals)
		signature.best_fit[0]= best_fit_normals->get_texture()->texture_2d()->get_static_srv();


	auto current = std::make_tuple(mesh_render_context->render_type, mesh_render_context->render_mesh, mesh_render_context->override_material);
	auto& render_stage = type_pipelines[current];
	auto& pipelines = render_stage.pipelines[default_pipeline];
	auto& pipeline_infos = pipelines.pipeline_infos;

	{
		auto timer = list.start(L"fixing objects");

		std::lock_guard<std::mutex> g(pipelines.init_mutex);
		if (pipeline_infos.empty())
			for (auto m : material_offsets)
			{
				init_pipeline(pipelines, default_pipeline, m.first, current);
			}
	}




	std::shared_ptr<DrawInfo> statistic_infos;
	if (statistic.enabled)
		statistic_infos = std::make_shared<DrawInfo>();

	auto timer = list.start(L"gpu culling");

	unsigned int pipeline_per_cycle = min((unsigned int)pipeline_infos.size(), 16u);


	signature.mat_textures=textures;
	signature.vertex_unknown= gpu_nodes.get_gpu()->get_gpu_address();
	/////////////////////////////
	/////gpu frustum culling/////

	const bool check_occlusion = mesh_render_context->g_buffer && mesh_render_context->render_type == RENDER_TYPE::PIXEL;
	const bool frustum_culling = mesh_render_context->render_type == RENDER_TYPE::PIXEL;
	if (frustum_culling)
	{


		GPUCacheComputeSignature<Signature> compsig(&compute);

		auto timer = list.start(L"frustum culling");
		list.transition(invisible_commands, Render::ResourceState::COPY_DEST);
		list.clear_counter(invisible_commands);
		list.transition(invisible_commands, Render::ResourceState::UNORDERED_ACCESS);


		list.transition(visible_id_buffer, Render::ResourceState::UNORDERED_ACCESS);
		list.clear_uav(visible_id_buffer, visible_id_buffer->get_raw_uav());


		compute.set_pipeline(gpu_frustum_pipeline[check_occlusion]);
		list.transition(frustum_culled_commands, Render::ResourceState::UNORDERED_ACCESS);
		list.transition(frustum_culled_commands->help_buffer, Render::ResourceState::UNORDERED_ACCESS);
		list.clear_counter(frustum_culled_commands);
		//	compute.transition(frustum_culled_commands, Render::ResourceState::UNORDERED_ACCESS);
		list.transition(second_draw_dispatch, Render::ResourceState::UNORDERED_ACCESS);

		compsig.boxes_instances= boxes_instances.get_gpu()->get_gpu_address();
		compsig.frustum_culled_commands[0]= frustum_culled_commands->get_uav();
		compsig.visible_id_buffer[0]= visible_id_buffer->get_uav();

		//compute.set_constants(compsig.boxes_instances_size, boxes_instances.size(), 32);

		compsig.boxes_instances_size.set(boxes_instances.size(), 32);

		compsig.second_draw_dispatch[0]=second_draw_dispatch->get_uav();
		compsig.invisible_commands[0]= invisible_commands->get_uav();

		std::array<vec4, Frustum::planes::COUNT> planes;
		for (int i = 0; i < Frustum::planes::COUNT; ++i)
		{
			auto plane = mesh_render_context->cam->GetFrustumPlane(Frustum::planes(i));
			planes[i] = float4(plane.n, plane.d);

		}

		compsig.camera_planes.set_raw(planes.data(), planes.size() );
		//compsig.camera_planes = planes;

		compsig.camera_info= mesh_render_context->cam->get_const_buffer();
		compute.dispach(ivec3(boxes_instances.size(), 1, 1), ivec3(64 * 32, 1, 1));

		//	compute.transition_uav(frustum_culled_commands.get());
		copy.copy_buffer(second_draw_arguments.get(), sizeof(UINT) * 2, frustum_culled_commands->help_buffer.get(), 0, 4);


		compute.set_pipeline(dispatch_init_pipeline);
		compsig.frustum_culled_commands[0]= frustum_culled_commands->get_uav();
		compsig.visible_id_buffer[0]=second_draw_dispatch->get_uav();

		compute.dispach(ivec3(1, 1, 1), ivec3(1, 1, 1));


		if (statistic.enabled)
		{
			copy.read_buffer(frustum_culled_commands->help_buffer.get(), 0, 4, [this](const char* data, UINT64 count) {

				statistic.frustum_visible_commands(*reinterpret_cast<const UINT*>(data));
				});
		}
		list.transition(second_draw_dispatch, Render::ResourceState::INDIRECT_ARGUMENT);
	}

	/////////////////////////////

	if (!check_occlusion)
		render_buffers(pipelines, mesh_render_context, frustum_culling ? frustum_culled_commands->get_srv() : HandleTable());

	if (check_occlusion)
		for (auto stage = 0; stage < 2; stage++)
		{
		//	auto compsig = &GPUCacheComputeStateSignature::signature;
			GPUCacheComputeStateSignature<Signature> compsig(&compute);

			// get_invisible instances
			if (stage == 1)
			{
				auto timer = list.start(L"invisible gather");

				list.transition(invisible_commands, Render::ResourceState::UNORDERED_ACCESS);
				list.transition(invisible_commands->help_buffer, Render::ResourceState::UNORDERED_ACCESS);
				list.transition(second_commands, Render::ResourceState::UNORDERED_ACCESS);
				list.clear_counter(invisible_commands);
				list.clear_counter(second_commands);
				//		compute.transition(second_commands, Render::ResourceState::UNORDERED_ACCESS);

				list.transition(second_draw_dispatch, Render::ResourceState::UNORDERED_ACCESS);
				list.transition(frustum_culled_commands, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);

				compute.set_pipeline(gather_invisible);
				compsig.visible_id_buffer[0]= visible_id_buffer->get_srv();
				compsig.from_buffer[0]= frustum_culled_commands->get_srv();

				compsig.commands[0]=invisible_commands->get_uav();
				compsig.seconds_dispatch[0]= second_draw_dispatch->get_uav();
				compsig.instances= boxes_instances.get_gpu()->get_gpu_address();
				compsig.target[0]=second_commands->get_uav();


				compute.dispach(ivec3(1, 1, 1), ivec3(1, 1, 1));
				copy.copy_buffer(second_draw_arguments.get(), sizeof(UINT) * 2, invisible_commands->help_buffer.get(), 0, 4);

				{
					compute.set_pipeline(dispatch_init_pipeline);
					GPUCacheComputeSignature<Signature> dispatch_signature(&compute);

					dispatch_signature.frustum_culled_commands[0] = invisible_commands->get_uav();
					dispatch_signature.visible_id_buffer[0]=second_draw_dispatch->get_uav();

					compute.dispach(ivec3(1, 1, 1), ivec3(1, 1, 1));
				}



				if (statistic.enabled)
				{
					copy.read_buffer(invisible_commands->help_buffer.get(), 0, 4, [this](const char* data, UINT64 count) {

						statistic.second_stage_test(*reinterpret_cast<const UINT*>(data));
						});
				}

				list.transition(invisible_commands, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
				list.transition(second_draw_dispatch, Render::ResourceState::INDIRECT_ARGUMENT);
			}



			auto timer = list.start(stage == 0 ? L"render_1" : L"render_2");

			{
				auto timer = list.start(L"occluders draw");
				list.transition(visible_id_buffer, Render::ResourceState::UNORDERED_ACCESS);
				list.transition(second_draw_arguments, Render::ResourceState::INDIRECT_ARGUMENT);


				graphics.set_pipeline(state);
				graphics.set_index_buffer(index_buffer->get_index_buffer_view(true));

				signature.vertex_buffer=vertex_buffer->get_gpu_address();
				signature.mat_virtual_textures[0]= visible_id_buffer->get_static_uav();

			//	assert(false);

			//	mesh_render_context->g_buffer->set_downsapled(mesh_render_context);

				//TODO: DOWNSAMPLEED!
				mesh_render_context->g_buffer->HalfBuffer.hiZ_table.set(mesh_render_context);
				mesh_render_context->g_buffer->HalfBuffer.hiZ_table.set_window(graphics);

				signature.instance_buffer=invisible_commands->get_gpu_address();


				graphics.execute_indirect(
					second_draw_commands,
					1,
					second_draw_arguments.get());
			}


			render_buffers(pipelines, mesh_render_context, (stage == 0 ? frustum_culled_commands : second_commands)->get_srv());

			//	if (stage == 0)
			{
				//	auto timer = graphics.start(L"downsample_depth");
					// TODO: DOWNSAMPLE MEEEEEEEEEEEEE
				//mesh_render_context->g_buffer->downsample_depth(mesh_render_context->list);

				downsample_depth(mesh_render_context->list, *mesh_render_context->g_buffer);
			}
		}
	rendering = false;


	if (statistic.enabled)
		list.on_done([this, statistic_infos]() {
		statistic.draw_infos(*statistic_infos);
			});
}
void gpu_cached_renderer::render_buffers(global_pipeline& pipeline, MeshRenderContext::ptr mesh_render_context, const HandleTable& from_buffer)
{
	std::shared_ptr<DrawInfo> statistic_infos;
	if (statistic.enabled)
		statistic_infos = std::make_shared<DrawInfo>();
	auto& list = *mesh_render_context->list;

	auto& copy = mesh_render_context->list->get_copy();
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& pipeline_infos = pipeline.pipeline_infos;


	list.transition(frustum_culled_commands, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
	list.transition(invisible_commands, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
	list.transition(visible_id_buffer, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);

	unsigned int pipeline_per_cycle = min((unsigned int)pipeline_infos.size(), 16u);

	//if (mesh_render_context->override_material)
	//	pipeline_per_cycle = pipeline_infos.size();

	GPUCacheComputeStateSignature<Signature> compsig(&compute);
	GPUMeshSignature<Signature> signature(&graphics);

	for (unsigned int start_index = 0; start_index < pipeline_infos.size(); start_index += pipeline_per_cycle)
	{
		unsigned int current_count = min(pipeline_per_cycle, UINT(pipeline_infos.size() - start_index));

		//	if (mesh_render_context->override_material)
			//	current_count = 1;


		{


			buffers.start_write(compute.get_base(), current_count);
			auto timer = list.start(L"gather all");
			compute.set_pipeline(compute_state[(GATHER_TYPE::ALL_GOOD * !from_buffer.valid()) | (GATHER_TYPE::OVERRIDED_MATERIAL * (!!mesh_render_context->override_material))]);
			list.flush_transitions();

			compsig.visible_id_buffer[0]= visible_id_buffer->get_srv();
			compsig.instances= gpu_commands.get_gpu()->get_gpu_address();
			compsig.constants.set(UINT(boxes_instances.size()), start_index + 1, 32);

			compsig.commands[0]=buffers.get_handles();
			if (from_buffer.valid())
			{
				compsig.from_buffer[0] =  from_buffer;
				compute.execute_indirect(
					command_dispatch,
					1,
					second_draw_dispatch.get());
			}
			else
			{
				compsig.from_buffer[0] = boxes_instances.get_gpu()->get_srv();
				compute.dispach(ivec3(boxes_instances.size(), 1, 1), ivec3(64 * 32, 1, 1));
			}

		}

		{
			buffers.start_indirect(compute.get_base(), current_count);
			auto timer = list.start(L"ExecuteIndirect");

			for (unsigned int i = 0; i < current_count; i++)
			{
				auto& p = pipeline_infos[indirection[i + start_index]];
				auto timer = list.start(std::to_wstring(i).c_str());
				auto& command_buffer = buffers.get_buffer(i);
				//	if(!mesh_render_context->override_material)
				{
					graphics.set_pipeline(p.pipeline);
					graphics.set_topology(p.topology);
				}
				//	else
				{
					//		mesh_render_context->override_material->set(mesh_type, mesh_render_context);
						//	graphics.set_pipeline(mesh_render_context->override_material->)
				}
				if (mesh_render_context->g_buffer)
				{
					mesh_render_context->g_buffer->rtv_table.set(mesh_render_context);
					mesh_render_context->g_buffer->rtv_table.set_window(graphics);
				}


				if (mesh_render_context->render_type == RENDER_TYPE::VOXEL)
				{
					signature.vertex_consts_geometry.set_raw(voxel_info);
					signature.vertex_consts_vertex.set_raw(voxel_info);
				}


				graphics.execute_indirect(
					m_commandSignature,
					direction[indirection[i + start_index]].draw_count,
					command_buffer.get(),
					0,
					command_buffer->help_buffer.get(),
					0);

				if (statistic.enabled)
				{
					copy.read_buffer(command_buffer->help_buffer.get(), 0, 4, [i, statistic_infos](const char* data, UINT64 count) {

						auto& stat = *statistic_infos;

						stat[-1][i] = *reinterpret_cast<const UINT*>(data);

						//	Log::get() << "count " << stat << Log::endl;
						});
				}
			}
		}
	}

	if (statistic.enabled)
		list.on_done([this, statistic_infos]() {
		statistic.draw_infos(*statistic_infos);
			});
}
gpu_cached_renderer::gpu_cached_renderer(Scene::ptr scene, MESH_TYPE _mesh_type) :gpu_mesh_renderer(scene), mesh_type(_mesh_type)
{


	scene->on_changed.register_handler(this, [this](scene_object* object) {

		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;
		if (render_object->type != mesh_type) return;

		/*for (auto& m : render_object->overrided_material)
		{
			init_material((*m).get());
		}
		*/

		for (auto& r : render_object->rendering)
		{


			auto& mt = render_object->overrided_material[r.mesh->material];
			materials::universal_material* mat = dynamic_cast<materials::universal_material*>((*mt).get());

			auto mat_id = mat->get_id();

			//clear old
			{
				auto& info = direction[r.pipeline_id];

				info.draw_count--;
			}

			//insert new
			{

				auto& info = direction[mat_id];
				instance& inst = boxes_instances[r.global_index];
				inst.pipeline_id = info.index;

				r.pipeline_id = mat_id;

				info.draw_count++;
			}

			//fix command pixel buffer
			auto& command = gpu_commands[r.global_index];
			command.mat_texture_offsets.texture_offset =(UINT)material_offsets[mat];
			auto& material_cb = command.mat_const;
			if (mat->get_pixel_buffer())
				material_cb = mat->get_pixel_buffer()->get_gpu_address();
			else
				material_cb = 0;


		}

		});


	scene->on_moved.register_handler(this, [this](scene_object* object) {
		auto render_object = dynamic_cast<MeshAssetInstance*>(object);


		if (!render_object) return;
		if (render_object->type != mesh_type) return;


		for (auto& r : render_object->nodes)
		{
			gpu_nodes[r.global_index].world = r.asset_node->mesh_matrix * render_object->global_transform;
		}

		for (auto& r : render_object->rendering)
		{
			auto& inst = boxes_instances[r.global_index];

			auto& m = *rendering_list[r.global_index];

			inst.b = { m.mesh->primitive->get_min(), m.mesh->primitive->get_max() };
			inst.node = render_object->mesh_asset->nodes[m.node_index]->mesh_matrix * render_object->global_transform;
			inst.inv = inst.node;
			inst.inv.inverse();
		}



		});
	node_indices.set_index = [this](MeshAssetInstance::mesh_asset_node* r, int index, MeshAssetInstance* inst) {
		r->global_index = index;
		inst->nodes_indexes[r->asset_node->index] = index;
		gpu_nodes.resize(index + 1);

	};

	render_indices.set_index = [this](MeshAssetInstance::render_info* r, int index, MeshAssetInstance* inst) {
		r->global_index = index;
		if (rendering_list.size() <= index)rendering_list.resize(index + 1);
		boxes_instances.resize(index + 1);
		gpu_commands.resize(index + 1);

		rendering_list[index] = r;
		auto mt = inst->overrided_material[r->mesh->material];
		materials::universal_material* mat = dynamic_cast<materials::universal_material*>((*mt).get());


		r->node_global_index = inst->nodes_indexes[r->node_index];






		{
			auto mat_id = mat->get_id();
			auto& info = direction[mat_id];
			instance& inst = boxes_instances[index];
			inst.pipeline_id = info.index;
			inst.instance_id = index;
			info.draw_count++;

			r->pipeline_id = mat_id;
		}

		{
			IndirectCommand& command = gpu_commands[index];
			command.mat_texture_offsets.texture_offset =(UINT)material_offsets[mat];
			command.mat_texture_offsets.node_offset = r->node_global_index;
			command.draw_indirect.InstanceCount = 1;
			command.draw_indirect.StartInstanceLocation = 0;
			command.draw_indirect.IndexCountPerInstance = r->mesh->index_count;
			command.draw_indirect.StartIndexLocation = r->mesh->index_offset;
			command.draw_indirect.BaseVertexLocation = 0;// r->mesh->vertex_offset;
			command.index_buffer = inst->mesh_asset->index_buffer->get_index_buffer_view(true);
			command.vertex_buffer = inst->mesh_asset->vertex_buffer->get_gpu_address()+ r->mesh->vertex_offset*sizeof(Vertex);

			if (mat->get_pixel_buffer())
				command.mat_const = mat->get_pixel_buffer()->get_gpu_address();

		}
		//	commands_changed = true;
	};



	Render::ComputePipelineStateDesc compute_desc;
	compute_desc.root_signature = GPUCacheComputeSignature<SignatureCreator>().create_root();

	compute_desc.shader = Render::compute_shader::get_resource({ "shaders/occluder_frustum.hlsl", "CS", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES | D3DCOMPILE_SKIP_OPTIMIZATION ,{} });
	gpu_frustum_pipeline[0] = std::make_shared<Render::ComputePipelineState>(compute_desc);

	compute_desc.shader = Render::compute_shader::get_resource({ "shaders/occluder_frustum.hlsl", "CS", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES | D3DCOMPILE_SKIP_OPTIMIZATION ,{ { "OCCLUSION_TEST_ENABLED", "1" } } });
	gpu_frustum_pipeline[1] = std::make_shared<Render::ComputePipelineState>(compute_desc);

	compute_desc.shader = Render::compute_shader::get_resource({ "shaders/occluder_cs_dispatch_init.hlsl", "CS_Dispatch", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES | D3DCOMPILE_SKIP_OPTIMIZATION });
	dispatch_init_pipeline = std::make_shared<Render::ComputePipelineState>(compute_desc);

	frustum_culled_commands = std::make_shared<Render::StructuredBuffer<unsigned int>>(2044, Render::counterType::HELP_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	second_commands = std::make_shared<Render::StructuredBuffer<unsigned int>>(2044, Render::counterType::HELP_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);


	Render::PipelineStateDesc desc;
	//	Render::RootSignatureDesc root_desc;
	//	root_desc[0] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 1);
	//root_desc.set_sampler(0, 0, Render::ShaderVisibility::ALL, Render::Samplers::SamplerPointClampDesc);
	desc.root_signature = GPUMeshSignature<SignatureCreator>().create_root();// std::make_shared<Render::RootSignature>(root_desc);
	desc.vertex = Render::vertex_shader::get_resource({ "shaders/depth_render.hlsl", "VS", 0,{} });
	desc.pixel = Render::pixel_shader::get_resource({ "shaders/depth_render.hlsl", "PS", 0,{} });
	desc.rtv.rtv_formats = {};
	desc.rtv.enable_depth = true;
	desc.rtv.enable_depth_write = true;
	desc.rtv.ds_format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	desc.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	desc.rtv.func = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
	//	desc.blend.render_target[0].enabled = true;
	render_depth_state = Render::PipelineStateCache::get().get_cache(desc);


}
void gpu_cached_renderer::init_material(MaterialAsset* m)
{

	materials::universal_material* mat = dynamic_cast<materials::universal_material*>(m);


	auto handles = mat->get_texture_handles();
	auto it = material_offsets.find(mat);

	if (it != material_offsets.end())
	{
		if (it->second != -1)
			textures.release(it->second);
		//material_offsets[mat] = textures.size();
		//mat->place_textures(textures);
	}
	if (handles.size())
	{
		const auto index = material_offsets[mat] = textures.get_free(handles.size());

		for (auto i = 0; i < handles.size(); i++)
		{
			textures[index + i] = handles[i];
		}
	}
	else
	{
		material_offsets[mat] = -1;
	}
	const auto mat_id = mat->get_id();


	if (direction[mat_id].index == 0)
	{
		indirection.push_back(mat_id);
		direction[mat_id].index = (int)indirection.size();
	}

	for (auto& p : type_pipelines)
		for (auto& pips : p.second.pipelines)
		{
			init_pipeline(pips.second, pips.first, m, p.first);
		}

}

void gpu_cached_renderer::on_add(scene_object* object)
{
	const auto inst = dynamic_cast<MeshAssetInstance*>(object);

	if (inst->type != mesh_type) return;

	gpu_mesh_renderer::on_add(object);


	const auto render_object = dynamic_cast<Render::renderable*>(object);

	if (render_object)
	{
		auto inst = dynamic_cast<MeshAssetInstance*>(render_object);
/*
		for (auto& m : inst->overrided_material)
		{
			init_material((*m).get());
		}*/
		for (auto& r : inst->nodes)
		{
			node_indices.add(&r, inst);
		}
		for (auto& r : inst->rendering)
		{
			render_indices.add(&r, inst);
		}

	}


	//mesh_func();




}


void gpu_cached_renderer::on_remove(scene_object* object)
{
	auto inst = dynamic_cast<MeshAssetInstance*>(object);

	if (inst->type != mesh_type) return;

	gpu_mesh_renderer::on_remove(object);


	if (inst)
	{

		for (auto& r : inst->rendering)
		{
			render_indices.remove(r.global_index, inst);
		}
	}
}

