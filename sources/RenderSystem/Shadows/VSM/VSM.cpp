module Graphics:VSM;

import :VSM;
import :BRDF;
import :EngineAssets;
import :FrameGraphContext;

import HAL;

import Graphics;
using namespace FrameGraph;
using namespace HAL;

float3 VSM::get_position()
{
	pos_mutex.lock();
	auto p = position;
	pos_mutex.unlock();
	return p;
}

void VSM::set_position(float3 p)
{
	pos_mutex.lock();
	bool changed = position.x != p.x || position.y != p.y || position.z != p.z;
	position = p;
	if (changed)
		light_change_pending.fill(true);
	pos_mutex.unlock();
}

namespace
{
	// Shared by every page's render() and by combine(): the rotation-only
	// light-space view used purely to classify "which page/level is this
	// world position in", built the same way PSSM builds its shadow camera.
	camera make_light_view_camera(float3 position)
	{
		camera light_cam;
		light_cam.set_projection_params(0, 1, 0, 1, 1, 1000);
		light_cam.position = position;
		light_cam.up = (float3(0.01, 1, 0.023)).normalize();
		light_cam.update();
		return light_cam;
	}
}

void VSM::attach_scene(std::shared_ptr<Scene> scene)
{
	// Per-page invalidation (Phase 2): map a touched object's world AABB to
	// the (level, page) bits it overlaps, via the same light-view-space
	// projection/clamp math VSMClipmap already uses for page selection --
	// world_to_page's edge-clamp means an object slightly outside a coarse
	// level's grid still marks that level's nearest edge page (conservative,
	// not a correctness bug).
	tracker.attach(scene, [this](scene_object* object)
	{
		vec3 wmin = object->get_min();
		vec3 wmax = object->get_max();

		camera light_view_cam = make_light_view_camera(get_position());
		box bounds_ls = light_view_cam.get_points(wmin, wmax).get_bounds_in(light_view_cam.get_view());
		float2 ls_min(bounds_ls.left, bounds_ls.top);
		float2 ls_max(bounds_ls.right, bounds_ls.bottom);

		for (int level = 0; level < page_table.clipmap.level_count; level++)
		{
			// A level that hasn't rendered yet has no meaningful
			// cached_origin -- it'll get a full-mask render (recentered)
			// the first time it does run, so there's nothing useful to
			// mark here yet.
			if (!level_initialized[level])
				continue;

			ivec2 p0 = page_table.clipmap.world_to_page(level, ls_min, cached_origin[level]);
			ivec2 p1 = page_table.clipmap.world_to_page(level, ls_max, cached_origin[level]);

			uint32_t mask = 0;
			for (int py = p0.y; py <= p1.y; py++)
				for (int px = p0.x; px <= p1.x; px++)
					mask |= 1u << (py * page_table.clipmap.pages_per_level + px);

			tracker.mark_pages(level, mask);
		}
	});
}

