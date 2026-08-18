module Graphics:Materials.PreviewSession;
import RenderSystem;

import :EngineAssets;
import :Scene;
import :MeshAsset;
import :Camera;
import HAL;
import Core;

using namespace HAL;

materials::MaterialPreviewSession::MaterialPreviewSession(universal_material* material) : material(material)
{
}

void materials::MaterialPreviewSession::rebuild_pso()
{
	auto src = material->get_preview_shader_source();

	// Same reasoning as the old PipelinePasses ctor: drive init_pso() +
	// override + create() manually rather than the PSOS type's normal
	// (device, modifier) ctor, since the generated init_pso() re-clobbers
	// the overridden stage's file_name/entry_point to the .sig's static
	// default right after any modifier callback runs.
	if (want_3d)
	{
		// Inline text, not a file compile -- unlike stencil.hlsl's PS_COLOR,
		// we don't get vertex_output for free from however the engine wires
		// mesh+pixel files together for a normal GraphicsPSO compile, so
		// material_preview_3d.hlsl pulls mesh_shader.hlsl in itself (see
		// that file).
		auto header = EngineAssets::material_preview_3d_header.get_asset();
		auto preview_source = src.uniforms + header->get_data() + src.text;

		pso3d = std::make_shared<PSOS::MaterialPreview3D>();
		PSOS::MaterialPreview3D::Keys key;
		auto mpso = pso3d->init_pso(key, nullptr);
		mpso.pixel = { preview_source, "PS_PREVIEW", HAL::ShaderOptions::None, src.macros, true };

		// See mesh_shader.hlsl's DISABLE_MESHLET_CULL comment -- our mesh
		// instance's cull data lives in the same shared global buffers the
		// main editor scene concurrently touches every frame, and boundary
		// meshlets were flickering in/out under that contention.
		mpso.amplification = { "shaders/mesh_shader.hlsl", "AS", HAL::ShaderOptions::None,
			{ HAL::shader_macro("DISABLE_MESHLET_CULL") }, false };

		pso3d->psos[key] = mpso.create(RenderSystem::get().device());
		pso = nullptr;
	}
	else
	{
		auto header = EngineAssets::material_preview_header.get_asset();
		auto preview_source = src.uniforms + header->get_data() + src.text;

		pso = std::make_shared<PSOS::MaterialPreview>();
		PSOS::MaterialPreview::Keys key;
		auto mpso = pso->init_pso(key, nullptr);
		mpso.compute = { preview_source, "CS", HAL::ShaderOptions::None, src.macros, true };
		pso->psos[key] = mpso.create(RenderSystem::get().device());
		pso3d = nullptr;
	}

	built_source_generation = material->preview_source_generation;
	built_3d = want_3d;

	// The results texture holds output from the *old* PSO -- force
	// get_slice_view()'s dispatched_generation check to redispatch even
	// though material->preview_generation itself hasn't moved (a mode
	// toggle isn't a material change).
	dispatched_generation = ~0u;
}

void materials::MaterialPreviewSession::ensure_3d_scene()
{
	if (preview_scene)
		return;

	// Lives for the session's lifetime -- never enters any FrameGraph, never
	// gets a real material assigned (we bypass the indirect mesh-shader
	// pipeline entirely and dispatch_mesh it directly in dispatch(), so the
	// mesh's own default material is irrelevant).
	preview_mesh.reset(new MeshAssetInstance(EngineAssets::material_tester.get_asset()));

	preview_scene = std::make_shared<Scene>();
	preview_scene->add_child(preview_mesh);
}

