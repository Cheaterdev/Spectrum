#include "pch.h"




BOOST_CLASS_EXPORT_IMPLEMENT(materials::universal_material);


materials::render_pass & materials::universal_material::get_pass(RENDER_TYPE render_type, MESH_TYPE type)
{

	render_pass * pass = nullptr;

	if (render_type == RENDER_TYPE::PIXEL)
		pass = &passes[PASS_TYPE::DEFERRED];
	else
		if (type == MESH_TYPE::STATIC)
			pass = &passes[PASS_TYPE::VOXEL_STATIC];
		else
			pass = &passes[PASS_TYPE::VOXEL_DYNAMIC];
	return *pass;
}



 materials::render_pass & materials::universal_material::get_pass(MESH_TYPE type, MeshRenderContext::ptr & context)
{
	
	update(context);
	std::lock_guard<std::mutex> g(m);

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		TiledTexture::ptr tiled = std::dynamic_pointer_cast<TiledTexture>(*textures[i]->asset);

		if (tiled)
			tiled->update(context->list);
	}
	render_pass * pass = nullptr;




	if (context->render_type == RENDER_TYPE::PIXEL)
		pass = &passes[PASS_TYPE::DEFERRED];
	else
		if (type == MESH_TYPE::STATIC)
			pass = &passes[PASS_TYPE::VOXEL_STATIC];
		else
			pass = &passes[PASS_TYPE::VOXEL_DYNAMIC];
	return *pass;
}

void materials::universal_material::set_pipeline_states(MESH_TYPE type, MeshRenderContext::ptr& context, Render::PipelineStateDesc& desc)
{

	render_pass * pass = nullptr;

	if (context->render_type == RENDER_TYPE::PIXEL)
		pass = &passes[PASS_TYPE::DEFERRED];
	else
		if (type == MESH_TYPE::STATIC)
			pass = &passes[PASS_TYPE::VOXEL_STATIC];
		else
			pass = &passes[PASS_TYPE::VOXEL_DYNAMIC];


	desc.pixel = pass->ps_shader;
	desc.domain = pass->ds_shader;
	desc.hull = pass->hs_shader;

	desc.topology = pass->ds_shader ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  /*  desc.pixel = shader;
	desc.domain = ds_shader;
	desc.hull = hs_shader;
	desc.topology = ds_shader ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;*/
}