void VSM::plan_frame(FrameGraph::Graph& graph)
{
	// Single-threaded, once per frame, strictly before any level's render()
	// is dispatched (see the LevelPlan comment in VSM.ixx for why this has
	// to live here and not in m_level_render).
	auto& sceneinfo = graph.get_context<SceneInfo>();
	auto& caminfo   = graph.get_context<CameraInfo>();
	auto  cam       = caminfo.cam;
	auto  scene     = sceneinfo.scene;
	if (!scene)
		return;

	float3 current_light_pos = get_position();
	camera light_view_cam = make_light_view_camera(current_light_pos);
	float2 cam_pos_ls = (float4(cam->position, 1) * light_view_cam.get_view()).xy;

	auto min = scene->get_min();
	auto max = scene->get_max();
	auto points_all = cam->get_points(min, max);
	auto bounds_all = points_all.get_bounds_in(light_view_cam.get_view());

	uint64_t tick = ++m_frame_id;

	const int pages_side      = page_table.clipmap.pages_per_level;
	const int pages_per_level = pages_side * pages_side;
	uint32_t  all_pages_mask  = (1u << pages_per_level) - 1;

	// Finest level first: acquire_slot_with_priority only ever steals from
	// a level STRICTLY coarser than the requester, so processing in this
	// order means a fine level always gets first pick of whatever's free
	// this frame, and can reclaim an already-resident coarser page if it
	// has to -- never the other way around.
	for (int level = 0; level < page_table.clipmap.level_count; level++)
	{
		LevelPlan plan;
		plan.origin     = page_table.clipmap.grid_origin(level, cam_pos_ls);
		plan.bounds_all = bounds_all;
		plan.valid      = true;

		// Recentered = this level's local page indices now mean a different
		// world region than they did last frame -- the indirection ROW must
		// be rewritten either way (it's indexed by local position), even on
		// frames where every page it now points to happens to already be
		// cached.
		bool recentered = !level_initialized[level] || plan.origin.x != cached_origin[level].x || plan.origin.y != cached_origin[level].y;

		// Read-and-clear this level's sticky flag (see VSM.ixx).
		pos_mutex.lock();
		bool light_moved = light_change_pending[level];
		light_change_pending[level] = false;
		pos_mutex.unlock();

		uint32_t scene_mask = tracker.take_dirty(level) & all_pages_mask;

		// A page that has scrolled entirely off this level's grid (recenter
		// moved past it) isn't covered by any local cell below any more, so
		// the per-cell "not needed" evict never gets a chance to see it --
		// left alone it's a permanent orphan for level 0 specifically
		// (nothing is finer, so nothing can ever steal it back). Sweep it
		// here, every frame, before this level tries to allocate anything;
		// harmless no-op when nothing actually left the grid.
		{
			ivec2 grid_min = page_table.clipmap.abs_page(level, ivec2(0, 0), plan.origin);
			ivec2 grid_max = grid_min + ivec2(pages_side, pages_side);
			page_table.evict_outside_range(level, grid_min, grid_max);
		}

		// Resolve every local cell's physical slot by page IDENTITY (level +
		// abs_page), not by local position -- a page that stays in view
		// keeps its slot and its rendered content across a recenter.
		//
		// Residency culling: a page whose light-space footprint misses
		// bounds_all (the camera-frustum-clipped scene) can't be
		// contributing to anything the camera sees right now -- skip
		// allocating it, freeing budget for pages that matter. Conservative
		// (bounds_all is an AABB, a superset of the true frustum), never a
		// hole.
		uint32_t newly_allocated_mask = 0;
		for (int local = 0; local < pages_per_level; local++)
		{
			ivec2 page      = ivec2(local % pages_side, local / pages_side);
			float2 page_min = page_table.clipmap.page_min(level, page, plan.origin);
			float2 page_max = page_min + float2(page_table.clipmap.page_world_size(level));
			ivec2 abs_page  = page_table.clipmap.abs_page(level, page, plan.origin);

			bool needed = page_max.x >= bounds_all.left && page_min.x <= bounds_all.right
			           && page_max.y >= bounds_all.top  && page_min.y <= bounds_all.bottom;

			if (!needed)
			{
				plan.slots[local] = (int)VSM_INVALID_SLOT;
				// The allocator's reclaim path: a page the camera has
				// panned away from gives its slot back so it's available
				// for whatever's actually needed this frame. Safe to do
				// unconditionally (not just when scene-dirty) because this
				// whole pass is single-threaded -- nothing else is touching
				// the allocator right now.
				page_table.evict_page(level, abs_page);
				continue;
			}

			auto alloc = page_table.allocate_or_touch(level, abs_page, tick);
			plan.slots[local] = (int)alloc.slot;

			if (alloc.slot != VSM_INVALID_SLOT)
				plan.resident_mask |= (1u << local);

			if (alloc.newly_allocated)
			{
				newly_allocated_mask |= (1u << local);
				allocation_exhausted.store(false, std::memory_order_relaxed);
			}
			else if (alloc.slot == VSM_INVALID_SLOT)
			{
				// Pool exhausted AND nothing coarser than this level to
				// steal from either. Log the start of an episode, then only
				// a periodic reminder while it persists, not once per
				// failed cell per frame.
				uint64_t failed = total_failed_allocations.fetch_add(1, std::memory_order_relaxed) + 1;
				bool was_exhausted = allocation_exhausted.exchange(true, std::memory_order_relaxed);
				if (!was_exhausted || (failed % 300 == 0))
				{
					Log::get() << (std::string("VSM: page pool exhausted (physical_page_count=")
						+ std::to_string(page_table.physical_page_count) + "), level=" + std::to_string(level)
						+ ", total failed allocations=" + std::to_string(failed)
						+ " -- affected pages fall back to a coarser level") << Log::endl;
				}
			}
		}

		// newly_allocated/light_moved invalidate that specific page (not the
		// whole level -- a page whose slot/content survived the recenter
		// needs neither); scene_mask adds whatever VSMInvalidationTracker
		// actually marked. Masked to resident_mask: a page with no valid
		// slot this frame -- not needed, or needed but the pool was
		// exhausted -- is never drawn into a slot it doesn't have.
		plan.dirty_mask          = (newly_allocated_mask | scene_mask | (light_moved ? all_pages_mask : 0u)) & plan.resident_mask;
		// A newly-allocated slot has no Hi-Z history; light_moved makes
		// every resident page's existing pyramid stale too (built under the
		// old light projection).
		plan.skip_occlusion_mask = (newly_allocated_mask | (light_moved ? all_pages_mask : 0u)) & plan.resident_mask;

		// Indirection row is stale if local->slot changed for ANY cell --
		// recenter, a residency flip, or a priority steal reassigning a
		// slot this level was pointing at.
		bool row_changed = recentered;
		for (int local = 0; local < pages_per_level; local++)
		{
			if (cached_slots[level][local] != plan.slots[local])
				row_changed = true;
			cached_slots[level][local] = plan.slots[local];
		}
		plan.row_changed = row_changed;

		level_initialized[level] = true;
		cached_origin[level]     = plan.origin;

		m_plan[level] = plan;
	}
}

