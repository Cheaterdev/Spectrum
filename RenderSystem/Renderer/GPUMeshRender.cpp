#include "pch.h"

void gpu_mesh_renderer::render(MeshRenderContext::ptr mesh_render_context, scene_object::ptr obj)
{
	// return;
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	//  std::list<MeshAssetInstance*> meshes;
	instances_count = 0;
	if (mesh_render_context->render_type == RENDER_TYPE::VOXEL)
		return;
	bool current_cpu_culling = use_cpu_culling && mesh_render_context->render_type == RENDER_TYPE::PIXEL;
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

	mesh_render_context->pipeline = default_pipeline;
	mesh_render_context->begin();
	mesh_render_context->set_frame_data(mesh_render_context->cam->get_const_buffer());

	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	if (best_fit_normals)
		graphics.set_dynamic(12, 0, best_fit_normals->get_texture()->texture_2d()->get_static_srv());
	using render_list = std::map<int, std::vector<MeshAssetInstance::render_info>>;
	//std::map<int, int> pipeline_ids;
	std::map<int, pipeline_draws> pipeline_infos;
	std::vector<int> indirection;
	std::map<materials::universal_material*, int> material_offsets;
	std::vector<Handle> textures;
	struct draw_info
	{
		UINT pipeline_id;
		UINT texture_offset;
	};
	auto get_draw_info = [&](materials::universal_material * mat, materials::render_pass * pass)
	{
		// mat->update(mesh_render_context);
		auto& info = pipeline_infos[pass->get_pipeline_id()];
		draw_info res;
		res.pipeline_id = 0;
		res.texture_offset = 0;



		if (!info.pipeline)
		{
			///////////////////////			info.index = pipeline_infos.size();
				//	mat->set_pipeline_states(default_pipeline);
			default_pipeline.pixel = pass->ps_shader;
			default_pipeline.domain = pass->ds_shader;
			default_pipeline.hull = pass->hs_shader;

			default_pipeline.topology = pass->ds_shader ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			info.pipeline = Render::PipelineStateCache::get_cache(default_pipeline);

			if (!info.pipeline)
				Log::get() << "error" << Log::endl;

			indirection.push_back(pass->get_pipeline_id());
			info.topology = pass->get_topology();
		}

///////////////////////		res.pipeline_id = info.index;
///////////////////////		info.draw_count++;
		auto it = material_offsets.find(mat);

		if (it == material_offsets.end())
		{
			res.texture_offset = textures.size();
			material_offsets[mat] = textures.size();
			mat->place_textures(textures);
		}

		else
			res.texture_offset = it->second;

		return res;
	};

	std::vector<instance> boxes;
	std::vector<IndirectCommand> commands;
	std::vector<mat4x4> nodes_buff;
	//nodes_buff.reserve(rendering_list.size());
	bool first = true;
	auto mesh_func = [&](MeshAssetInstance * l)
	{
		if (l->rendering.empty()) return;

		render_list rendering;
		//rendering.clear();
		int count = l->rendering.size();
		int count_per_thread = std::max(32, count / 8);
		int thread_count = count / count_per_thread;

		if (!use_parrallel) thread_count = 1;

		auto thread_func = [this, &l, &mesh_render_context, current_cpu_culling](int offset, int count)->render_list
		{
			render_list result;

			for (int i = offset; i < offset + count; i++)
			{
				auto& e = l->rendering[i];
				auto& node = l->nodes[e.node_index];

				if (!current_cpu_culling)
					result[l->overrided_material[e.mesh->material]->get_pipeline_id()].push_back({ e.mesh, e.node_index, false });
				else
				{
					auto in = intersect(*mesh_render_context->cam, e.primitive_global.get());

					if (in)
						result[l->overrided_material[e.mesh->material]->get_pipeline_id()].push_back({ e.mesh, e.node_index ,in == INTERSECT_TYPE::IN_NEAR });
				}
			}

			return result;
		};

		if (thread_count == 1)
			rendering = thread_func(0, count);
		else
		{
			std::vector<std::future<render_list>> results;
			int current_offset = 0;

			for (int i = 0; i < thread_count; i++)
			{
				results.emplace_back(thread_pool::get().enqueue(std::bind(thread_func, current_offset, count_per_thread)));
				current_offset += count_per_thread;
			}

			if (count_per_thread * thread_count < count)
				rendering = thread_func(count_per_thread * thread_count, count - count_per_thread * thread_count);

			for (auto& r : results)
			{
				auto& res = r.get();

				for (auto& e : res)
				{
					auto& v = rendering[e.first];
					v.insert(v.end(), e.second.begin(), e.second.end());
				}
			}
		}

		auto nodes = l->node_buffer.get_for(*graphics.get_manager());

		
			for (auto& p : rendering)
			{
				for (auto& m : p.second)
				{
					//	if (!first) continue;
						if (m.inside)
					{
					if (!mesh_render_context->override_material)
					l->use_material(m.mesh->material, mesh_render_context);

					graphics.set(4, l->mesh_asset->vertex_buffer->get_gpu_address());
					graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));
				//	graphics.set_const_buffer(5, nodes.resource->get_gpu_address() + nodes.offset + m.node_index * sizeof(decltype(l->node_buffer)::type));
				graphics.set_srv(14, nodes.resource->get_gpu_address() + nodes.offset);
					graphics.set_constants(8,0, m.node_index);
					mesh_render_context->draw_indexed(m.mesh->index_count, m.mesh->index_offset, m.mesh->vertex_offset);
					continue;
					}

					materials::universal_material* mt = dynamic_cast<materials::universal_material*>((*l->overrided_material[m.mesh->material]).get());

					if (!mt)
						continue;

					auto pass = mt->get_pass(l->type, mesh_render_context);
					auto info = get_draw_info(mt, &pass);
					instance inst;
					inst.pipeline_id = info.pipeline_id;
					inst.b = { m.mesh->primitive->get_min(), m.mesh->primitive->get_max() };
					inst.node = l->global_transform*l->mesh_asset->nodes[m.node_index]->mesh_matrix;
					inst.instance_id = boxes.size();
					boxes.push_back(inst);
					IndirectCommand command;
					command.material.texture_offset = info.texture_offset;
					command.material.node_offset = m.node_index;
					command.drawArguments.InstanceCount = 1;
					command.drawArguments.StartInstanceLocation = 0;
					command.drawArguments.IndexCountPerInstance = m.mesh->index_count;
					command.drawArguments.StartIndexLocation = m.mesh->index_offset;
					command.drawArguments.BaseVertexLocation = m.mesh->vertex_offset;
					command.index_buffer = l->mesh_asset->index_buffer->get_index_buffer_view(true);
					//command.index_buffer.BufferLocation += m.mesh->index_offset * sizeof(UINT32);
					//ommand.index_buffer.SizeInBytes -= m.mesh->index_offset * sizeof(UINT32);
					command.vb_srv = l->mesh_asset->vertex_buffer->get_gpu_address();
					nodes_buff.push_back(inst.node);
					//command.vertex_node_srv = nodes.resource->get_gpu_address() + nodes.offset + m.node_index * sizeof(decltype(l->node_buffer)::type);
					//Log::get() << "nodes offset " << nodes.offset << Log::endl;

					if (mt->get_pixel_buffer())
						command.material_cb_pixel = mt->get_pixel_buffer()->get_gpu_address();

					commands.push_back(command);
					first = false;
				}
			}

	
	};

	if ((MESH_TYPE::STATIC&mesh_render_context->render_mesh) && static_objects.size())
	{
	auto timer =	graphics.start(L"static");
		for (auto & instance : static_objects)
		{
			if (instance->is_inside(*mesh_render_context->cam))
				mesh_func(instance);
			/*auto in = intersect(*mesh_render_context->cam, instance->primitive.get(), instance->global_transform);
			if (in)
				mesh_func(instance);*/
		}
	}

	if ((MESH_TYPE::DYNAMIC&mesh_render_context->render_mesh) && dynamic_objects.size())
	{

		auto timer = graphics.start(L"dynamic");
		for (auto & instance : dynamic_objects)
			if (instance->is_inside(*mesh_render_context->cam))
				mesh_func(instance);
			/*
		{
			auto in = intersect(*mesh_render_context->cam, instance->primitive.get(), instance->global_transform);
			if (in)
				mesh_func(instance);
		}*/
	}

	if (!static_objects.size() && !dynamic_objects.size())
		obj->iterate([&](scene_object * node)
	{
		Render::renderable* render_object = dynamic_cast<Render::renderable*>(node);

		if (render_object)
		{
			auto instance = dynamic_cast<MeshAssetInstance*>(render_object);

			if (!(instance->type&mesh_render_context->render_mesh)) return true;


				if (instance->is_inside(*mesh_render_context->cam))
					mesh_func(instance);


	/*		auto in = intersect(*mesh_render_context->cam, instance->primitive.get(), instance->global_transform);

			if (in)
				mesh_func(instance);*/
		}

		return true;
	});


	if ( boxes.size())
	{

		if (!nodes_resource || nodes_resource->get_count() < nodes_buff.size())
			nodes_resource = std::make_shared<Render::StructuredBuffer<mat4x4>>(nodes_buff.size());
		nodes_resource->set_data(mesh_render_context->list, 0, nodes_buff);



		buffers.new_frame();


		///////////////////////		buffers.process_pipeline(direction);
		unsigned int pipeline_per_cycle = min((unsigned int)pipeline_infos.size(), 16u);
		auto timer = graphics.start(L"gpu culling");
		auto all_commands = graphics.get_manager()->set_data(commands.data(), commands.size());
		auto all_instances = graphics.get_manager()->set_data(boxes.data(), boxes.size());
	
		graphics.set(2, textures);
		graphics.set_srv(14, nodes_resource->get_gpu_address());

		for (int stage = 0; stage <2; stage++)
		{
			auto timer = graphics.start(stage == 0 ? L"render_1" : L"render_2");
			{
				auto timer = graphics.start(L"occluders draw");
				graphics.transition(visible_id_buffer, Render::ResourceState::UNORDERED_ACCESS);
				graphics.clear_uav(visible_id_buffer, visible_id_buffer->get_raw_uav());
			/*	UINT values[4] = { 0 };
				graphics.flush_transitions();
				graphics.get_native_list()->ClearUnorderedAccessViewUint(visible_id_buffer->get_uav().get_base().gpu, visible_id_buffer->get_static_uav().get_base().cpu, visible_id_buffer->get_native().Get(), values, 0, nullptr);*/
				graphics.set_pipeline(state);
				graphics.set_index_buffer(index_buffer->get_index_buffer_view(true));
				graphics.set(4, vertex_buffer->get_gpu_address());
				graphics.set_dynamic(3, 0, visible_id_buffer->get_static_uav());
				mesh_render_context->g_buffer->set_downsapled(mesh_render_context);

				if (stage == 0)
				{
					graphics.set_srv(7, all_instances.get_gpu_address());
					graphics.draw_indexed(36, 0, 0, boxes.size());
				}

				else
				{
					graphics.set_srv(7, invisible_commands->get_gpu_address());
					graphics.execute_indirect(
						second_draw_commands,
						1, \
						second_draw_arguments.get());
				}
			}


			{
				graphics.transition(visible_id_buffer, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
			}


			// get_invisible instances
			if (stage == 0)
			{
				auto timer = graphics.start(L"invisible gather");
				compute.transition(invisible_commands, Render::ResourceState::COPY_DEST);
				compute.copy_buffer(invisible_commands.get(), invisible_commands->get_counter_offset(), buffers.clear_buffer.get(), 0, 4);
				compute.transition(invisible_commands, Render::ResourceState::UNORDERED_ACCESS);
				compute.transition(second_draw_dispatch, Render::ResourceState::UNORDERED_ACCESS);
				compute.set_pipeline(gather_invisible);
				compute.set_dynamic(0, 0, visible_id_buffer->get_srv());
				compute.set_srv(1, all_instances.get_gpu_address());
				compute.set_dynamic(2, 0, invisible_commands->get_uav());
				compute.set_constants(3, UINT(boxes.size()), UINT(0), UINT(max(1u, UINT(boxes.size() / 64))));
				compute.set_dynamic(4, 0, second_draw_dispatch->get_uav());
				compute.dispach(ivec3(1, 1, 1), ivec3(1, 1, 1));
				graphics.copy_buffer(second_draw_arguments.get(), sizeof(UINT) * 2, invisible_commands.get(), invisible_commands->get_counter_offset(), 4);

				graphics.transition(invisible_commands, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
				graphics.transition(second_draw_arguments, Render::ResourceState::INDIRECT_ARGUMENT);
				compute.transition(second_draw_dispatch, Render::ResourceState::INDIRECT_ARGUMENT);
			}

			for (unsigned int start_index = 0; start_index < pipeline_infos.size(); start_index += pipeline_per_cycle)
			{
				unsigned int current_count = min(pipeline_per_cycle, UINT(pipeline_infos.size() - start_index));
				{
					buffers.start_write(compute, current_count);
					auto timer = graphics.start(L"gather all");
					compute.set_pipeline(compute_state[stage]);
					compute.flush_transitions();
					compute.set_dynamic(0, 0, visible_id_buffer->get_srv());
					compute.set_srv(1, all_commands.get_gpu_address());
					compute.set_dynamic(2, 0, buffers.get_handles());
					compute.set_constants(3, UINT(boxes.size()), start_index + 1, 32);
					compute.set_dynamic(5, 0, invisible_commands->get_srv());

					{
						if (stage == 0)
							compute.dispach(ivec3(boxes.size(), 1, 1), ivec3(64 * 32, 1, 1));
						else
							compute.execute_indirect(
								command_dispatch,
								1,
								second_draw_dispatch.get());
					}
				}
				{
					buffers.start_indirect(compute, current_count);
					auto timer = graphics.start(L"ExecuteIndirect");

					for (unsigned int i = 0; i < current_count; i++)
					{
						auto& p = pipeline_infos[indirection[i + start_index]];
						auto timer = graphics.start(std::to_wstring(i).c_str());
						auto& command_buffer = buffers.get_buffer(i);
						graphics.set_pipeline(p.pipeline);
						graphics.set_topology(p.topology);

						{
							mesh_render_context->g_buffer->set_original(mesh_render_context);
						}

/*
						graphics.execute_indirect(
							m_commandSignature,
							p.draw_count,
							all_commands.resource.get(),
							all_commands.offset);*/

				/*		graphics.execute_indirect(
							m_commandSignature,
							p.draw_count,
							command_buffer.get(),
							0,
							command_buffer.get(),
							command_buffer->get_counter_offset());*/
					}
				}
			}

			//	if (stage == 0)
			{
				//	auto timer = graphics.start(L"downsample_depth");
				mesh_render_context->g_buffer->downsample_depth(mesh_render_context->list);
			}
		}

		
	}


}

 void gpu_mesh_renderer::on_add(scene_object* object)
{
	auto render_object = dynamic_cast<MeshAssetInstance*>(object);

	if (!render_object) return;

	if (render_object->type == MESH_TYPE::STATIC)
		static_objects.insert(render_object);

	if (render_object->type == MESH_TYPE::DYNAMIC)
		dynamic_objects.insert(render_object);

}


 void gpu_mesh_renderer::on_remove(scene_object* object)
{
	auto render_object = dynamic_cast<MeshAssetInstance*>(object);

	if (!render_object) return;

	if (render_object->type == MESH_TYPE::STATIC)
		static_objects.erase(render_object);

	if (render_object->type == MESH_TYPE::DYNAMIC)
		dynamic_objects.erase(render_object);

}
void gpu_mesh_renderer::iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)> f)
{


	if (mesh_type&MESH_TYPE::STATIC)
		for (auto & instance : static_objects)
			f(instance->get_ptr<scene_object>());

	if (mesh_type& MESH_TYPE::DYNAMIC)
		for (auto & instance : dynamic_objects)
			f(instance->get_ptr<scene_object>());
}
gpu_mesh_renderer::gpu_mesh_renderer(Scene::ptr scene)
{
	this->scene = scene;
	if (scene) {
		scene->on_element_add.register_handler(this, [this](scene_object* object) {

			on_add(object);

		});

		scene->on_element_remove.register_handler(this, [this](scene_object*object) {
			on_remove(object);
		});
	}
	shader = Render::vertex_shader::get_resource({ "shaders/triangle.hlsl", "VS", 0,{} });
	voxel_geometry_shader = Render::geometry_shader::get_resource({ "shaders/voxelization.hlsl", "GS", 0,{} });
	Render::RootSignatureDesc root_desc;
	root_desc[0] = Render::DescriptorConstBuffer(1, Render::ShaderVisibility::PIXEL); // material constants
	root_desc[1] = Render::DescriptorConstBuffer(1, Render::ShaderVisibility::DOMAIN); // material constants
	root_desc[2] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 32); // material textures
	root_desc[3] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::PIXEL, 0, 8); // material virtual texture
	root_desc[4] = Render::DescriptorSRV(0, Render::ShaderVisibility::VERTEX, 1); //vertex buffer
	root_desc[5] = Render::DescriptorConstBuffer(1, Render::ShaderVisibility::VERTEX); // vertex node data
	root_desc[6] = Render::DescriptorConstBuffer(0, Render::ShaderVisibility::ALL); // camera
	root_desc[7] = Render::DescriptorSRV(2, Render::ShaderVisibility::VERTEX, 1); // instance data
	root_desc[8] = Render::DescriptorConstants(2, 2, Render::ShaderVisibility::ALL); // material offsets
	root_desc[9] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::PIXEL, 0, 3, 1); //for voxels
	root_desc[10] = Render::DescriptorConstBuffer(3, Render::ShaderVisibility::GEOMETRY); // voxel scale info
	root_desc[11] = Render::DescriptorConstBuffer(3, Render::ShaderVisibility::PIXEL); // voxel scale info
	root_desc[12] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 1, 1); //best fit normals
	root_desc[13] = Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 1, 2); //best fit normals
	root_desc[14] = Render::DescriptorSRV(1, Render::ShaderVisibility::VERTEX, 1); // instance data
																													 // root_desc[8] = Render::DescriptorSRV(1, Render::ShaderVisibility::VERTEX, 1); // node buffer
																													 //  root_desc[8] = Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::PIXEL, 0, 1, 1); // Occlusion visible buffer
	root_desc.set_sampler(0, 0, Render::ShaderVisibility::ALL, Render::Samplers::SamplerLinearWrapDesc);
	root_desc.set_sampler(1, 0, Render::ShaderVisibility::ALL, Render::Samplers::SamplerPointClampDesc);
	root_desc.set_sampler(2, 0, Render::ShaderVisibility::ALL, Render::Samplers::SamplerAnisoWrapDesc);
	my_signature.reset(new Render::RootSignature(root_desc));
	my_signature->set_unfixed(2);
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
	Render::PipelineStateDesc desc;
	desc.root_signature = my_signature;
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
	state = Render::PipelineStateCache::get().get_cache(desc);
	visible_id_buffer = std::make_shared<Render::StructuredBuffer<UINT>>(2048, false, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	//	cpu_commands = std::make_shared<Render::FrameStorage<IndirectCommand>>();
	//	cpu_instances = std::make_shared<Render::FrameStorage<instance>>();
	invisible_commands = std::make_shared<Render::StructuredBuffer<instance>>(2044, true, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	// Create the command signature used for indirect drawing.
	{
		/*
		D3D12_GPU_VIRTUAL_ADDRESS index_buffer;
		CD3DX12_GPU_DESCRIPTOR_HANDLE vb_srv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE textures_srv;

		D3D12_DRAW_INDEXED_ARGUMENTS drawArguments;
		*/
		// Each command consists of a CBV update and a DrawInstanced call.
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[5] = {};
		int i = 0;
		argumentDescs[i].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
		argumentDescs[i].ConstantBufferView.RootParameterIndex = 0;
		i++;
		argumentDescs[i].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		argumentDescs[i].ShaderResourceView.RootParameterIndex = 4;
	//	i++;
		//argumentDescs[i].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	//	argumentDescs[i].ConstantBufferView.RootParameterIndex = 5;
		i++;
		argumentDescs[i].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		argumentDescs[i].Constant.RootParameterIndex = 8;
		argumentDescs[i].Constant.DestOffsetIn32BitValues = 0;
		argumentDescs[i].Constant.Num32BitValuesToSet = 2;
		i++;
		argumentDescs[i].Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
		i++;
		argumentDescs[i].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(IndirectCommand);
		TEST(Render::Device::get().get_native_device()->CreateCommandSignature(&commandSignatureDesc, my_signature->get_native().Get(), IID_PPV_ARGS(&m_commandSignature)));
	}
	{
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
		argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		argumentDescs[0].Constant.RootParameterIndex = 8;
		argumentDescs[0].Constant.DestOffsetIn32BitValues = 0;
		argumentDescs[0].Constant.Num32BitValuesToSet = 1;
		argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(second_draw);
		TEST(Render::Device::get().get_native_device()->CreateCommandSignature(&commandSignatureDesc, my_signature->get_native().Get(), IID_PPV_ARGS(&second_draw_commands)));
	}
	second_draw_arguments = std::make_shared<Render::GPUBuffer>(sizeof(second_draw));
	second_draw args;
	args.constant = 0;
	args.args.IndexCountPerInstance = 36;
	args.args.BaseVertexLocation = 0;
	args.args.InstanceCount = 0;
	args.args.StartIndexLocation = 0;
	args.args.StartInstanceLocation = 0;
	second_draw_arguments->set_data(args);
	second_draw_dispatch = std::make_shared<Render::StructuredBuffer<gather_second>>(1, false, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	Render::ComputePipelineStateDesc compute_desc;
	compute_desc.root_signature = std::make_shared<Render::RootSignature>(
		Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 1),
		Render::DescriptorSRV(1, Render::ShaderVisibility::ALL),
		Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 16),
		Render::DescriptorConstants(0, 3),
		Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 0, 1, 1),
		Render::DescriptorTable(Render::DescriptorRange::SRV, Render::ShaderVisibility::ALL, 0, 1, 1),
		Render::DescriptorTable(Render::DescriptorRange::UAV, Render::ShaderVisibility::ALL, 1, 1, 1)
		);

	for (int i = 0; i < compute_state.size();i++)
	{
		std::vector<D3D::shader_macro> macros;

		if (i&static_cast<int>(GATHER_TYPE::ALL_GOOD))
			macros.emplace_back("ALL_GOOD", "1");

		if (i&&static_cast<int>(GATHER_TYPE::OVERRIDED_MATERIAL))
			macros.emplace_back("OVERRIDED_MATERIAL", "1");

		compute_desc.shader = Render::compute_shader::get_resource({ "shaders/occluder_cs.hlsl", "CS", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES | D3DCOMPILE_SKIP_OPTIMIZATION ,macros });
		compute_state[i] = std::make_shared<Render::ComputePipelineState>(compute_desc);
	}



	compute_desc.shader = Render::compute_shader::get_resource({ "shaders/occluder_cs_invisible.hlsl", "CS", 0,{} });
	gather_invisible = std::make_shared<Render::ComputePipelineState>(compute_desc);
	{
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
		argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		argumentDescs[0].Constant.RootParameterIndex = 3;
		argumentDescs[0].Constant.DestOffsetIn32BitValues = 0;
		argumentDescs[0].Constant.Num32BitValuesToSet = 1;
		argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(gather_second);
		TEST(Render::Device::get().get_native_device()->CreateCommandSignature(&commandSignatureDesc, compute_desc.root_signature->get_native().Get(), IID_PPV_ARGS(&command_dispatch)));
	}

	best_fit_normals = EngineAssets::best_fit_normals.get_asset();
}

