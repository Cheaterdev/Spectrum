#include "pch.h"

void mesh_renderer::render(MeshRenderContext::ptr mesh_render_context, scene_object::ptr obj)
{
	// return;
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& list = *mesh_render_context->list;

	//  std::list<MeshAssetInstance*> meshes;
	instances_count = 0;
	bool current_cpu_culling = use_cpu_culling && mesh_render_context->render_type == RENDER_TYPE::PIXEL;
	mesh_render_context->transformer = transformer;
	Render::PipelineStateDesc default_pipeline = mesh_render_context->pipeline;
	default_pipeline.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	default_pipeline.root_signature = my_signature;
	default_pipeline.rtv.enable_depth = true;
	default_pipeline.rasterizer.fill_mode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	default_pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	default_pipeline.blend.independ_blend = false;
	default_pipeline.blend.render_target[0].enabled = false;
	default_pipeline.vertex = shader;
	default_pipeline.rasterizer.conservative =  GetAsyncKeyState('B') && mesh_render_context->render_type == RENDER_TYPE::VOXEL;

	if (mesh_render_context->render_type == RENDER_TYPE::VOXEL)
	{
		default_pipeline.geometry = voxel_geometry_shader;
		default_pipeline.rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
		default_pipeline.rtv.enable_depth = false;
		graphics.set_signature(my_signature);



		voxel_info.voxel_min = { mesh_render_context->voxel_min,5 };
		voxel_info.voxel_size = { mesh_render_context->voxel_size,6 };
		voxel_info.voxel_map_size = { mesh_render_context->voxel_target_size ,0};// ivec3(512, 512, 512);
		voxel_info.voxel_tiles_count = ivec4(mesh_render_context->voxel_tiles_count, 0);// ivec3(512, 512, 512);
		voxel_info.voxels_per_tile = ivec4(mesh_render_context->voxels_per_tile, 0);// ivec3(512, 512, 512);

	//	Log::get() << "params: " << mesh_render_context->voxel_min << " " << mesh_render_context->voxel_size << Log::endl;
	//	graphics.set_const_buffer(10, voxel_info);
	//	graphics.set_const_buffer(11, voxel_info);
	}

	mesh_render_context->pipeline = default_pipeline;
	mesh_render_context->begin();
	mesh_render_context->set_frame_data(mesh_render_context->cam->get_const_buffer());

	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GPUMeshSignature<Signature> signature(&graphics);

	if (best_fit_normals)
		signature.best_fit[0]= best_fit_normals->get_texture()->texture_2d()->get_static_srv();

	using render_list = std::map<int, std::vector<MeshAssetInstance::render_info>>;


	bool first = true;
	auto mesh_func = [&](MeshAssetInstance * l)
	{
		if (l->rendering.empty()) return;
		render_list rendering;
		//rendering.clear();
		int count = l->rendering.size();
		int count_per_thread = std::max(32, count / 8);
		int thread_count = 1;// count / count_per_thread;

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
						result[l->overrided_material[e.mesh->material]->get_pipeline_id()].push_back({ e.mesh, e.node_index ,   in == INTERSECT_TYPE::IN_NEAR });
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

		auto nodes = l->node_buffer.get_for(*list.get_manager());
		signature.vertex_unknown = nodes.resource->get_gpu_address() + nodes.offset;
		if (mesh_render_context->render_type == RENDER_TYPE::VOXEL)
		{
			signature.vertex_consts_geometry.set_raw(voxel_info);
			signature.vertex_consts_vertex.set_raw(voxel_info);
		}
			for (auto& p : rendering)
			{
				for (auto& m : p.second)
				{
					if (!mesh_render_context->override_material)
						l->use_material(m.mesh->material, mesh_render_context);

				//	graphics.set_srv(4, l->mesh_asset->vertex_buffer->get_gpu_address());
					graphics.set_index_buffer(l->mesh_asset->index_buffer->get_index_buffer_view(true));
					//graphics.set_const_buffer(5, nodes.resource->get_gpu_address() + nodes.offset + m.node_index * sizeof(decltype(l->node_buffer)::type));
				//	graphics.set_constants(8,0, m.node_index);

					signature.vertex_buffer = l->mesh_asset->vertex_buffer->get_gpu_address();
					signature.mat_texture_offsets.set(0, m.node_index);

					mesh_render_context->draw_indexed(m.mesh->index_count, m.mesh->index_offset, m.mesh->vertex_offset);
				}
			}
	};

	if ((MESH_TYPE::STATIC&mesh_render_context->render_mesh) && static_objects.size())
	{

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
		for (auto & instance : dynamic_objects)
		{
			if (instance->is_inside(*mesh_render_context->cam))
				mesh_func(instance);

	/*		auto in = intersect(*mesh_render_context->cam, instance->primitive.get(), instance->global_transform);
			if (in)
				mesh_func(instance);*/
		}
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


		/*	auto in = intersect(*mesh_render_context->cam, instance->primitive.get(), instance->global_transform);

			if (in)
				mesh_func(instance);*/
		}

		return true;
	});
}
void mesh_renderer::iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)> f)
{


	if (mesh_type&MESH_TYPE::STATIC)
		for (auto & instance : static_objects)
			f(instance->get_ptr<scene_object>());

	if (mesh_type& MESH_TYPE::DYNAMIC)
		for (auto & instance : dynamic_objects)
			f(instance->get_ptr<scene_object>());
}
mesh_renderer::mesh_renderer(Scene::ptr scene)
{
	this->scene = scene;
	if (scene) {
		scene->on_element_add.register_handler(this, [this](scene_object* object) {

			auto render_object = dynamic_cast<MeshAssetInstance*>(object);

			if (!render_object) return;

			if (render_object->type == MESH_TYPE::STATIC)
				static_objects.insert(render_object);

			if (render_object->type == MESH_TYPE::DYNAMIC)
				dynamic_objects.insert(render_object);


		});

		scene->on_element_remove.register_handler(this, [this](scene_object*object) {
			auto render_object = dynamic_cast<MeshAssetInstance*>(object);

			if (!render_object) return;

			if (render_object->type == MESH_TYPE::STATIC)
				static_objects.erase(render_object);

			if (render_object->type == MESH_TYPE::DYNAMIC)
				dynamic_objects.erase(render_object);

		});
	}
	shader = Render::vertex_shader::get_resource({ "shaders/triangle.hlsl", "VS", 0, {} });
	voxel_geometry_shader = Render::geometry_shader::get_resource({ "shaders/voxelization.hlsl", "GS", 0, {} });
	/*Render::RootSignatureDesc root_desc;
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
	root_desc.set_sampler(2, 0, Render::ShaderVisibility::ALL, Render::Samplers::SamplerAnisoWrapDesc);*/
	my_signature = GPUMeshSignature<SignatureCreator>().create_root();// .reset(new Render::RootSignature(root_desc));
	my_signature->set_unfixed(2);

	best_fit_normals = EngineAssets::best_fit_normals.get_asset();
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
			list.transition(command_buffers[i]->help_buffer,Render::ResourceState::UNORDERED_ACCESS);

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