void materials::MaterialPreviewSession::dispatch()
{
	int count = material->get_preview_slot_count();

	if (count <= 0)
	{
		results = nullptr;
		slice_views.clear();
		dispatched_generation = material->preview_generation;
		dispatched_orbit = material->preview_orbit;
		dispatched_zoom  = material->preview_zoom;
		return;
	}

	bool need_views = !results || (int)results->get_desc().as_texture().ArraySize != count;

	if (need_views)
	{
		results.reset(new HAL::Texture(RenderSystem::get().device(),
			HAL::ResourceDesc::Tex2D(HAL::Format::R32G32B32A32_FLOAT, { preview_resolution, preview_resolution }, count, 1,
				HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess)));
	}

	auto frame = RenderSystem::get().device().get_frame_manager().begin_frame();
	auto list  = frame->start_list(L"MaterialPreview", HAL::CommandListType::DIRECT, true);

	if (want_3d)
	{
		ensure_3d_scene();

		// Real rasterization, not a full-screen dispatch -- pixels outside
		// the mesh silhouette never get a PS invocation, so they'd otherwise
		// keep whatever was in the texture before (garbage on first use,
		// stale content on redispatch).
		list->clear_uav(results->texture_2d().rwTexture2DArray, vec4(0.05f, 0.05f, 0.05f, 1.0f));

		preview_scene->update_transforms();
		auto mn = preview_scene->get_min();
		auto mx = preview_scene->get_max();

		float base_dist = (mx - mn).length();
		if (base_dist < 0.001f) base_dist = 1.0f;

		// Shared with whatever full preview of this material the user last
		// orbited/zoomed (asset_preview_content, via universal_material::
		// preview_orbit/preview_zoom) -- so every node's thumbnail matches
		// that view instead of a fixed 3/4 angle.
		float yaw = material->preview_orbit.x, pitch = material->preview_orbit.y;
		vec3 dir = vec3(
			Math::cos(pitch) * Math::sin(yaw),
			Math::sin(pitch),
			Math::cos(pitch) * Math::cos(yaw));

		preview_cam.target   = (mn + mx) / 2;
		preview_cam.position = preview_cam.target + dir * (base_dist * material->preview_zoom);
		preview_cam.set_projection_params(Math::pi / 4, 1.0f, 0.01f, base_dist * 4.0f + 1.0f);
		preview_cam.update();

		dispatched_orbit = material->preview_orbit;
		dispatched_zoom  = material->preview_zoom;

		preview_scene->update(*frame);

		// Scene::update() alone only refreshes CPU-side handle allocations --
		// SceneFrameManager::prepare() is the actual GPU upload of the node/
		// mesh/material buffers mesh_shader.hlsl reads (see PreSceneSystem.cpp's
		// PreScene pass, which every real render calls once per frame). Without
		// this, our own dispatch_mesh call reads stale/empty global buffers and
		// renders nothing -- what visibly rendered before was riding on the
		// *main* scene's own PreScene pass happening to run around the same
		// time, racing unpredictably against ours (holes, flicker).
		SceneFrameManager::get().prepare(list, *preview_scene);

		if (!preview_depth)
		{
			preview_depth.reset(new HAL::Texture(RenderSystem::get().device(),
				HAL::ResourceDesc::Tex2D(HAL::Format::R32_TYPELESS, { preview_resolution, preview_resolution }, 1, 1,
					HAL::ResFlags::DepthStencil)));
		}

		auto& graphics = list->get_graphics();
		graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
		graphics.set_viewport(vec4(0, 0, (float)preview_resolution, (float)preview_resolution));
		graphics.set_scissor({ 0, 0, preview_resolution, preview_resolution });

		// Real depth test/write -- overlapping front/back geometry on the
		// mesh has nothing else to resolve visibility per pixel (no rtv,
		// UAV-only PS -- see material_preview.sig).
		{
			RT::DepthOnly rt;
			rt.GetDepth() = preview_depth->texture_2d().depthStencil;
			auto rtv = rt.compile(*list);
			graphics.set_rtv(rtv, RTOptions::Default | RTOptions::ClearDepth);
		}

		// Binds the PSO + MaterialPreviewInfo (results/textures/uniforms).
		material->render_preview_3d(graphics, pso3d, results->texture_2d().rwTexture2DArray);

		graphics.set(preview_scene->compiledScene);
		{
			Slots::FrameInfo frameInfo;
			frameInfo.GetCamera() = preview_cam.camera_cb.current;
			graphics.set(frameInfo);
		}

		// Direct (non-indirect) dispatch_mesh, same pattern StencilRenderer
		// uses to draw arbitrary scene meshes with a custom PSO -- bypasses
		// the GPU-driven indirect GatherPipeline entirely, so there's no
		// dependency on material_tester's own (irrelevant here) material.
		for (auto& m : preview_mesh->rendering)
		{
			graphics.set(m.compiled_mesh_info);
			graphics.set(m.mesh_instance_info);
			graphics.dispatch_mesh(m.dispatch_mesh_arguments);
		}
	}
	else
	{
		material->render_preview(list->get_compute(), pso, results->texture_2d().rwTexture2DArray, ivec2(preview_resolution));
	}

	// Leave the result readable for whoever picks it up next -- the GUI's SRV
	// read, or our own next dispatch. The resting layout follows from the
	// resource's flags, so recording the use is all that is needed.
	{
		auto layout = HAL::resting_layout(results->resource.get());
		list->add_resource_usage(results->resource.get(), check(layout & HAL::TextureLayout::UNORDERED_ACCESS)
			? HAL::ResourceStates::UNORDERED_ACCESS
			: HAL::ResourceStates::SHADER_RESOURCE);
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
	// pso vs. pso3d depending on mode -- rebuild_pso() nulls out whichever
	// one isn't current, so checking the wrong one is always "missing" and
	// forces a full PSO rebuild (shader recompile) + redispatch on every
	// single call, i.e. every frame -- the actual cause of the "flickering"
	// (a fresh render each frame with whatever GPU scheduling variance
	// happens to land) rather than anything about the geometry itself.
	bool pso_missing = want_3d ? !pso3d : !pso;
	if (pso_missing || built_source_generation != material->preview_source_generation || built_3d != want_3d)
		rebuild_pso();

	bool orbit_changed = want_3d &&
		(dispatched_orbit.x != material->preview_orbit.x || dispatched_orbit.y != material->preview_orbit.y ||
		 dispatched_zoom != material->preview_zoom);

	if (dispatched_generation != material->preview_generation || orbit_changed)
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

GUI::base::ptr materials::open_material_editor(::FlowGraph::graph::ptr g)
{
	auto canva = GUI::Elements::FlowGraph::manager::get().create_canvas(g);
	if (!canva)
		return nullptr;

	auto* mg  = dynamic_cast<MaterialGraph*>(g.get());
	auto* mat = mg ? dynamic_cast<universal_material*>(mg->preview_material) : nullptr;

	// Not an actual material graph (e.g. a nested MaterialFunction opened
	// via "edit in new tab", or the frame-flow-graph debug view) -- just the
	// bare canvas, same as before this feature existed.
	if (!mat || !create_settings_panel_hook)
		return canva;

	auto editor_dock = std::make_shared<GUI::Elements::dock_base>();
	editor_dock->docking = GUI::dock::FILL;
	editor_dock->add_child(canva);

	auto right_dock = editor_dock->get_dock(GUI::dock::RIGHT);
	right_dock->size = { 300.0f, 0.0f };

	auto panel = create_settings_panel_hook(mat, g.get());
	right_dock->get_tabs()->add_page("Material Settings", panel);

	return editor_dock;
}
