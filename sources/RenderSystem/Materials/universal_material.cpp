module Graphics:Materials.UniversalMaterial;
import RenderSystem;



import :EngineAssets;
import :RTX;


//import FrameGraph;
import HAL;
import Core;

using namespace HAL;

void removeme() // TODO: VS issue - make dummy unused func to compile entire cpp =[
{
	auto res = Serializer::deserialize<std::string>("");
}

REGISTER_TYPE(materials::universal_material);
CEREAL_REGISTER_POLYMORPHIC_RELATION(MaterialAsset, materials::universal_material);
CEREAL_REGISTER_DYNAMIC_INIT(universal_material);
CEREAL_FORCE_REGISTER(materials::universal_material);
// Register Asset→MaterialAsset first so the BFS in PolymorphicVirtualCaster
// can immediately build the transitive path Asset→universal_material.
CEREAL_FORCE_REGISTER_RELATION(Asset, MaterialAsset);
CEREAL_FORCE_REGISTER_RELATION(MaterialAsset, materials::universal_material);

REGISTER_TYPE(materials::PipelinePasses);
CEREAL_REGISTER_POLYMORPHIC_RELATION(materials::Pipeline, materials::PipelinePasses);
CEREAL_FORCE_REGISTER(materials::PipelinePasses);
CEREAL_FORCE_REGISTER_RELATION(materials::Pipeline, materials::PipelinePasses);

REGISTER_TYPE(materials::PipelineSimple);
CEREAL_REGISTER_POLYMORPHIC_RELATION(materials::Pipeline, materials::PipelineSimple);
CEREAL_FORCE_REGISTER(materials::PipelineSimple);
CEREAL_FORCE_REGISTER_RELATION(materials::Pipeline, materials::PipelineSimple);

																						 
// ---------------------------------------------------------------------------
// PipelinePasses
// ---------------------------------------------------------------------------

materials::PipelinePasses::PipelinePasses(UINT id, std::string pixel, std::string tess, std::string voxel, std::string raytracing, MaterialContext::ptr context) :Pipeline(id)
{
	depth_draw = std::make_shared<PSOS::DepthDraw>(RenderSystem::get().device(),[&](SimpleGraphicsPSO& target, PSOS::DepthDraw::Keys& )
	{
		target.name += std::to_string(id);
		target.pixel = { pixel, "PS", HAL::ShaderOptions::None,context->get_pixel_result().macros, true };

		if (!tess.empty()) {
			target.hull = { tess, "HS", HAL::ShaderOptions::None,context->get_tess_result().macros, true };
			target.domain = { tess, "DS", HAL::ShaderOptions::None,context->get_tess_result().macros, true };
			target.topology = HAL::PrimitiveTopologyType::PATCH;
		}
		else
		{
			target.topology = HAL::PrimitiveTopologyType::TRIANGLE;
		}
	});

	gbuffer = std::make_shared<PSOS::GBufferDraw>(RenderSystem::get().device(),[&](SimpleGraphicsPSO& target, PSOS::GBufferDraw::Keys& )
	{
		target.name += std::to_string(id);
		target.pixel = { pixel, "PS", HAL::ShaderOptions::None,context->get_pixel_result().macros, true };

		if (!tess.empty()) {
			target.hull = { tess, "HS", HAL::ShaderOptions::None,context->get_tess_result().macros, true };
			target.domain ={ tess, "DS", HAL::ShaderOptions::None,context->get_tess_result().macros, true };
			target.topology = HAL::PrimitiveTopologyType::PATCH;
		}
		else
		{
			target.topology = HAL::PrimitiveTopologyType::TRIANGLE;
		}
	});

	voxelization = std::make_shared<PSOS::Voxelization>(RenderSystem::get().device(),[&](SimpleGraphicsPSO& target, PSOS::Voxelization::Keys& )
	{
		target.name += std::to_string(id);
		target.pixel = { pixel, "PS_VOXEL", HAL::ShaderOptions::None ,context->get_pixel_result().macros, true };

		if (!tess.empty()) {
			target.hull = { tess, "HS", HAL::ShaderOptions::None,context->get_tess_result().macros, true };
			target.domain = { tess, "DS", HAL::ShaderOptions::None,context->get_tess_result().macros, true };
			target.topology = HAL::PrimitiveTopologyType::PATCH;
		}
		else
		{
			target.topology = HAL::PrimitiveTopologyType::TRIANGLE;
		}
	});

	raytrace_lib = HAL::library_shader::get_resource({ raytracing, "" , HAL::ShaderOptions::None, context->hit_shader.macros, true });
}

