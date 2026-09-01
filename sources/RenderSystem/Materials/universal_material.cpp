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

	transparent = context->transparent;

	// Only for alpha-cutout materials -- see vsm_depth_draw's own comment.
	// VSM has no tessellation support (mesh_shader_vsm.hlsl is mesh-shader-
	// only, no HS/DS stage), so unlike depth_draw/gbuffer above this never
	// wires a tess variant even when `tess` is non-empty -- a tessellated
	// cutout material simply keeps its VSM shadow un-displaced, same
	// limitation VSM already has for tessellated opaque materials via the
	// plain VSMDepthDraw PSO.
	if (transparent)
	{
		vsm_depth_draw = std::make_shared<PSOS::VSMDepthDrawMaterial>(RenderSystem::get().device(), [&](SimpleGraphicsPSO& target, PSOS::VSMDepthDrawMaterial::Keys&)
		{
			target.name += std::to_string(id);
			// PS_VSM_DEPTH, not PS -- mesh_shader_vsm.hlsl's vertex output is
			// much smaller than mesh_shader.hlsl's (no normal/tangent/
			// binormal/motion data), so this can't reuse the same PS entry
			// depth_draw/gbuffer above do. See PS_VSM_DEPTH's own comment in
			// UniversalMaterial.hlsl.
			target.pixel = { pixel, "PS_VSM_DEPTH", HAL::ShaderOptions::None, context->get_pixel_result().macros, true };
			target.topology = HAL::PrimitiveTopologyType::TRIANGLE;
		});
	}

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

	// Mirrors MaterialContext::generate_uniform_struct()'s own fallback: an
	// empty uniform list still gets "struct MaterialCB { int dummy; };" on
	// the HLSL side (a material with only textures, no scalar/vector params,
	// is a real, common case), so the CPU-side buffer can never be truly
	// empty either -- a 0-byte dynamic CBV against a 4-byte-expecting shader
	// struct was corrupting the whole MaterialInfo binding (textures
	// included), not just leaving an unused dummy field, making
	// texture-only materials render black.
	if (data.empty())
		data.resize(sizeof(int));

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

		// pixel_data just changed (e.g. a slider drag) -- let any attached
		// MaterialPreviewSession know its cached preview PSO/dispatch is stale.
		preview_generation++;
	}

	if (need_update_uniforms || need_update_compiled)
		mark_contents_changed();

	need_update_uniforms = false;
	need_update_compiled = false;


}

// Phase 5.19 (RTX blocker search): finds which texture (if any) directly
// drives the material graph's opacity output, so VSM's inline RayQuery
// candidate loop can sample it later without needing the material's
// compiled shader (inline ray tracing has no local root signature to reach
// per-material shader code with -- see VSM_ShadowResolve.hlsl's own
// comment). Deliberately narrow: only recognizes the single common shape
// AssimpLoader's own opacity import produces (get_opacity() <- a
// SamplingNode's output <- a TextureAssetNode) -- a one-hop-then-one-hop
// walk, not a general graph evaluator. Anything else (opacity computed by
// a value node, combined from multiple textures, driven by a chain of
// intermediate math nodes) returns nullptr, and the caller leaves
// opacity_texture_index at its "unknown" sentinel -- that material's
// candidate hits are then always committed by the verify ray (today's
// behavior, not a regression, just not improved yet for that case).
static TextureSRVParams::ptr find_opacity_texture(MaterialGraph* graph)
{
	auto opacity = graph->get_opacity();
	if (!opacity || opacity->input_connections.empty())
		return nullptr;

	auto src = (*opacity->input_connections.begin())->from;
	auto sampling = src ? dynamic_cast<SamplingNode*>(src->owner) : nullptr;
	if (!sampling)
		return nullptr;

	auto tex_input = sampling->get_texture_input();
	if (!tex_input || tex_input->input_connections.empty())
		return nullptr;

	auto tex_src = (*tex_input->input_connections.begin())->from;
	auto tex_node = tex_src ? dynamic_cast<TextureAssetNode*>(tex_src->owner) : nullptr;
	if (!tex_node)
		return nullptr;

	return tex_node->get_texture_info();
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
	elem[0].is_transparent = transparent ? 1 : 0;

	// ~0u ("unknown") unless find_opacity_texture() recognizes a simple
	// direct texture->opacity wiring -- see its own comment. Stored as the
	// RAW bindless descriptor-heap index of that one texture (texture_srvs[
	// idx].get_offset(), same accessor material_cb itself uses above) --
	// deliberately NOT an index into MaterialInfo.textures[]: that array
	// lives inline after MaterialInfo's own [dynamic] MaterialCB data,
	// whose byte size varies per material's own uniform payload, so its
	// real offset isn't knowable without the per-material compiled shader
	// (see VSM_ShadowResolve.hlsl's own comment on why that's unavailable
	// to an inline RayQuery). Reading straight from texture_srvs sidesteps
	// that entirely -- ResourceDescriptorHeap[opacity_texture_index] is a
	// plain Texture2D, unconditionally, from anywhere.
	elem[0].opacity_texture_index = ~0u;
	if (auto opacity_tex = find_opacity_texture(graph.get().get()))
	{
		auto srv_list = context->get_textures();
		auto it = std::find(srv_list.begin(), srv_list.end(), opacity_tex);
		if (it != srv_list.end())
		{
			size_t idx = it - srv_list.begin();
			if (idx < texture_srvs.size())
				elem[0].opacity_texture_index = texture_srvs[idx].get_offset();
		}
	}

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
	++universal_material::pipeline_epoch;
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
		preview_source_generation++;
		preview_generation++;
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

std::atomic<uint64_t> materials::universal_material::pipeline_epoch{ 1 };

materials::Pipeline::ptr materials::universal_material::get_pipeline()
{
	return pipeline;
}

int materials::universal_material::get_preview_slot(::FlowGraph::Node* node)
{
	return context ? context->get_preview_slot(node) : -1;
}

int materials::universal_material::get_preview_slot_count()
{
	return context ? context->get_preview_slot_count() : 0;
}

ShaderSource materials::universal_material::get_preview_shader_source()
{
	return context ? context->get_preview_result() : ShaderSource();
}

ShaderSource materials::universal_material::get_pixel_shader_source()
{
	return context ? context->get_pixel_result() : ShaderSource();
}

void materials::universal_material::render_preview(HAL::ComputeContext& compute, PSOS::MaterialPreview::ptr preview_pso, HLSL::RWTexture2DArray<float4> results, ivec2 res)
{
	if (!preview_pso)
		return;

	compute.set_signature(Layouts::DefaultLayout);
	compute.set_pipeline(preview_pso->GetPSO());

	Slots::MaterialPreviewInfo data;
	data.GetTextures() = texture_srvs;
	data.GetData()     = pixel_data;
	data.GetResults()  = results;
	compute.set(data);

	compute.dispatch(res, ivec2(8, 8));
}

void materials::universal_material::render_preview_3d(HAL::GraphicsContext& graphics, PSOS::MaterialPreview3D::ptr preview_pso, HLSL::RWTexture2DArray<float4> results)
{
	if (!preview_pso)
		return;

	graphics.set_signature(Layouts::DefaultLayout);
	graphics.set_pipeline(preview_pso->GetPSO());

	Slots::MaterialPreviewInfo data;
	data.GetTextures() = texture_srvs;
	data.GetData()     = pixel_data;
	data.GetResults()  = results;
	graphics.set(data);
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
