module Graphics:Materials.PreviewSession;
import RenderSystem;

import :EngineAssets;
import HAL;
import Core;

using namespace HAL;

materials::MaterialPreviewSession::MaterialPreviewSession(universal_material* material) : material(material)
{
}

void materials::MaterialPreviewSession::rebuild_pso()
{
	auto header = EngineAssets::material_preview_header.get_asset();
	auto src = material->get_preview_shader_source();
	auto preview_source = src.uniforms + header->get_data() + src.text;

	// Same reasoning as the old PipelinePasses ctor: drive init_pso() +
	// override + create() manually rather than PSOS::MaterialPreview's
	// normal (device, modifier) ctor, since the generated init_pso()
	// re-clobbers mpso.compute.file_name/entry_point to the .sig's static
	// default right after any modifier callback runs.
	auto macros = src.macros;
	if (want_3d)
		macros.emplace_back("PREVIEW_3D", "1");

	pso = std::make_shared<PSOS::MaterialPreview>();
	PSOS::MaterialPreview::Keys key;
	auto mpso = pso->init_pso(key, nullptr);
	mpso.compute = { preview_source, "CS", HAL::ShaderOptions::None, macros, true };
	pso->psos[key] = mpso.create(RenderSystem::get().device());

	built_source_generation = material->preview_source_generation;
	built_3d = want_3d;

	// The results texture holds output from the *old* PSO -- force
	// get_slice_view()'s dispatched_generation check to redispatch even
	// though material->preview_generation itself hasn't moved (a mode
	// toggle isn't a material change).
	dispatched_generation = ~0u;
}

void materials::MaterialPreviewSession::dispatch()
{
	int count = material->get_preview_slot_count();

	if (count <= 0)
	{
		results = nullptr;
		slice_views.clear();
		dispatched_generation = material->preview_generation;
		return;
	}

	bool need_views = !results || (int)results->get_desc().as_texture().ArraySize != count;

	if (need_views)
	{
		results.reset(new HAL::Texture(RenderSystem::get().device(),
			HAL::ResourceDesc::Tex2D(HAL::Format::R32G32B32A32_FLOAT, { preview_resolution, preview_resolution }, count, 1,
				HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess)));
	}

	auto list = RenderSystem::get().device().get_frame_manager().begin_frame()->start_list(L"MaterialPreview", HAL::CommandListType::DIRECT, true);

	material->render_preview(list->get_compute(), pso, results->texture_2d().rwTexture2DArray, ivec2(preview_resolution));

	// This resource is only touched by ad hoc immediate lists (never the
	// FrameGraph), so its tracked GPU state doesn't get kept in sync
	// automatically. Explicitly transition to the desired read state and
	// pin it as the resting state (same fix as Texture.cpp's upload path)
	// so later lists -- the GUI's SRV read, or our own next dispatch --
	// start from an accurate assumption instead of a stale one.
	{
		auto desired = results->resource->get_state_manager().get_desired_state();
		list->transition(results->resource.get(), desired);
		results->resource->get_state_manager().set_resting_state(desired.layout);
	}

	if (need_views)
	{
		// Persistent allocator, not *list (a throwaway per-call list) --
		// these views need to survive across many future GUI draw calls.
		auto& static_data = RenderSystem::get().device().get_static_gpu_data();

		slice_views.clear();
		slice_views.reserve(count);
		for (int i = 0; i < count; i++)
			slice_views.push_back(results->texture_2d().create_2d_slice(i, static_data));
	}

	list->execute_and_wait();

	dispatched_generation = material->preview_generation;
}

HAL::Texture2DView materials::MaterialPreviewSession::get_slice_view(int slot)
{
#ifdef HAL_BACKEND_VULKAN
	return HAL::Texture2DView();
#else
	if (!pso || built_source_generation != material->preview_source_generation || built_3d != want_3d)
		rebuild_pso();

	if (dispatched_generation != material->preview_generation)
		dispatch();

	if (slot < 0 || slot >= (int)slice_views.size())
		return HAL::Texture2DView();

	return slice_views[slot];
#endif
}

namespace
{
	std::map<::FlowGraph::graph*, std::unique_ptr<materials::MaterialPreviewSession>> g_preview_sessions;
}

void materials::MaterialPreviewSession::open(::FlowGraph::graph* g)
{
	auto* mg = dynamic_cast<MaterialGraph*>(g);
	if (!mg)
		return;

	auto* mat = dynamic_cast<universal_material*>(mg->preview_material);
	if (!mat)
		return;

	// on_add() can fire more than once for the same canvas -- don't discard
	// an already-built PSO/dispatch just because we got asked again.
	if (g_preview_sessions.find(g) != g_preview_sessions.end())
		return;

	g_preview_sessions[g] = std::make_unique<MaterialPreviewSession>(mat);
}

void materials::MaterialPreviewSession::close(::FlowGraph::graph* g)
{
	g_preview_sessions.erase(g);
}

materials::MaterialPreviewSession* materials::MaterialPreviewSession::find(::FlowGraph::graph* g)
{
	auto it = g_preview_sessions.find(g);
	return it != g_preview_sessions.end() ? it->second.get() : nullptr;
}