void materials::PipelinePasses::set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion)
{
	if (render_type == RENDER_TYPE::DEPTH)
		graphics.set_pipeline(depth_draw->GetPSO(PSOS::DepthDraw::HiZOcclusion.Use(hiz_occlusion)));
	else
		if (render_type == RENDER_TYPE::PIXEL)
			graphics.set_pipeline(gbuffer->GetPSO(PSOS::GBufferDraw::HiZOcclusion.Use(hiz_occlusion)));
		else
		{
			graphics.set_pipeline(voxelization->GetPSO(PSOS::Voxelization::Dynamic.Use(type == MESH_TYPE::DYNAMIC)));
		}
}


// ---------------------------------------------------------------------------
// PipelineManager
// ---------------------------------------------------------------------------

materials::Pipeline::ptr materials::PipelineManager::get_pipeline(Pipeline::ptr orig)
{
	std::lock_guard<std::mutex> g(m);

	auto& pip = pipelines[orig->hash];
	if (!pip)
		pip = orig;

	return pip;
}

materials::Pipeline::ptr materials::PipelineManager::get_pipeline(std::string pixel, std::string tess, std::string voxel, std::string raytracing, MaterialContext::ptr context)
{
	std::lock_guard<std::mutex> g(m);
	auto hash = crc32(pixel + tess);
	auto&& pip = pipelines[hash];

	if (!pip)
	{
		auto pipeline = std::make_shared<PipelinePasses>((UINT)pipelines.size(), pixel,tess,voxel,raytracing,context);
		pipeline->hash = hash;
		pip = pipeline;
	}

	return pip;
}


// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

DynamicData generate_data(std::vector<Uniform::ptr>& un)
{
	DynamicData data;

	int offset = 0;

	for (auto& u : un)
	{
		int need_size = u->type.get_size();

		while (offset > 0 && (offset + need_size) > 16)
		{
			data.emplace_back(std::byte(0));
			offset = (offset + 1) % 16;
		}

		std::byte* ptr = nullptr;


		if (u->type == ShaderParams::get().FLOAT1)
		{
			ptr = reinterpret_cast<std::byte*>(&u->value.f_value);
		}

		if (u->type == ShaderParams::get().FLOAT2)
		{
			ptr = reinterpret_cast<std::byte*>(&u->value.f2_value);
		}

		if (u->type == ShaderParams::get().FLOAT3)
		{
			ptr = reinterpret_cast<std::byte*>(&u->value.f3_value);
		}

		if (u->type == ShaderParams::get().FLOAT4)
		{
			ptr = reinterpret_cast<std::byte*>(&u->value.f4_value);
		}

		data.insert(data.end(), ptr, ptr + need_size);


		offset = (offset + need_size) % 16;
	}

	return data;
}


// ---------------------------------------------------------------------------
// universal_material — heavy rendering logic
// ---------------------------------------------------------------------------