VSM::VSM()
{
	position = float3(200, 400, 200);

	// Phase 5.4: 6 levels now that residency culling (below) keeps demand
	// within physical_page_count instead of requiring
	// level_count * pages_per_level^2 <= physical_page_count outright --
	// MaxLevels (8) and the [Multiple=8]/level_info[8] budgets in vsm.sig
	// already supported more than 3, this was the only thing actually
	// capping it. Coarsest level spans pages_per_level * base_page_world_size
	// * 2^(level_count-1) world units centered on the camera -- 6 levels / 4
	// pages / base=32 reaches ~4096u total (2048u radius), vs. 512u at 3
	// levels. Geometry past the coarsest level's edge clamps into an
	// unrelated page (shadows vanishing with distance -- see also the
	// out-of-range check in get_shadow_vsm, which returns unshadowed
	// instead of clamp-sampling).
	page_table.clipmap.level_count = 6;
	page_table.clipmap.pages_per_level = 4;
	page_table.clipmap.base_page_world_size = 32.0f;
	// page_size (texels/page) is the resolution lever. The atlas is an array
	// of page_size^2 slices, so the old 16-viewport cap on pages_per_level is
	// gone. physical_page_count no longer needs to cover every level's full
	// grid at once (level_count * pages_per_level^2 = 96 at 6 levels) --
	// residency culling (the `needed` test in m_level_render) keeps most of
	// each coarse level's ring outside the camera's frustum and never allocated.
	// Measured against a large/dense scene, 64 wasn't enough even with
	// culling active (a wide FOV or a lot of visible geometry can still
	// leave demand near or over supply) -- allocate_or_touch now fails
	// closed rather than evicting under pressure (see VSMPageTable), so
	// running out shows as coarser-level fallback, not corruption, but
	// still means less resolution than intended.
	//
	// 128 is a moderate bump, not a guess at the "real" number -- VRAM cost
	// is committed for every slot regardless of use (atlas + Hi-Z pyramid
	// together run ~2.6MB/slot at page_size=512, so 128 ~= 335MB). Push this
	// higher only after checking it's actually still hitting VSM_INVALID_SLOT
	// (a debug counter would be the honest way to know, not currently
	// wired up). A scene that genuinely needs far more resident pages than
	// that wants this backed by a reserved/tiled resource instead (commit
	// memory only for slices actually in use), not just a bigger committed
	// array -- see the class comment on VSM_Atlas's creation below.
	page_table.page_size = 512;
	page_table.physical_page_count = 128;

	const int pages_side       = page_table.clipmap.pages_per_level;
	const int pages_per_level  = pages_side * pages_side;

	// One Hi-Z pyramid slice per physical page, mip chain down to an 8x8
	// floor (1x1 wastes dispatches/VRAM for no real occluder benefit).
	const int physical_slots = page_table.physical_page_count;
	int pyramid_mip_count = 1;
	for (int s = page_table.page_size; s > 8; s >>= 1)
		pyramid_mip_count++;

	// ---- Page rendering (one pass per clipmap LEVEL, all of that level's
	// pages_per_level^2 pages rendered in a single mesh-shader dispatch via
	// SV_RenderTargetArrayIndex -- see mesh_shader_vsm.hlsl) ------------------

	for (int level = 0; level < page_table.clipmap.level_count; level++)
	{
		m_level_setup[level] = [this, level, physical_slots, pyramid_mip_count](Passes::VSM_RenderPage::Context& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			if (level == 0)
			{
				// Static: Phase 2 caching relies on this atlas (and the page
				// table / page cameras) surviving across frames -- a level
				// that isn't dirty this frame does nothing and relies on its
				// pages still holding last frame's content.
				//
				// One array slice per physical page rather than regions of one
				// big texture: slot IS the slice, so no atlas-offset math, and
				// routing via SV_RenderTargetArrayIndex drops the 16-viewport
				// cap that pinned pages_per_level at 4.
				//
				// This is a committed resource -- VRAM for all physical_slots
				// slices is paid up front regardless of how many are ever
				// used. Scaling physical_page_count much further than a
				// couple hundred should switch this (and VSM_PageHiZ below)
				// to a reserved/tiled resource, mapping physical memory only
				// to slices actually handed out by allocate_or_touch and
				// unmapping on evict_page -- the array can then be sized
				// generously (its logical slice count costs nothing by
				// itself) without paying for slices nobody's using.
				builder.create(data.VSM_Atlas, { ivec3(page_table.page_size, page_table.page_size, 0), HAL::Format::R32_TYPELESS, (UINT)physical_slots, 1 }, FrameGraph::ResourceFlags::DepthStencil | FrameGraph::ResourceFlags::Static);
				builder.create(data.VSM_PageTable, { ivec3(page_table.clipmap.pages_per_level, page_table.clipmap.pages_per_level, 0), HAL::Format::R32_UINT, (UINT)page_table.clipmap.level_count, 1 }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
				builder.create(data.VSM_PageCameras, { (size_t)MaxPages }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
				// Static like VSM_Atlas: must survive until this page is next dirty.
				builder.create(data.VSM_PageHiZ, { ivec3(page_table.page_size, page_table.page_size, 0), HAL::Format::R32_FLOAT, (UINT)physical_slots, (UINT)pyramid_mip_count }, FrameGraph::ResourceFlags::UnorderedAccess | FrameGraph::ResourceFlags::Static);
			}
			else
			{
				builder.need(data.VSM_Atlas, FrameGraph::ResourceFlags::DepthStencil);
				builder.need(data.VSM_PageTable, FrameGraph::ResourceFlags::CopyDest);
				builder.need(data.VSM_PageCameras, FrameGraph::ResourceFlags::CopyDest);
				builder.need(data.VSM_PageHiZ, FrameGraph::ResourceFlags::UnorderedAccess);
			}
			return true;
		};

		m_level_render[level] = [this, level, pages_side, pages_per_level, pyramid_mip_count](Passes::VSM_RenderPage::Context& data, FrameGraph::FrameContext& context)
		{
			// All the decision-making (which pages get a slot, priority
			// stealing, dirty tracking) already happened in plan_frame(),
			// single-threaded, before this -- and any other level's --
			// render() was dispatched. This is purely "emit GPU commands for
			// what was already decided."
			const LevelPlan& plan = m_plan[level];
			if (!plan.valid)
				return;

			// Nothing to redraw AND the indirection row still matches what's
			// on the GPU -- true no-op frame for this level.
			if (plan.dirty_mask == 0 && !plan.row_changed)
				return;

			auto& command_list = context.get_list();
			auto& graphics      = command_list->get_graphics();
			auto& compute       = command_list->get_compute();

			graphics.set_signature(Layouts::DefaultLayout);
			compute.set_signature(Layouts::DefaultLayout);

			// Camera bounds only depend on (level, abs_page) -- page_min ==
			// origin + local*size == size*(grid_idx + local) == size*abs_page
			// regardless of how that abs_page was reached -- but znear/zfar
			// track the live scene/camera bounds, so still refresh every
			// active frame rather than only on (re)allocation. Skipped for
			// non-resident cells: their slot is invalid, nothing to upload.
			for (int local = 0; local < pages_per_level; local++)
			{
				if (!((plan.resident_mask >> local) & 1))
					continue;

				ivec2 page = ivec2(local % pages_side, local / pages_side);
				float2 page_min = page_table.clipmap.page_min(level, page, plan.origin);
				float2 page_max = page_min + float2(page_table.clipmap.page_world_size(level));

				camera page_cam = make_light_view_camera(get_position());
				page_cam.set_projection_params(
					page_min.x, page_max.x,
					page_min.y, page_max.y,
					plan.bounds_all.znear - 10, plan.bounds_all.zfar);
				page_cam.update();

				command_list->get_copy().update(*data.VSM_PageCameras, plan.slots[local], std::span{ &page_cam.camera_cb.current, 1 });
			}

			// Indirection is indexed by (local position, level) -- rewrite
			// this level's row whenever local->slot changed for any cell
			// (plan.row_changed, computed in plan_frame()).
			if (plan.row_changed)
			{
				std::vector<uint32_t> row(pages_per_level);
				for (int local = 0; local < pages_per_level; local++)
					row[local] = (uint32_t)plan.slots[local];

				UINT row_stride = (UINT)(pages_side * sizeof(uint32_t));
				command_list->get_copy().update_texture(
					(*data.VSM_PageTable).resource, ivec3(0, 0, 0), ivec3(pages_side, pages_side, 1), (UINT)level,
					reinterpret_cast<const char*>(row.data()), row_stride);
			}

			if (level == 0 && data.VSM_PageHiZ.is_new())
			{
				// Fresh/resized pyramid holds garbage -- clear to far (0,
				// reversed-Z) so a page's occlusion test can't falsely cull
				// against it before that page has ever rendered real depth.
				// One array-spanning UAV clear per mip (all physical slots
				// at once) -- only runs once ever (cold start / resize).
				for (int mip = 0; mip < pyramid_mip_count; mip++)
					command_list->clear_uav(data.VSM_PageHiZ->create_mip(mip, *command_list).rwTexture2DArray, vec4(0, 0, 0, 0));
			}

			if (plan.dirty_mask == 0)
				return;

			// Clear only the pages actually being re-rendered this frame: the
			// atlas is Static and shared across levels, so a cached page must
			// keep its depth. One slice DSV per dirty page (the create_2d_slice
			// + ClearDepth idiom PSSM.cpp uses), replacing the old multi-rect
			// clear into a packed atlas.
			for (int local = 0; local < pages_per_level; local++)
			{
				if (!((plan.dirty_mask >> local) & 1))
					continue;

				RT::DepthOnly slice_rt;
				slice_rt.GetDepth() = data.VSM_Atlas->create_2d_slice(plan.slots[local], *command_list).depthStencil;
				graphics.set_rtv(slice_rt, RTOptions::Default | RTOptions::ClearDepth);
			}

			{
				// Draw target: a DSV spanning every slice, so the mesh shader
				// can route each primitive to its page with
				// SV_RenderTargetArrayIndex (needs the multi-slice DSV support
				// added in HAL.HLSL.ixx).
				RT::DepthOnly rt;
				rt.GetDepth() = data.VSM_Atlas->depthStencil;
				graphics.set_rtv(rt, RTOptions::Default);
			}

			// One viewport now -- each slice is its own full page_size target,
			// so there is no per-page offset to encode.
			graphics.set_viewport(data.VSM_Atlas->get_viewport());
			graphics.set_scissor(data.VSM_Atlas->get_scissor());
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
			graphics.set_pipeline<PSOS::VSMDepthDraw>();

			{
				Slots::VSMPageTableData pageTableData;
				pageTableData.GetPage_table()   = data.VSM_PageTable->texture2DArray;
				pageTableData.GetPage_cameras() = data.VSM_PageCameras->structuredBuffer;
				graphics.set(pageTableData);
			}
			{
				Slots::VSMPageBatch batch;
				batch.GetLevel()           = level;
				batch.GetDirty_mask()      = (int)plan.dirty_mask;
				// Per-page now, not per-level -- see the field's comment in
				// vsm.sig. A page whose slot/content survived the recenter
				// keeps its Hi-Z history and doesn't need this bit.
				batch.GetSkip_occlusion()  = (int)plan.skip_occlusion_mask;
				graphics.set(batch);
			}

			// No manual transition needed: like VSM_PageTable, graphics.set()
			// tracks the read state for this SRV bind itself.
			{
				Slots::VSMPageHiZ pageHiZ;
				pageHiZ.GetPage_hiz() = data.VSM_PageHiZ->texture2DArray;
				graphics.set(pageHiZ);
			}

			auto& sceneinfo = context.graph->get_context<SceneInfo>();
			auto  scene     = sceneinfo.scene;

			graphics.set(scene->compiledScene);

			scene->iterate_meshes(MESH_TYPE::STATIC | MESH_TYPE::DYNAMIC, [&](scene_object::ptr obj)
			{
				auto mesh = dynamic_cast<MeshAssetInstance*>(obj.get());
				if (!mesh) return;

				for (auto& m : mesh->rendering)
				{
					graphics.set(m.compiled_mesh_info);
					graphics.set(m.mesh_instance_info);
					// Dispatch AS threadgroups, not MS ones directly: 32
					// (meshlet, page) pairs tested per group, AS compacts
					// visible pairs into a payload and calls DispatchMesh
					// itself (see mesh_shader_vsm.hlsl). Count must cover
					// meshlet_count * pages_per_level total pairs.
					UINT pair_count = (UINT)m.meshlet_count * (UINT)pages_per_level;
					UINT as_groups  = (pair_count + 31) / 32;
					graphics.dispatch_mesh(ivec3{ (int)as_groups, 1, 1 });
				}
			});

			// Rebuild each just-redrawn page's pyramid for next time it's dirty.
			// VSM_Atlas was just a DSV -- nudge readable for the copy shader
			// below, same one-off idiom PSSM.cpp uses; the next draw into it
			// transitions it back automatically via set_rtv.
			command_list->transition(data.VSM_Atlas->resource, HAL::ResourceStates::SHADER_RESOURCE);

			for (int local = 0; local < pages_per_level; local++)
			{
				if (!((plan.dirty_mask >> local) & 1))
					continue;

				int slot = plan.slots[local];
				auto slice = data.VSM_PageHiZ->create_2d_slice(slot, *command_list);

				compute.set_pipeline<PSOS::VSMCopyPageDepth>();
				{
					// Both sides are per-page slices of the same size now, so
					// this is a straight 1:1 slice copy -- no atlas offset.
					Slots::VSMCopyPageDepth copy;
					copy.GetAtlas()    = data.VSM_Atlas->create_2d_slice(slot, *command_list).texture2D;
					copy.GetDst_mip0() = slice.create_mip(0, *command_list).rwTexture2D;
					compute.set(copy);
				}
				compute.dispatch(ivec2(page_table.page_size, page_table.page_size), ivec2(8, 8));

				MipMapGenerator::get().build_hiz_pyramid(compute, slice);

				// Every mip but the last is later read as another mip's
				// SrcMip, so it naturally rests as SHADER_RESOURCE; only the
				// coarsest mip's write is never followed by a read.
				UINT last_mip_subres = (UINT)(pyramid_mip_count - 1) + (UINT)slot * (UINT)pyramid_mip_count;
				command_list->transition(data.VSM_PageHiZ->resource, HAL::ResourceStates::SHADER_RESOURCE, last_mip_subres);
			}
		};
	}

	// ---- Combine lighting ------------------------------------------------

	m_combine_setup = [this](Passes::VSM_Combine::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		GBufferViewDesc::need(builder, data.gbuffer);
		builder.need(data.ResultTexture, FrameGraph::ResourceFlags::UnorderedAccess);
		builder.need(data.VSM_Atlas, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.VSM_PageTable, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.VSM_PageCameras, FrameGraph::ResourceFlags::ComputeRead);
		return true;
	};

	m_combine_render = [this](Passes::VSM_Combine::Context& data, FrameGraph::FrameContext& context)
	{
		GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);

		auto& list    = *context.get_list();
		auto& compute = list.get_compute();

		auto& caminfo = context.graph->get_context<CameraInfo>();
		auto  cam     = caminfo.cam;

		context.graph->set_slot(SlotID::FrameInfo, compute);

		camera light_cam = make_light_view_camera(get_position());
		float2 cam_pos_ls = (float4(cam->position, 1) * light_cam.get_view()).xy;

		{
			Slots::VSMLighting lighting;
			gbuffer.SetTable(lighting.GetGbuffer());
			lighting.GetVsm_atlas()    = data.VSM_Atlas->texture2DArray;
			lighting.GetPage_table()   = data.VSM_PageTable->texture2DArray;
			lighting.GetPage_cameras() = data.VSM_PageCameras->structuredBuffer;
			lighting.GetResult()       = data.ResultTexture->rwTexture2D;
			compute.set(lighting);
		}

		{
			Slots::VSMConstants constants;
			constants.GetLevel_count()          = page_table.clipmap.level_count;
			constants.GetPage_size()            = page_table.page_size;
			constants.GetPages_per_level()      = page_table.clipmap.pages_per_level;
			constants.GetLight_view()           = light_cam.get_view();

			for (int level = 0; level < page_table.clipmap.level_count; level++)
			{
				float2 origin = page_table.clipmap.grid_origin(level, cam_pos_ls);
				constants.GetLevel_info()[level] = float4(origin.x, origin.y, page_table.clipmap.page_world_size(level), 0);
			}

			compute.set(constants);
		}

		compute.set_pipeline<PSOS::VSMApplyCompute>();
		compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 16, 16 });
	};
}