void  materials::universal_material::set(RENDER_TYPE render_type, MESH_TYPE type, Render::PipelineStateDesc &pipeline)
{
	auto pass = &get_pass(render_type,type);


	std::lock_guard<std::mutex> g(m);



	pipeline.pixel = pass->ps_shader;
	pipeline.domain = pass->ds_shader;
	pipeline.hull = pass->hs_shader;

	if (pass->ds_shader)
	{
		pipeline.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
	else
	{
		pipeline.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}

}

void materials::universal_material::set(MESH_TYPE type, MeshRenderContext::ptr& context)
{
	auto pass = &get_pass(type, context);


	std::lock_guard<std::mutex> g(m);
 


	context->pipeline.pixel = pass->ps_shader;
	context->pipeline.domain = pass->ds_shader;
	context->pipeline.hull = pass->hs_shader;

//	if (tess_buffer)
	//	context->set_material_tess_buffer(tess_buffer);

	if (pixel_buffer)
		context->set_material_const_buffer(pixel_buffer);

	if (pass->ds_shader)
	{
		context->pipeline.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		context->list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}

	else
	{
		context->pipeline.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		context->list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	if (textures.size())
	{
		context->set_material_textures(texture_handles);

		if (uav_handles.valid())
			context->set_material_unordered(uav_handles);
	}
}

void materials::universal_material::update(MeshRenderContext::ptr& c)
{
	std::lock_guard<std::mutex> g(m);

	if (need_regenerate_material)
	{
		generate_material();
		need_regenerate_material = false;
	}

	if (!need_update_uniforms) return;

	auto generate = [this, c](Render::GPUBuffer::ptr & buffer, std::vector<Uniform::ptr>& un)
	{
		std::vector<std::byte> data;

		for (auto u : un)
		{
			if (u->type == ShaderParams::FLOAT1)
			{
				auto& v = u->value.f4_value;
				data.insert(data.end(), reinterpret_cast<std::byte*>(&v), reinterpret_cast<std::byte*>(&v) + sizeof(v));
			}

			if (u->type == ShaderParams::FLOAT2)
			{
				auto& v = u->value.f4_value;
				data.insert(data.end(), reinterpret_cast<std::byte*>(&v), reinterpret_cast<std::byte*>(&v) + sizeof(v));
			}

			if (u->type == ShaderParams::FLOAT3)
			{
				auto& v = u->value.f4_value;
				data.insert(data.end(), reinterpret_cast<std::byte*>(&v), reinterpret_cast<std::byte*>(&v) + sizeof(v));
			}

			if (u->type == ShaderParams::FLOAT4)
			{
				auto& v = u->value.f4_value;
				data.insert(data.end(), reinterpret_cast<std::byte*>(&v), reinterpret_cast<std::byte*>(&v) + sizeof(v));
			}
		}

		if (data.empty())
			return;

		data.resize(Math::AlignUp(data.size(), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), std::byte{ 0 });
		buffer->set_data(c->list, data);
	};
	generate(pixel_buffer, ps_uniforms);
//	generate(tess_buffer, tess_uniforms);
	need_update_uniforms = false;

	mark_contents_changed();
}

 void materials::universal_material::place_textures(std::vector<Render::Handle>& handles)
{
	//  for(auto &t:textures)
	handles.insert(handles.end(), texture_handles.begin(), texture_handles.end());
}
 size_t  materials::universal_material::get_id()
 {

	return  passes[PASS_TYPE::DEFERRED].get_pipeline_id(); ///TODO: change for graph id
 }
 materials::render_pass & materials::universal_material::get_normal_pass()
{
//	std::lock_guard<std::mutex> g(m);
	return passes[PASS_TYPE::DEFERRED];
}

void materials::universal_material::compile()
{
	start_changing_contents();
	handlers.clear();
	texture_handles.resize(textures.size());
	texture_table = Render::DescriptorHeapManager::get().get_csu_static()->create_table(textures.size());


	for (unsigned int i = 0; i < textures.size(); i++)
	{
		handlers.emplace_back();
		auto &t = textures[i];
		TextureAsset::ptr tex =t->asset->get_ptr<TextureAsset>();
/*
		if (tex && tex->get_texture()->texture_2d())
		{
			texture_handles[i] = tex->get_texture()->texture_2d()->get_static_srv();//place_srv(texture_handles[texture_count++]);

			texture_table[i].place(texture_handles[i]);
		}*/

		if (tex && tex->get_texture()->texture_2d())

			t->to_linear.register_change(&handlers.back(), [this, i, tex](bool to_linear)
		{
			//need_update_uniforms = true;
			auto func = tex->get_texture()->texture_2d()->srv(to_linear ? PixelSpace::MAKE_LINERAR : PixelSpace::MAKE_SRGB);


			//		func(texture_handles[i]);
			func(texture_table[i]);
			texture_handles[i] = texture_table[i];
			//	texture_handles[i] = tex->get_texture()->texture_2d()->srv;//place_srv(texture_handles[texture_count++]);
				//texture_table[i].place(texture_handles[i]);

		});

		else
		{

			auto asset = EngineAssets::missing_texture.get_asset();
			auto func = asset->get_texture()->texture_2d()->srv();
			func(texture_table[i]);
			texture_handles[i] = texture_table[i];
		}
		//	textures.push_back(t);

		t->asset = register_asset(*t->asset);

		mark_contents_changed();
	}


	int texture_count = 0;
	int uav_count = 0;

	if (textures.size())
	{
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			TextureAsset::ptr tex = std::dynamic_pointer_cast<TextureAsset>(*textures[i]->asset);

			if (tex)
				texture_count++;

			TiledTexture::ptr tiled = std::dynamic_pointer_cast<TiledTexture>(*textures[i]->asset);

			if (tiled)
			{
				texture_count++;
				texture_count++;
				uav_count++;
			}
		}
	}

	/*  if (texture_count)
		  texture_handles = Render::DescriptorHeapManager::get().get_csu()->create_table(texture_count);
	  else
		  texture_handles = Render::HandleTable();
		*/
	if (uav_count)
		uav_handles = Render::DescriptorHeapManager::get().get_csu_static()->create_table(uav_count);
	else
		uav_handles = Render::HandleTable();

	texture_count = 0;
	uav_count = 0;

	if (textures.size())
	{
	

		for (unsigned int i = 0; i < textures.size(); i++)
		{
	   /*     TextureAsset::ptr tex = std::dynamic_pointer_cast<TextureAsset>(*textures[i]->asset);

			if (tex && tex->get_texture()->texture_2d())
			{
				texture_handles[i] = tex->get_texture()->texture_2d()->get_static_srv();//place_srv(texture_handles[texture_count++]);

				texture_table[i].place(texture_handles[i]);
			}*/
			TiledTexture::ptr tiled = std::dynamic_pointer_cast<TiledTexture>(*textures[i]->asset);

			if (tiled)
			{
				//  tiled->tiled_tex->texture_2d()->place_srv(texture_handles[texture_count++]);
				//   tiled->residency_texture->texture_2d()->place_srv(texture_handles[texture_count++]);
				//  tiled->visibility_texture->place_uav(uav_handles[uav_count++]);
			}
		}
	}

	auto generate = [this](std::vector<Uniform::ptr>& un)->Render::GPUBuffer::ptr
	{
		std::vector<std::byte> data;

		for (auto u : un)
		{
			handlers.emplace_back();
			u->on_change.register_handler(&handlers.back(), [this](Uniform * u)
			{
				need_update_uniforms = true;
			});

			if (u->type == ShaderParams::FLOAT1)
			{
				auto& v = u->value.f4_value;
				data.insert(data.end(), reinterpret_cast<std::byte*>(&v), reinterpret_cast<std::byte*>(&v) + sizeof(v));
			}

			if (u->type == ShaderParams::FLOAT2)
			{
				auto& v = u->value.f4_value;
				data.insert(data.end(), reinterpret_cast<std::byte*>(&v), reinterpret_cast<std::byte*>(&v) + sizeof(v));
			}

			if (u->type == ShaderParams::FLOAT3)
			{
				auto& v = u->value.f4_value;
				data.insert(data.end(), reinterpret_cast<std::byte*>(&v), reinterpret_cast<std::byte*>(&v) + sizeof(v));
			}

			if (u->type == ShaderParams::FLOAT4)
			{
				auto& v = u->value.f4_value;
				data.insert(data.end(), reinterpret_cast<std::byte*>(&v), reinterpret_cast<std::byte*>(&v) + sizeof(v));
			}
		}

		if (data.empty())
			return nullptr;



		data.resize(Math::AlignUp(data.size(), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), std::byte{ 0 });

		Render::GPUBuffer::ptr buff(new Render::GPUBuffer(data.size()));
		buff->set_raw_data(data);
		return buff;
	};





  
	pixel_buffer = generate(ps_uniforms);
//	tess_buffer = generate(tess_uniforms);


	end_changing_contents();

//	textures.clear();

//	textures.push_back(t);// TextureSRVParamsRef{ register_asset(*t->asset),t->to_linear.get() });

}

void materials::universal_material::on_graph_changed()
{
	need_regenerate_material = true;
}


void materials::universal_material::generate_texture_handles()
{
/*	{
		textures.clear();

		for (auto& t : context->get_textures())
			textures.push_back(TextureSRVParamsRef{ register_asset(*t->asset),t->to_linear.get() });

		compile();
	}*/
}

void materials::universal_material::generate_material()
{
//   std::lock_guard<std::mutex> g(m);
	if (!context)
		context.reset(new MaterialContext);

	context->start(include_file->get_data(), graph.get().get());
	auto ps_str = include_file->get_data() + context->get_result();
	auto tess_orig_shader = context->get_tess_result();
	auto tess_str = include_file->get_data() + tess_orig_shader;
	auto voxel_str = include_file->get_data() + context->get_voxel_result();
	//    Log::get() << "SHADER: " << ps_str << "\n" << Hasher::hash(ps_str) << Log::endl;
	Render::pixel_shader::ptr res_p_shader = passes[PASS_TYPE::DEFERRED].ps_shader;
	Render::hull_shader::ptr res_h_shader = passes[PASS_TYPE::DEFERRED].hs_shader;
	Render::domain_shader::ptr res_d_shader = passes[PASS_TYPE::DEFERRED].ds_shader;
	Render::pixel_shader::ptr res_voxel_shader = passes[PASS_TYPE::VOXEL_STATIC].ps_shader;
	Render::pixel_shader::ptr res_voxel_shader_dynamic = passes[PASS_TYPE::VOXEL_DYNAMIC].ps_shader;

 //   bool textures_changed = false;
	bool shaders_changed = false;
	/*
	if (shader)
		Log::get() << "old " <<  shader->get_hash().hexdigest() << " new " << MD5(ps_str).hexdigest() << Log::endl;
	else
		Log::get() << "old " << "__" << " new " << MD5(ps_str).hexdigest() << Log::endl;
	*/
	if (!res_p_shader || res_p_shader->get_hash() != MD5(ps_str))
	{
		res_p_shader = Render::pixel_shader::create_from_memory(ps_str, "PS", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES);
		shaders_changed = true;

		if (!res_p_shader) return;
	}

	if (!res_voxel_shader || res_voxel_shader->get_hash() != MD5(voxel_str))
	{
		res_voxel_shader = Render::pixel_shader::create_from_memory(voxel_str, "PS_VOXEL", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES);
		res_voxel_shader_dynamic = Render::pixel_shader::create_from_memory(voxel_str, "PS_VOXEL", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, { D3D::shader_macro("VOXEL_DYNAMIC","1") });
		shaders_changed = true;

		if (!res_voxel_shader||!res_voxel_shader_dynamic) return;
	}

	
	if (tess_orig_shader.size())
	{
		//      Log::get() << "TESSSHADER: " << context->get_tess_result() << Log::endl;
		if (!res_h_shader || res_h_shader->get_hash() != MD5(tess_str))
		{
			res_h_shader = Render::hull_shader::create_from_memory(tess_str, "HS", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES);

			if (!res_h_shader) return;
		}

		if (!res_d_shader || res_d_shader->get_hash() != MD5(tess_str))
		{
			res_d_shader = Render::domain_shader::create_from_memory(tess_str, "DS", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES);

			if (!res_d_shader) return;
		}

		shaders_changed = true;
	}

	else
	{
		if (res_d_shader || res_h_shader)
		{
			res_d_shader = nullptr;
			res_h_shader = nullptr;
			shaders_changed = true;
		}
	}
	
	int i = 0;
/*
	if (context->get_textures().size() == textures.size())
	{
		for (auto& t : context->get_textures())
		{
			if (textures[i].asset != t->asset || textures[i].to_linear != t->to_linear.get())
				textures_changed = true;

			i++;
		}
	}

	else
		textures_changed = true;*/
	
	if (shaders_changed)
	{

		passes[PASS_TYPE::DEFERRED].ps_shader= res_p_shader;
		passes[PASS_TYPE::DEFERRED].ds_shader = res_d_shader;
		passes[PASS_TYPE::DEFERRED].hs_shader = res_h_shader;



		passes[PASS_TYPE::VOXEL_DYNAMIC].ps_shader = res_voxel_shader_dynamic;
		passes[PASS_TYPE::VOXEL_DYNAMIC].ds_shader = res_d_shader;
		passes[PASS_TYPE::VOXEL_DYNAMIC].hs_shader = res_h_shader;


		passes[PASS_TYPE::VOXEL_STATIC].ps_shader = res_voxel_shader;
		passes[PASS_TYPE::VOXEL_STATIC].ds_shader = res_d_shader;
		passes[PASS_TYPE::VOXEL_STATIC].hs_shader = res_h_shader;

	  
		for (auto&p : passes)
			p.init_pipeline_id();
  }
	
	ps_uniforms = context->uniforms_ps;
//	tess_uniforms = context->uniforms_tess;


	//generate_texture_handles();

	//   if (textures_changed)
	{
		for (auto & t : textures)
		{
			t->asset.destroy();
		}
		textures = context->get_textures();
		/*for(auto & t:textures)
		{
			t->asset = register_asset(*t->asset);
		}*/
		compile();
	}

  //  if ((textures_changed || shaders_changed))
		mark_contents_changed();

	need_regenerate_material = false;
	on_change();
}

MaterialGraph::ptr materials::universal_material::get_graph()
{
	return graph.get();
}

 Render::GPUBuffer::ptr materials::universal_material::get_pixel_buffer()
{
	return pixel_buffer;
}

materials::universal_material::universal_material(MaterialGraph::ptr graph) : include_file(this)
{

	passes.resize(PASS_TYPE::COUNTER);

	include_file = register_asset(EngineAssets::material_header.get_asset());

	this->graph = BinaryData<MaterialGraph>(graph);
	graph->add_listener(this, false);
	need_regenerate_material = true;
	need_update_uniforms = true;
	generate_material();
}

 void materials::universal_material::test()
{
	graph.test();
}

materials::universal_material::universal_material() : include_file(this)
{
	graph.on_create = [this](MaterialGraph::ptr g)
	{
		g->add_listener(this, false);
		on_graph_changed();
	};
}

void materials::universal_material::on_asset_change(std::shared_ptr<Asset> asset)
{
	if (asset == *include_file)
		on_graph_changed();

	if (asset->get_type() == Asset_Type::TEXTURE)
		on_graph_changed();
}

void materials::universal_material::on_unlink(::FlowGraph::parameter*, ::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_link(::FlowGraph::parameter*, ::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_remove_output(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_add_output(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_remove_input(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_add_input(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_remove(::FlowGraph::window*)
{
	on_graph_changed();
}

void materials::universal_material::on_register(::FlowGraph::window*)
{
	on_graph_changed();
}

template void materials::universal_material::serialize(serialization_oarchive& arch, const unsigned int version);
template void materials::universal_material::serialize(serialization_iarchive& arch, const unsigned int version);

 D3D_PRIMITIVE_TOPOLOGY materials::render_pass::get_topology()
{
	return  ds_shader ? D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

 size_t materials::render_pass::get_pipeline_id()
{
	return pipeline_id;
}


template<class Archive>
void materials::universal_material::serialize(Archive& ar, const unsigned int file_version)
{
	ar& NVP(boost::serialization::base_object<MaterialAsset>(*this));
	ar& NVP(textures);
	ar& NVP(passes);
	ar& NVP(graph);
	ar& NVP(include_file);
	ar& NVP(ps_uniforms);
	ar& NVP(tess_uniforms);

	if (Archive::is_loading::value)
	{
		//   if (include_file->is_changed())
		compile();
	}
}