void materials::universal_material::update()
{
#ifdef HAL_BACKEND_VULKAN
	return; // material pipeline not active on Vulkan yet
#endif
	//std::lock_guard<std::mutex> g(m);
	PROFILE(L"universal_material");

	// Catch header reloads that happened before this material existed (or whose
	// change notification we missed): the shared BinaryAsset's version will be
	// ahead of what we last generated against.
	if ((include_file          && include_file->get_version()          != ps_header_version) ||
	    (include_file_raytacing && include_file_raytacing->get_version() != rt_header_version))
		need_regenerate_material = true;

	if (need_regenerate_material)
	{
		generate_material();
		need_regenerate_material = false;
	}

	if (need_update_uniforms)
	{
		auto generate = [this](std::vector<Uniform::ptr>& un)
		{
			pixel_data = generate_data(un);
			if (pixel_data.empty())
				return;
		};

		generate(ps_uniforms);

		need_update_compiled = true;
	}

	if (need_update_compiled)
	{
		material_info.GetTextures() = texture_srvs;// textures_handle ? (UINT)textures_handle.get_offset() : 0;
		material_info.GetData() = pixel_data;
		compiled_material_info = material_info.compile(RenderSystem::get().device().get_static_gpu_data());
		local_addr = compiled_material_info.compiled().get_offset();

		//local_addr_ids = to_native(compiled_material_info.offsets_cb);
		{
			auto elem = info_handle.map();// universal_material_info_part_manager::get().map_elements(info_handle.get_offset(), 1);
			elem[0].pipeline_id = pipeline->get_id();
			elem[0].material_cb = compiled_material_info.compiled().get_offset();

			info_handle.write(0, elem);

		}

		update_rtx();
	}

	if (need_update_uniforms || need_update_compiled)
		mark_contents_changed();

	need_update_uniforms = false;
	need_update_compiled = false;


}

void materials::universal_material::compile()
{
	start_changing_contents();

	handlers.clear();

	texture_srvs.resize(textures.size());
	texture_feedbacks.resize(textures.size());

	for (int i = 0; i < textures.size(); i++)
	{
			TextureAsset::ptr tex = *textures[i];

		if (tex && tex->get_texture()->texture_2d())
		{
			texture_srvs[i] = tex->get_texture()->texture_2d().texture2D;
		texture_feedbacks[i] = tex->get_texture()->texture_2d().feedback;
		}

		else
			texture_srvs[i] = EngineAssets::missing_texture.get_asset()->get_texture()->texture_2d().texture2D;
	}


	auto generate = [this](std::vector<Uniform::ptr>& un)
	{
		pixel_data = generate_data(un);

		for (auto u : un)
		{
			handlers.emplace_back();
			u->on_change.register_handler(&handlers.back(), [this](Uniform* u)
				{
					need_update_uniforms = true;
				});
		}
	};


	generate(ps_uniforms);
	material_info.GetTextures() = texture_srvs;// textures_handle ? (UINT)textures_handle.get_offset() : 0;
	material_info.GetData() = pixel_data;
	compiled_material_info = material_info.compile(RenderSystem::get().device().get_static_gpu_data());

	local_addr = compiled_material_info.compiled().get_offset();
	//local_addr_ids = to_native(compiled_material_info.offsets_cb);

	if (!info_handle)
	{
		info_handle = universal_material_info_part_manager::get().allocate(1);
	}

	auto elem = info_handle.map();
	elem[0].pipeline_id = pipeline->get_id();
	elem[0].material_cb = compiled_material_info.compiled().get_offset();
	info_handle.write(0, elem);

	need_update_compiled = false;
	need_update_uniforms = false;

	RTX::get().rtx.init_material(this);
	end_changing_contents();
}

void materials::universal_material::generate_texture_handles()
{

}

void materials::universal_material::generate_material()
{
#ifdef HAL_BACKEND_VULKAN
	// Material PSO compilation requires the 3D pipeline (GBufferDraw, Voxelization,
	// raytracing libs, etc.) which is not yet active on Vulkan. Skip generation
	// silently — materials will be re-generated when the 3D pipeline is enabled.
	return;
#endif
	//   std::lock_guard<std::mutex> g(m);
	if (!context)
		context.reset(new MaterialContext);

	if (!info_handle)
	{
		info_handle = universal_material_info_part_manager::get().allocate(1);
	}



	context->start(include_file->get_data(), graph.get().get());

	// Whether the graph drives opacity — used later by RTXColorPass.
	transparent = context->transparent;


	auto ps_str = context->get_pixel_result().uniforms + include_file->get_data() + context->get_pixel_result().text;
	auto tess_orig_shader = context->get_tess_result().text;
	auto tess_str = tess_orig_shader.empty() ? std::string() : (context->get_tess_result().uniforms + include_file->get_data() + tess_orig_shader);
	auto voxel_str = context->get_voxel_result().uniforms + include_file->get_data() + context->get_voxel_result().text;



	auto raytracing_str = context->hit_shader.uniforms + include_file_raytacing->get_data() + context->hit_shader.text;


	raytracing_lib = HAL::library_shader::get_resource({ raytracing_str, "" , ShaderOptions::None, context->hit_shader.macros, true });
	pipeline = PipelineManager::get().get_pipeline(ps_str, tess_str, voxel_str, raytracing_str, context);
	ps_uniforms = context->uniforms_ps;

	// Remember which header revisions this generation baked in.
	if (include_file)           ps_header_version = include_file->get_version();
	if (include_file_raytacing) rt_header_version = include_file_raytacing->get_version();


	//	tess_uniforms = context->uniforms_tess;


		//generate_texture_handles();

		//   if (textures_changed)
	{
	/*	for (auto& t : textures)
		{
			t->asset.destroy();
		}*/

		textures.clear();
		for(auto &t:context->get_textures())
		{
			textures.emplace_back(register_asset(t->asset->get_ptr<TextureAsset>()));
		}


		compile();
	}

	//  if ((textures_changed || shaders_changed))
	mark_contents_changed();

	need_regenerate_material = false;

	on_change();
}

materials::universal_material::universal_material(MaterialGraph::ptr graph) : include_file(this), include_file_raytacing(this)
{


	include_file = EngineAssets::material_header.get_asset();
	include_file_raytacing = EngineAssets::material_raytracing_header.get_asset();
	this->graph = BinaryData<MaterialGraph>(graph);
	graph->add_listener(this, false);
	graph->preview_material = this; // let the graph-output node build a live preview
	need_regenerate_material = true;
	need_update_uniforms = true;
	generate_material();
}

void materials::universal_material::update_rtx()
{
	if (!RenderSystem::get().device().is_rtx_supported()) return;
	RTX::get().rtx.update_material(this);



}

D3D_PRIMITIVE_TOPOLOGY materials::render_pass::get_topology()
{
	return  ds_shader ? D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

size_t materials::render_pass::get_pipeline_id()
{
	return pipeline_id;
}


//CEREAL_REGISTER_DYNAMIC_INIT(myclasses)

void materials::render_pass::init_pipeline_id()
{
	pipeline_id = 2166136261U;
	pipeline_id = HashIterate(ps_shader ? ps_shader->id : 0, pipeline_id);
	pipeline_id = HashIterate(ds_shader ? ds_shader->id : 0, pipeline_id);
	pipeline_id = HashIterate(hs_shader ? hs_shader->id : 0, pipeline_id);
}

materials::PipelineSimple::PipelineSimple(UINT id, pixel_shader::ptr pixel) : Pipeline(id)
{
}

void materials::PipelineSimple::set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion)
{
	//	pipeline.pixel = pixel;
}

UINT materials::universal_material::get_material_id()
{
	return (UINT)info_handle.get_offset();
}

materials::Pipeline::ptr materials::universal_material::get_pipeline()
{
	return pipeline;
}

Slots::MaterialInfo& materials::universal_material::get_render_info()
{
	return material_info;
}

void materials::universal_material::set(MESH_TYPE type, MeshRenderContext::ptr&)
{
}

void materials::universal_material::set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion)
{
}
