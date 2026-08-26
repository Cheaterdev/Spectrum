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

	// Phase 5.15: flat per-frame collections m_renderpages_render's process
	// phase fills before executing each GPU-command type in its own pass
	// (see there). File-scope, not local-to-the-lambda: a local class type
	// used as a std::vector element passed through a cross-module template
	// (CopyContext::update<T>, defined in HAL) hit real deduction failures
	// under this toolset's C++20 modules implementation -- file scope sidesteps
	// it and matches this file's own existing convention for shared helpers.
	struct PageCameraUpdate { int slot; camera cam; };
	struct RowUpdate { int level; std::vector<uint32_t> row; };
}

void VSM::attach_scene(std::shared_ptr<Scene> scene)
{
	// Per-page invalidation (Phase 2): map a touched object's world AABB to
	// the (level, page) bits it overlaps, via the same light-view-space
	// projection/clamp math VSMClipmap already uses for page selection --
	// world_to_page's edge-clamp means an object slightly outside a coarse
	// level's grid still marks that level's nearest edge page (conservative,
	// not a correctness bug).
	tracker.attach(scene,
	[this](scene_object* object)
	{
		// on_element_add: object->get_min()/get_max() aren't reliably valid
		// yet here. on_add() (SceneObject.cpp) fires this event immediately
		// on insertion, before update_transforms() has necessarily run for
		// the new object -- occluder::min/max are cached fields only ever
		// set by apply_transform(), which update_transforms() only calls
		// when its own change-detection fires. Worse,
		// MeshAssetInstance::update_transforms() gates its entire bounds
		// computation (and the on_moved firing that would otherwise
		// self-correct this) behind `res || need_update_mats`, and
		// need_update_mats starts false -- so a freshly added object whose
		// very first computed transform happens not to trip that check
		// never gets a correcting on_moved at all. Rather than trust a
		// possibly-default (0,0,0) AABB, just mark every currently-active
		// level fully dirty -- object adds are rare, not a per-frame cost.
		for (int level = 0; level < VSM::MaxLevels; level++)
			if (level_initialized[level])
				tracker.mark_level_full(level);
	},
	[this](scene_object* object)
	{
		vec3 wmin = object->get_min();
		vec3 wmax = object->get_max();

		auto mark_bounds = [this](vec3 mn, vec3 mx)
		{
			camera light_view_cam = make_light_view_camera(get_position());
			box bounds_ls = light_view_cam.get_points(mn, mx).get_bounds_in(light_view_cam.get_view());
			float2 ls_min(bounds_ls.left, bounds_ls.top);
			float2 ls_max(bounds_ls.right, bounds_ls.bottom);

			for (int level = 0; level < VSM::MaxLevels; level++)
			{
				// A level that hasn't rendered yet has no meaningful
				// cached_origin -- it'll get a full-mask render (recentered)
				// the first time it does run, so there's nothing useful to
				// mark here yet. Also true of any level currently outside
				// [active_min, active_max] -- plan_frame() never planned it.
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
		};

		mark_bounds(wmin, wmax);

		// A fast-moving object can clear a page's footprint entirely between
		// two on_moved events -- only marking its CURRENT bounds then leaves
		// the page it just LEFT never invalidated, so that page's cached
		// content keeps showing the object's shadow at its old position
		// forever (nothing else will ever redraw that page). Track each
		// object's last-marked world bounds and also mark those on a change,
		// so the vacated page gets a redraw too.
		std::lock_guard<std::mutex> g(bounds_mutex);
		auto it = last_marked_bounds.find(object);
		if (it != last_marked_bounds.end() && (it->second.first != wmin || it->second.second != wmax))
			mark_bounds(it->second.first, it->second.second);
		last_marked_bounds[object] = { wmin, wmax };
	});
}

void VSM::build_atlas_views()
{
	std::call_once(atlas_views_once, [this]
	{
		auto& storage = RenderSystem::get().device().get_static_gpu_data();
		int physical_slots = page_table.physical_page_count;

		atlas_slot_views.resize(physical_slots);
		for (int slot = 0; slot < physical_slots; slot++)
			atlas_slot_views[slot] = HAL::Texture2DView(vsm_atlas_tex->resource, storage,
				HAL::TextureViewDesc{ 0, 1, (uint)slot, 1 });

		atlas_array_view = HAL::Texture2DView(vsm_atlas_tex->resource, storage,
			HAL::TextureViewDesc{ 0, 1, 0, (uint)physical_slots });
	});
}

void VSM::build_page_hiz_views(Passes::VSM_HiZRebuild::Context& data, int pyramid_mip_count)
{
	std::call_once(page_hiz_views_once, [this, &data, pyramid_mip_count]
	{
		auto& storage = RenderSystem::get().device().get_static_gpu_data();

		page_hiz_mip_array_views.resize(pyramid_mip_count);
		for (int mip = 0; mip < pyramid_mip_count; mip++)
			page_hiz_mip_array_views[mip] = data.VSM_PageHiZ->create_mip(mip, storage);
	});
}

void VSM::build_level_hiz_views(Passes::VSM_HiZRebuild::Context& data, int level_pyramid_mip_count)
{
	std::call_once(level_hiz_views_once, [this, &data, level_pyramid_mip_count]
	{
		auto& storage = RenderSystem::get().device().get_static_gpu_data();

		level_hiz_mip_array_views.resize(level_pyramid_mip_count);
		for (int mip = 0; mip < level_pyramid_mip_count; mip++)
			level_hiz_mip_array_views[mip] = data.VSM_LevelHiZ->create_mip(mip, storage);
	});
}

void VSM::pass_data(FrameGraph::TaskBuilder& builder)
{
	if (!vsm_atlas_tex)
		vsm_atlas_tex.reset(new HAL::Texture(RenderSystem::get().device(), HAL::ResourceDesc::Tex2D(HAL::Format::R32_TYPELESS, uint2(page_table.page_size, page_table.page_size), (uint)MaxPhysicalSlots, 1, HAL::ResFlags::DepthStencil | HAL::ResFlags::ShaderResource | HAL::ResFlags::Virtual)));
	builder.pass_texture(FrameGraph::ResourceID::VSM_Atlas, vsm_atlas_tex->resource);
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

	// While a non-Final debug view is selected, UI_Render points its own
	// need() at a different GBuffer/debug resource instead of ResultTexture
	// (GUI/Base.cpp's debug_source) -- nothing consumes ResultTexture that
	// frame, so FrameGraph's dependency-based culling drops VSM_Combine and,
	// transitively, VSM_RenderPages/VSM_HiZRebuild (none of them
	// [Required]). This function isn't part of that cullable pass graph --
	// it's a plain add_slot_generator callback -- so without this check it
	// would keep planning (evicting, priority-stealing, dirty-tracking)
	// every frame regardless, as if pages were still being rendered, while
	// nothing ever actually redraws them. Switching back to Final then
	// resumes rendering against a page table that drifted out from under
	// the GPU's real content (confirmed live: pages visibly evicted/wrong
	// on returning to Final after time spent in another debug view).
	// Freezing here instead -- the page table simply holds whatever it held
	// the last time this actually ran, and picks back up cleanly.
	if (graph.get_context<FrameGraph::DebugContext>().mode != FrameGraph::DebugMode::Final)
		return;

	// Re-enabling after a period with the pyramid un-rebuilt: any page
	// redrawn while disabled has stale Hi-Z content that no longer matches
	// its current atlas depth. Force the same full-level invalidation a
	// light move already causes, so every resident page gets a real
	// rebuild before anything trusts its pyramid again.
	if (hiz_culling_enabled && !hiz_culling_enabled_prev)
		light_change_pending.fill(true);
	hiz_culling_enabled_prev = hiz_culling_enabled;

	float3 current_light_pos = get_position();
	frame_light_pos = current_light_pos;
	camera light_view_cam = make_light_view_camera(current_light_pos);
	float2 cam_pos_ls = (float4(cam->position, 1) * light_view_cam.get_view()).xy;

	auto min = scene->get_min();
	auto max = scene->get_max();
	auto points_all = cam->get_points(min, max);
	auto bounds_all = points_all.get_bounds_in(light_view_cam.get_view());

	// Z range overridden with fixed constants (see VSM_LIGHT_Z_NEAR/FAR's
	// own comment) -- NOT derived from the scene at all, so it can't drift
	// frame to frame or need changing if a bigger scene loads later. XY
	// (left/right/top/bottom) stays the real per-frame value from above --
	// that's about which pages are currently needed, not about Z
	// comparability, and SHOULD track the live scene.
	bounds_all.znear = VSM_LIGHT_Z_NEAR;
	bounds_all.zfar  = VSM_LIGHT_Z_FAR;

	uint64_t tick = ++m_frame_id;

	const int pages_side      = page_table.clipmap.pages_per_level;
	const int pages_per_level = pages_side * pages_side;
	uint32_t  all_pages_mask  = (1u << pages_per_level) - 1;

	// Phase 5.7: compute this frame's active window first, then plan exactly
	// the levels inside it -- one uniform loop, no more separate adaptive/
	// regular passes. Finest-active-first: acquire_slot_with_priority only
	// ever steals from a level STRICTLY coarser than the requester, so
	// processing in this order means a fine level always gets first pick of
	// whatever's free this frame, and can reclaim an already-resident
	// coarser page if it has to -- never the other way around.
	update_active_window(cam->z_far);
	for (int level = active_min; level <= active_max; level++)
		plan_level(level, cam_pos_ls, bounds_all, tick, pages_side, pages_per_level, all_pages_mask);
	for (int level = 0; level < VSM::MaxLevels; level++)
		if (level < active_min || level > active_max)
			m_plan[level].valid = false;
}

void VSM::update_active_window(float z_far)
{
	float measured = std::bit_cast<float>(measured_texel_size_bits.load(std::memory_order_relaxed));

	// active_max: correctness floor, driven purely by z_far (CPU-only,
	// zero-latency, can never under-cover based on what happened to be
	// visible in the depth-analysis sample region this/last frame).
	// radius(level) = 2 * base_page_world_size * 2^(level - level_zero_slot)
	// -- smallest level whose radius covers z_far.
	float base = page_table.clipmap.base_page_world_size;
	int   zero = page_table.clipmap.level_zero_slot;
	int required_max = zero + (int)std::ceil(std::log2(std::max(z_far, 1.0f) / (2.0f * base)));
	required_max = std::clamp(required_max, zero, VSM::MaxLevels - 1);

	// active_min: quality knob, driven by the depth-analysis MIN-texel
	// signal (same readback Phase 5.6 already built) -- finest level whose
	// texel density is still <= measured. No valid measurement (FLT_MAX)
	// clamps naturally to level_zero_slot: no gratuitous fine levels
	// without evidence there's anything nearby to resolve.
	int required_min = zero;
	if (measured < std::numeric_limits<float>::max() * 0.5f && measured > 0.0f)
		required_min = zero + (int)std::floor(std::log2(measured * (float)page_table.page_size / base));
	required_min = std::clamp(required_min, 0, zero);

	// Instant grow, debounced shrink -- both bounds. "Grow" for active_max
	// means numerically larger (more coverage); "grow" for active_min means
	// numerically smaller (more/finer detail).
	if (required_max > active_max) { active_max = required_max; max_shrink_run = 0; }
	else if (required_max < active_max) { if (++max_shrink_run >= ShrinkFrames) { active_max = required_max; max_shrink_run = 0; } }
	else max_shrink_run = 0;

	if (required_min < active_min) { active_min = required_min; min_shrink_run = 0; }
	else if (required_min > active_min) { if (++min_shrink_run >= ShrinkFrames) { active_min = required_min; min_shrink_run = 0; } }
	else min_shrink_run = 0;

	// TEMP diagnostics (Phase 5.7 tuning): confirm the computed window is
	// tracking z_far/measured sensibly, throttled to avoid spam. Remove
	// once confirmed working.
	{
		static uint64_t s_diag_frame = 0;
		if ((s_diag_frame++ % 120) == 0)
		{
			Log::get() << (std::string("VSM active window diag: measured=") + std::to_string(measured)
				+ " z_far=" + std::to_string(z_far)
				+ " active_min=" + std::to_string(active_min)
				+ " active_max=" + std::to_string(active_max)
				+ " required_min=" + std::to_string(required_min)
				+ " required_max=" + std::to_string(required_max)) << Log::endl;
		}
	}
}

void VSM::plan_level(int level, float2 cam_pos_ls, const box& bounds_all, uint64_t tick,
                      int pages_side, int pages_per_level, uint32_t all_pages_mask)
{
	LevelPlan plan;
	plan.origin     = page_table.clipmap.grid_origin(level, cam_pos_ls);
	plan.bounds_all = bounds_all;
	plan.valid      = true;

	// Recentered = this level's local page indices now mean a different
	// world region than they did last frame -- the indirection ROW must
	// be rewritten either way (it's indexed by local position), even on
	// frames where every page it now points to happens to already be
	// cached. Also true, correctly, the first frame an adaptive tier
	// activates (level_initialized[level] is still false then).
	bool recentered = !level_initialized[level] || plan.origin.x != cached_origin[level].x || plan.origin.y != cached_origin[level].y;

	// Read-and-clear this level's sticky flag (see VSM.ixx).
	pos_mutex.lock();
	bool light_moved = light_change_pending[level];
	light_change_pending[level] = false;
	pos_mutex.unlock();

	// Combine this frame's freshly-marked dirty pages with whatever's still
	// pending from an earlier frame because it wasn't resident yet (see the
	// pending_scene_mask member comment in VSM.ixx).
	uint32_t scene_mask = (tracker.take_dirty(level) & all_pages_mask) | pending_scene_mask[level];

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

	// A bit for a page that isn't resident THIS frame (not needed yet, or
	// needed but the pool was exhausted) would otherwise be lost forever --
	// nothing else ever re-marks it, since newly_allocated_mask only fires
	// the frame an allocation actually succeeds, not retroactively for a
	// scene-change that happened while the page was unavailable. Keep it in
	// pending_scene_mask instead of routing it back through
	// VSMInvalidationTracker (see that member's comment in VSM.ixx).
	pending_scene_mask[level] = scene_mask & ~plan.resident_mask;

	// newly_allocated/light_moved invalidate that specific page (not the
	// whole level -- a page whose slot/content survived the recenter
	// needs neither); scene_mask adds whatever VSMInvalidationTracker
	// actually marked. Masked to resident_mask: a page with no valid
	// slot this frame -- not needed, or needed but the pool was
	// exhausted -- is never drawn into a slot it doesn't have.
	plan.dirty_mask          = (newly_allocated_mask | scene_mask | (light_moved ? all_pages_mask : 0u)) & plan.resident_mask;
	// A newly-allocated slot has no Hi-Z history; light_moved makes
	// every resident page's existing pyramid stale too (built under the
	// old light projection). hiz_culling_enabled==false forces every
	// resident page into this state -- the pyramid is never rebuilt for
	// any of them (see m_renderpages_render), so none ever have valid
	// history to test against.
	plan.skip_occlusion_mask = (newly_allocated_mask | (light_moved || !hiz_culling_enabled ? all_pages_mask : 0u)) & plan.resident_mask;

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

VSM::VSM() : VariableContext(L"VSM")
{
	position = float3(200, 400, 200);

	// Phase 5.7: level_count is now just the fixed storage budget (MaxLevels,
	// 26 slots) -- which of those slots are actually planned/rendered each
	// frame is the dynamically-computed [active_min, active_max] window
	// (see update_active_window()), not a compile-time split. level_zero_slot
	// (12) is the fixed index whose size equals base_page_world_size=32;
	// slots below are progressively finer, above progressively coarser (see
	// VSMClipmap::page_world_size) -- geometry outside every active level's
	// grid clamps into an unrelated page (shadows vanishing with distance --
	// see also the out-of-range check in get_shadow_vsm, which returns
	// unshadowed instead of clamp-sampling). active_min/active_max both
	// start at level_zero_slot (VSM.ixx's in-class initializers), matching
	// today's default behavior before any real measurement/z_far arrives.
	page_table.clipmap.level_count = VSM::MaxLevels;
	page_table.clipmap.level_zero_slot = VSM::LevelZeroSlot;
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
	// 256 (raised from 128 -- Phase 5.7): the unified active window can
	// legitimately span more simultaneously-active levels than the old
	// fixed-6-regular-plus-occasional-tier system did -- active_min can sit
	// several levels below level_zero_slot AND active_max still has to
	// reach z_far's coverage floor at the same time, so worst case is
	// (active_max - active_min + 1) levels all wanting up to 16 pages each
	// at once, confirmed via the "VSM: page pool exhausted" log actually
	// firing at the coarse end (levels active_max-3..active_max) under a
	// normal test scene at the old 128 budget. VRAM cost is committed for
	// every slot regardless of use (atlas + Hi-Z pyramid together run
	// ~2.6MB/slot at page_size=512, so 256 ~= 670MB). Push this higher only
	// after checking it's actually still hitting VSM_INVALID_SLOT (a debug
	// counter would be the honest way to know, not currently wired up). A
	// scene that genuinely needs far more resident pages than that wants
	// this backed by a reserved/tiled resource instead (commit memory only
	// for slices actually in use), not just a bigger committed array -- see
	// the class comment on VSM_Atlas's creation below.
	page_table.page_size = 512;
	page_table.physical_page_count = 256;

	const int pages_side       = page_table.clipmap.pages_per_level;
	const int pages_per_level  = pages_side * pages_side;

	// One Hi-Z pyramid slice per physical page, mip chain down to 1x1.
	// Used to floor at 8x8 (1x1 "wastes dispatches/VRAM for no real
	// occluder benefit") back when this only served occlusion culling.
	// Phase 5.18 Part A's blocker-search classification (VSM_impl_search.
	// hlsl's vsm_search_blocker) needs deeper mips too: its rect_in_page
	// guard already caps the search radius it will ever classify at half
	// the page width (a wider disc always fails that check, at any mip
	// depth), which works out to needing mip index up to log2(page_size)
	// -- i.e. a chain reaching all the way to 1x1 for a 512 page. Stopping
	// at 8x8 left that last stretch (mips 7/8/9) permanently unavailable,
	// which showed up live as classification silently falling back to a
	// mip finer than the coverage guarantee needs -- a missed blocker, a
	// shadow that should exist vanishing. The extra 3 mips are tiny
	// (4x4+2x2+1x1 per slot) so the old VRAM/dispatch-count worry doesn't
	// really apply at this depth.
	const int physical_slots = page_table.physical_page_count;
	int pyramid_mip_count = 1;
	for (int s = page_table.page_size; s > 1; s >>= 1)
		pyramid_mip_count++;

	// Phase 5.18 follow-up: VSMPageHiZ's level_hiz field, one array slice
	// per LEVEL (MaxLevels), mip chain down to 1x1 -- same "down to 1x1"
	// reasoning as pyramid_mip_count above, just one step smaller since
	// mip 0 here is already pages_per_level^2 (a whole page's summary per
	// texel) instead of page_size^2 raw depth texels.
	int level_pyramid_mip_count = 1;
	for (int s = pages_side; s > 1; s >>= 1)
		level_pyramid_mip_count++;

	// ---- Page rendering (Phase 5.8: ONE pass, one exec_indirect() call
	// covering every active+dirty level's every mesh, instead of one
	// Multiple-slot pass per level) ------------------------------------------

	m_gatherdispatch_setup = [this](Passes::VSM_GatherDispatch::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		// CPU-built and re-uploaded fresh every frame (like VSM_PageCameras),
		// but the underlying allocation persists -- Static, sized to the
		// fixed level-count budget (small, not mesh-count-dependent).
		builder.create(data.VSM_LevelDispatchInfo, { (size_t)MaxLevels }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
		// GPU-appended now (Phase 5.12), not CPU-uploaded -- UnorderedAccess,
		// not CopyDest. Sized to the same generous fixed upper bound as
		// before (worst case is still bounded by total mesh parts x active
		// dirty level count, same shape the old CPU loop's own worst case
		// was).
		// counted=true: this buffer is GPU-appended (AppendStructuredBuffer in
		// the gather shader) and needs a real counter -- clear_counter() and
		// exec_indirect()'s GPU-computed count both read it.
		builder.create(data.VSM_DispatchCommands, { (size_t)MaxDispatchEntries, true }, FrameGraph::ResourceFlags::UnorderedAccess | FrameGraph::ResourceFlags::Static);
		return true;
	};

	m_renderpages_setup = [this, physical_slots, pyramid_mip_count, pages_side, level_pyramid_mip_count](Passes::VSM_RenderPages::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		builder.need(data.VSM_Atlas, FrameGraph::ResourceFlags::DepthStencil);
		builder.create(data.VSM_PageTable, { ivec3(page_table.clipmap.pages_per_level, page_table.clipmap.pages_per_level, 0), HAL::Format::R32_UINT, (UINT)page_table.clipmap.level_count, 1 }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
		builder.create(data.VSM_PageCameras, { (size_t)MaxPages }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
		// Static like VSM_Atlas: must survive until this page is next dirty.
		// R32G32_FLOAT, not R32_FLOAT: Phase 5.18 Part A widened the pyramid
		// to two channels (.x = min/farthest, .y = max/closest -- see
		// VSMPageHiZ's own comment in vsm.sig).
		builder.create(data.VSM_PageHiZ, { ivec3(page_table.page_size, page_table.page_size, 0), HAL::Format::R32G32_FLOAT, (UINT)physical_slots, (UINT)pyramid_mip_count }, FrameGraph::ResourceFlags::UnorderedAccess | FrameGraph::ResourceFlags::Static);
		// Phase 5.18 follow-up: one array slice per LEVEL (not per physical
		// slot), mip0 = pages_side x pages_side (one texel per page). Same
		// Static/UnorderedAccess shape as VSM_PageHiZ.
		builder.create(data.VSM_LevelHiZ, { ivec3(pages_side, pages_side, 0), HAL::Format::R32G32_FLOAT, (UINT)MaxLevels, (UINT)level_pyramid_mip_count }, FrameGraph::ResourceFlags::UnorderedAccess | FrameGraph::ResourceFlags::Static);
		// Now GPU-appended by VSM_GatherDispatch -- this pass only reads it
		// via exec_indirect.
		builder.need(data.VSM_DispatchCommands, FrameGraph::ResourceFlags::ComputeRead);
		return true;
	};

	// Phase 5.17: the async-compute Hi-Z rebuild pass. need()s VSM_Atlas
	// (read, establishes "runs after VSM_RenderPages' draw") and
	// VSM_PageHiZ (write, for the per-frame rebuild -- VSM_RenderPages
	// still owns create() for the cold-start clear, see its own setup()).
	// VSM_DirtySlots moves here entirely since only this pass's dispatches
	// consume it now.
	m_hizrebuild_setup = [this, physical_slots, pages_per_level](Passes::VSM_HiZRebuild::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		builder.need(data.VSM_Atlas, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.VSM_PageHiZ, FrameGraph::ResourceFlags::UnorderedAccess);
		// Phase 5.14: CPU-built and re-uploaded fresh every frame (like
		// VSM_PageCameras), sized to the whole physical slot budget -- every
		// dirty page occupies a distinct slot, so that's a hard upper bound
		// on how many entries this can ever need in one frame.
		builder.create(data.VSM_DirtySlots, { (size_t)physical_slots }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
		// Phase 5.18 follow-up: feeds/downsamples VSM_LevelHiZ right after
		// the per-page rebuild above. VSM_DirtyPages carries logical
		// level/page_x/page_y alongside physical slot -- see
		// VSMDirtyPageInfo's own comment. Sized to EVERY local page of
		// EVERY level (MaxLevels x pages_per_level), not physical_slots --
		// unlike VSM_DirtySlots, this list is built unconditionally every
		// frame (every local page of every active level, mapped or not),
		// not just for dirty/resident pages, so its bound is the full
		// logical grid, which can exceed the physical slot budget.
		builder.need(data.VSM_LevelHiZ, FrameGraph::ResourceFlags::UnorderedAccess);
		builder.create(data.VSM_DirtyPages, { (size_t)(VSM::MaxLevels * pages_per_level) }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
		return true;
	};

	m_gatherdispatch_render = [this, pages_side, pages_per_level](Passes::VSM_GatherDispatch::Context& data, FrameGraph::FrameContext& context)
	{
		// GPU-driven replacement (Phase 5.12) for the old CPU
		// scene->iterate_meshes() walk -- builds one small per-level info
		// entry per active+dirty level (bounded by level count, same as the
		// VSMPageBatch CBs already compiled per level), uploads it, and
		// dispatches a compute shader that tests every scene mesh's AABB
		// against every entry and Append()s a VSMDispatchCommandData for
		// each overlap. plan_frame() already decided which levels are dirty
		// (m_plan[]) -- this only decides which meshes matter for them.
		auto& command_list = context.get_list();
		auto& compute       = command_list->get_compute();
		compute.set_signature(Layouts::DefaultLayout);

		auto& sceneinfo = context.graph->get_context<SceneInfo>();
		auto  scene     = sceneinfo.scene;

		// Always reset, even if nothing ends up appended this frame --
		// otherwise a stale count from a previous frame would make
		// VSM_RenderPages' exec_indirect replay old entries.
		compute.clear_counter(*data.VSM_DispatchCommands);

		if (!scene)
			return;

		m_level_dispatch_info.clear();
		for (int level = 0; level < VSM::MaxLevels; level++)
		{
			const LevelPlan& plan = m_plan[level];
			if (!plan.valid || plan.dirty_mask == 0)
				continue;

			// One per-level page-batch CB (level/dirty_mask/skip_occlusion),
			// compiled here (not in VSM_RenderPages) since the gather
			// shader needs a valid pointer to embed in every entry it
			// appends for this level -- reused as-is by VSM_RenderPages'
			// exec_indirect later this same frame (a compiled CB is just an
			// index into per-frame dynamic-constant storage, valid from
			// either context).
			Slots::VSMPageBatch batch;
			batch.GetLevel()          = level;
			batch.GetDirty_mask()     = (int)plan.dirty_mask;
			batch.GetSkip_occlusion() = (int)plan.skip_occlusion_mask;
			auto compiled_batch = batch.compile(compute);

			// This level's overall grid extent in light-space (not per-page
			// -- the gather shader only decides "does this mesh matter for
			// this LEVEL at all"; the AS still does per-page/per-meshlet
			// culling exactly as before).
			float2 level_min = page_table.clipmap.page_min(level, ivec2(0, 0), plan.origin);
			float2 level_max = level_min + float2(page_table.clipmap.page_world_size(level) * pages_side);

			Table::VSMLevelDispatchInfo info;
			info.page_batch_cb = compiled_batch.compiled();
			info.bounds_min = level_min;
			info.bounds_max = level_max;
			m_level_dispatch_info.push_back(info);
		}

		if (m_level_dispatch_info.empty())
			return;

		command_list->get_copy().update(*data.VSM_LevelDispatchInfo, 0, std::span{ m_level_dispatch_info });

		UINT mesh_count = (UINT)scene->command_ids[(int)MESH_TYPE::ALL].size();
		if (mesh_count == 0)
			return;

		{
			Slots::VSMGatherDispatchData gatherData;
			gatherData.GetLevels()            = data.VSM_LevelDispatchInfo->structuredBuffer;
			gatherData.GetLevel_count()       = (uint)m_level_dispatch_info.size();
			gatherData.GetDispatch_commands() = data.VSM_DispatchCommands->appendStructuredBuffer;

			camera light_view_cam = make_light_view_camera(frame_light_pos);
			gatherData.GetLight_view() = light_view_cam.get_view();

			compute.set(gatherData);
		}

		compute.set(scene->compiledGather[(int)MESH_TYPE::ALL]);
		compute.set(scene->compiledScene);

		compute.set_pipeline<PSOS::VSMGatherDispatch>();
		compute.dispatch(ivec2((int)mesh_count, (int)m_level_dispatch_info.size()), ivec2(64, 1));
	};

	m_renderpages_render = [this, pages_side, pages_per_level, pyramid_mip_count, level_pyramid_mip_count](Passes::VSM_RenderPages::Context& data, FrameGraph::FrameContext& context)
	{
		// All the decision-making (which pages get a slot, priority
		// stealing, dirty tracking) already happened in plan_frame(),
		// single-threaded, before this ran. This is purely "emit GPU
		// commands for what was already decided" -- looping every level's
		// m_plan[] instead of being called once per level.
		auto& command_list = context.get_list();
		auto& graphics      = command_list->get_graphics();
		auto& compute       = command_list->get_compute();

		graphics.set_signature(Layouts::DefaultLayout);
		compute.set_signature(Layouts::DefaultLayout);

		build_atlas_views();

		{
			PROFILE(L"vsm_tile_mapping");
			std::vector<int> to_map, to_unmap;
			page_table.take_pending_tile_changes(to_map, to_unmap);
			if (!to_map.empty() || !to_unmap.empty())
			{
				auto& atlas_tiled = vsm_atlas_tex->resource->get_tiled_manager();
				uint3 tile_dims = atlas_tiled.get_tiles_count();
				uint3 to = uint3(tile_dims.x - 1, tile_dims.y - 1, 0);

				// Phase 5.10: one batched UpdateTileMappings call for however
				// many slots need it this frame, not one call per slot -- see
				// load_tiles_batch()/zero_tiles_batch()'s own comments.
				if (!to_map.empty())
					atlas_tiled.load_tiles_batch(command_list.get(), uint3(0, 0, 0), to, to_map);
				if (!to_unmap.empty())
					atlas_tiled.zero_tiles_batch(command_list.get(), uint3(0, 0, 0), to, to_unmap);
			}
		}

		auto& sceneinfo = context.graph->get_context<SceneInfo>();
		auto  scene     = sceneinfo.scene;

		bool any_dirty = false;

		// Phase 5.15: process every level FIRST, without touching the
		// command list at all, then execute each GPU-command TYPE in its
		// own contiguous pass -- every page-camera CB write, then every
		// indirection-row write, then the (at most once-ever) Hi-Z
		// cold-clear, then every DSV clear. Replaces the old per-level
		// "update, maybe update_texture, maybe clear_uav, maybe clear_dsv"
		// interleaving, which forced these four distinct command types to
		// alternate up to MaxLevels times a frame. PageCameraUpdate/RowUpdate
		// are file-scope (top of this file), not local to this lambda -- see
		// their declaration comment for why.
		std::vector<PageCameraUpdate> camera_updates;
		std::vector<RowUpdate> row_updates;

		std::vector<int> dsv_clear_slots;

		{
			PROFILE(L"vsm_plan_levels_process");
			for (int level = 0; level < VSM::MaxLevels; level++)
			{
				const LevelPlan& plan = m_plan[level];
				if (!plan.valid)
					continue;

				// Nothing to redraw AND the indirection row still matches what's
				// on the GPU -- true no-op frame for this level.
				if (plan.dirty_mask == 0 && !plan.row_changed)
					continue;

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

					camera page_cam = make_light_view_camera(frame_light_pos);
					page_cam.set_projection_params(
						page_min.x, page_max.x,
						page_min.y, page_max.y,
						plan.bounds_all.znear - 10, plan.bounds_all.zfar);
					page_cam.update();

					camera_updates.push_back({ plan.slots[local], page_cam });
				}

				// Indirection is indexed by (local position, level) -- rewrite
				// this level's row whenever local->slot changed for any cell
				// (plan.row_changed, computed in plan_frame()).
				if (plan.row_changed)
				{
					RowUpdate ru;
					ru.level = level;
					ru.row.resize(pages_per_level);
					for (int local = 0; local < pages_per_level; local++)
						ru.row[local] = (uint32_t)plan.slots[local];
					row_updates.push_back(std::move(ru));
				}

				if (plan.dirty_mask == 0)
					continue;

				any_dirty = true;

				for (int local = 0; local < pages_per_level; local++)
					if ((plan.dirty_mask >> local) & 1)
						dsv_clear_slots.push_back(plan.slots[local]);
			}
		}

		{
			PROFILE(L"UPDATE");
			for (auto& u : camera_updates)
			{
				command_list->get_copy().update(*data.VSM_PageCameras, u.slot, std::span{ &u.cam.camera_cb.current, 1 });
			}
		}

		{
			PROFILE(L"update_texture");
			UINT row_stride = (UINT)(pages_side * sizeof(uint32_t));
			for (auto& ru : row_updates)
				command_list->get_copy().update_texture(
					(*data.VSM_PageTable).resource, ivec3(0, 0, 0), ivec3(pages_side, pages_side, 1), (UINT)ru.level,
					reinterpret_cast<const char*>(ru.row.data()), row_stride);
		}

		// Fresh/resized pyramid holds garbage -- clear to far (0, reversed-Z)
		// so a page's occlusion test can't falsely cull against it before
		// that page has ever rendered real depth. One array-spanning UAV
		// clear per mip (all physical slots at once) -- only runs once ever
		// (cold start / resize), independent of level iteration order.
		if (data.VSM_PageHiZ.is_new())
		{
			PROFILE(L"clear_uav");
			// whole_resource: the loop clears every mip of every slice, so the
			// declaration is exactly the whole resource. Declaring each
			// create_mip() view separately shredded it into one entry per slice
			// per mip -- all 1792 -- and the next whole-resource use then had to
			// reconcile every one of them individually.
			for (int mip = 0; mip < pyramid_mip_count; mip++)
				command_list->clear_uav(data.VSM_PageHiZ->create_mip(mip, *command_list).rwTexture2DArray,
				                        vec4(0, 0, 0, 0), /*whole_resource*/ true);
		}

		// Phase 5.18 follow-up: level_hiz's cold-start clear is NOT (0,0,0,0)
		// like page_hiz's above -- a page that's never been resident could
		// still hold real geometry once it loads, so (0,0) ("confirmed
		// empty") would be actively wrong, not just imprecise. -FLT_MAX/
		// +FLT_MAX poisons instead: min()/max() propagate it through the
		// downsample chain exactly like a real spike would, so any
		// classification query whose footprint touches an unresolved page
		// automatically fails both confidence checks and falls back to the
		// real search -- see VSMPageHiZ's own comment.
		if (data.VSM_LevelHiZ.is_new())
		{
			PROFILE(L"clear_uav_level");
			float max_f = std::numeric_limits<float>::max();
			for (int mip = 0; mip < level_pyramid_mip_count; mip++)
				command_list->clear_uav(data.VSM_LevelHiZ->create_mip(mip, *command_list).rwTexture2DArray,
				                        vec4(-max_f, max_f, 0, 0), /*whole_resource*/ true);
		}

		{
			PROFILE(L"clear_dsv");
			// Clear only the pages actually being re-rendered this frame: the
			// atlas is Static and shared across levels, so a cached page must
			// keep its depth. clear_dsv() (not set_rtv+ClearDepth) -- we're
			// not about to draw into this slice yet, so binding it as the
			// active render target would just be wasted OM-bind/transition/
			// size-bookkeeping work on top of the actual clear.
			// whole_resource: the whole atlas is bound as a DSV a few lines below
			// regardless, so declaring each clear over just its own slice buys
			// nothing and costs a lot -- 257 per-slice declarations diverged the
			// group's tracking, and the following whole-resource use then had to
			// reconcile all 2048 subresources one at a time.
			for (int slot : dsv_clear_slots)
				command_list->clear_dsv(atlas_slot_views[slot].depthStencil,
				                        true, false, 0, 0, /*whole_resource*/ true);
		}

		if (!any_dirty)
			return;

		{
			PROFILE(L"vsm_draw");
			// Dispatch entries were built and appended by VSM_GatherDispatch
			// (Phase 5.12), GPU-side -- this pass just consumes them.
			// exec_indirect's real per-entry count comes from
			// VSM_DispatchCommands' own append counter, MaxDispatchEntries
			// is only the upper bound.
			graphics.set_pipeline<PSOS::VSMDepthDraw>();
			{
				Slots::VSMPageTableData pageTableData;
				pageTableData.GetPage_table()   = data.VSM_PageTable->texture2DArray;
				pageTableData.GetPage_cameras() = data.VSM_PageCameras->structuredBuffer;
				graphics.set(pageTableData);
			}
			// No manual transition needed: like VSM_PageTable, graphics.set()
			// tracks the read state for this SRV bind itself.
			{
				Slots::VSMPageHiZ pageHiZ;
				pageHiZ.GetPage_hiz() = data.VSM_PageHiZ->texture2DArray;
				graphics.set(pageHiZ);
			}
			graphics.set(scene->compiledScene);

			{
				// Draw target: a DSV spanning every slot the mesh shader can
				// ever route a primitive to via SV_RenderTargetArrayIndex
				// (needs the multi-slice DSV support added in HAL.HLSL.ixx).
				// atlas_array_view (narrowed to physical_page_count slots,
				// not data.VSM_Atlas->depthStencil's full MaxPhysicalSlots=
				// 2048 reserved-resource array) is enough: VSMPageTable's
				// allocator never returns a slot >= physical_page_count, so
				// nothing can ever target a slice past this view's range.
				// set_rtv's own rt_transitions step walks every subresource
				// in the bound DSV's range on every draw -- this is the same
				// stop_using()-adjacent cost class as the Hi-Z batch binds,
				// just for the draw pass instead of the compute passes.
				RT::DepthOnly rt;
				rt.GetDepth() = atlas_array_view.depthStencil;
				graphics.set_rtv(rt, RTOptions::Default);
			}

			// One viewport now -- each slice is its own full page_size target,
			// so there is no per-page offset to encode.
			graphics.set_viewport(data.VSM_Atlas->get_viewport());
			graphics.set_scissor(data.VSM_Atlas->get_scissor());
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);

			// Every (level,mesh) pair's AS/MS dispatch in one call -- D3D12's
			// command signature rebinds mesh_cb/meshinstance_cb/page_batch_cb
			// per entry from the buffer just uploaded above, exactly as if a
			// live graphics.set(...) had preceded each dispatch_mesh() the
			// old per-level CPU loop issued.
			graphics.exec_indirect(*data.VSM_DispatchCommands, (UINT)MaxDispatchEntries);

			// (Was a post-mortem on assert_shared_state / SubResourcesCPU folding.
			//  That whole tracking layer is gone as of the 2026-08 barrier
			//  rewrite, so the hazard it described no longer exists.)
		}
	};

	// ---- Hi-Z pyramid rebuild (Phase 5.17: async compute, see vsm.sig's
	// VSM_HiZRebuild comment) -----------------------------------------------

	m_hizrebuild_render = [this, pages_side, pages_per_level, pyramid_mip_count, level_pyramid_mip_count](Passes::VSM_HiZRebuild::Context& data, FrameGraph::FrameContext& context)
	{
		// Rebuilds each just-redrawn page's pyramid for next time it's dirty
		// -- skipped entirely when hiz_culling_enabled is false, not just
		// left unconsulted: no copy/downsample dispatches at all, so this
		// toggle actually measures the pyramid-maintenance cost rather than
		// just disabling its result.
		if (!hiz_culling_enabled)
			return;

		auto& command_list = context.get_list();
		auto& compute       = command_list->get_compute();
		compute.set_signature(Layouts::DefaultLayout);

		// This pass records on its own (async-compute) thread, possibly
		// concurrently with VSM_RenderPages' -- build both view caches
		// from here too (each is a no-op past the first caller, thread-
		// safe via std::call_once) rather than assuming VSM_RenderPages'
		// render() already ran on the same thread. Confirmed the hard way:
		// a plain bool guard produced a live, reproducible null-deref
		// crash the first frame this pass had real work to do.
		build_atlas_views();
		build_page_hiz_views(data, pyramid_mip_count);
		build_level_hiz_views(data, level_pyramid_mip_count);

		// Phase 5.14: collect every dirty page's physical slot into one
		// flat list -- lets the whole rebuild (copy + full mip chain)
		// run as pyramid_mip_count dispatches TOTAL this frame (Z
		// dimension = dirty page count), instead of pyramid_mip_count
		// dispatches PER dirty page. Cheap CPU-side walk (at most
		// MaxLevels x pages_per_level iterations).
		std::vector<uint32_t> dirty_slots;
		{
			PROFILE(L"vsm_hiz_collect");
			for (int level = 0; level < VSM::MaxLevels; level++)
			{
				const LevelPlan& plan = m_plan[level];
				if (!plan.valid || plan.dirty_mask == 0)
					continue;

				for (int local = 0; local < pages_per_level; local++)
					if ((plan.dirty_mask >> local) & 1)
						dirty_slots.push_back((uint32_t)plan.slots[local]);
			}
		}

		// Phase 5.18 follow-up: level_hiz's own feed list, deliberately NOT
		// gated by dirty_mask like dirty_slots above -- every local page of
		// every ACTIVE level, every frame, mapped or not. Rebuilding only on
		// dirty left level_hiz's per-page texel holding whatever value that
		// page had the LAST time it was dirty, indefinitely -- including
		// after eviction (a page that's no longer mapped kept its old,
		// increasingly stale content forever instead of reading as empty)
		// and, more importantly, including the page's camera Z-range from
		// whenever that was (VSM.cpp's page_cam setup derives near/far from
		// the per-frame bounds_all, so a page dirtied 50 frames ago can
		// disagree in Z-scale with a neighbor dirtied this frame -- exactly
		// what surfaced live as flatly wrong, not just stale, confident_lit/
		// dark classification right at page seams). Rebuilding every page's
		// entry every frame doesn't fix that Z-scale drift by itself (the
		// underlying page_hiz VALUE is still whatever it was last rendered
		// with), but it does fix eviction staleness, and it's what a proper
		// per-level-stabilized-Z-range fix (separate, not done here) needs
		// to sit on top of. VSM_INVALID_SLOT (unmapped -- not needed this
		// frame, or needed but the pool was exhausted) is passed through
		// as-is; the shader writes float2(0,0) for it instead of reading
		// page_hiz, same "treat missing as empty" convention plan.slots[]
		// itself already uses elsewhere.
		std::vector<Table::VSMDirtyPageInfo> dirty_pages;
		{
			PROFILE(L"vsm_level_hiz_collect");
			for (int level = 0; level < VSM::MaxLevels; level++)
			{
				const LevelPlan& plan = m_plan[level];
				if (!plan.valid)
					continue;

				for (int local = 0; local < pages_per_level; local++)
				{
					Table::VSMDirtyPageInfo info;
					info.physical_slot = (uint32_t)plan.slots[local];
					info.level         = (uint32_t)level;
					info.page_x        = (uint32_t)(local % pages_side);
					info.page_y        = (uint32_t)(local / pages_side);
					dirty_pages.push_back(info);
				}
			}
		}

		if (dirty_slots.empty() && dirty_pages.empty())
			return;

		// No manual transitions here any more. VSM_Atlas (DSV -> SRV for
		// the copy shader) and VSM_PageHiZ's per-mip UAV/SRV binds are
		// declared [Barrier = ALL] in vsm.sig, so each bind records one
		// whole-resource use instead of expanding into its
		// physical_page_count-slice range -- which is exactly what the
		// bare add_resource_usage() calls that used to sit here were
		// faking by hand.
		//
		// Guarded on dirty_slots specifically (not the combined early-out
		// above) -- level_hiz's own feed below runs unconditionally now and
		// no longer implies page_hiz had any dirty work to do this frame.
		if (!dirty_slots.empty())
		{
			command_list->get_copy().update(*data.VSM_DirtySlots, 0, std::span{ dirty_slots });

			ivec3 dispatch_size((int)page_table.page_size, (int)page_table.page_size, (int)dirty_slots.size());

			{
				PROFILE(L"vsm_hiz_copy");
				compute.set_pipeline<PSOS::VSMCopyPageDepthBatch>();
				{
					// Narrowed views (atlas_array_view/page_hiz_mip_array_views,
					// VSM's own members, built just above), not the base
					// handlers' full-array/full-mip-chain ones. dirty_slots.
					// Load(z) still picks which physical slice each Z-group
					// touches.
					Slots::VSMCopyPageDepthBatch copy;
					copy.GetAtlas()       = atlas_array_view.texture2DArray;
					copy.GetDst_mip0()    = page_hiz_mip_array_views[0].rwTexture2DArray;
					copy.GetDirty_slots() = data.VSM_DirtySlots->structuredBuffer;
					compute.set(copy);
				}
				compute.dispatch(dispatch_size, ivec3(8, 8, 1));
			}

			{
				PROFILE(L"vsm_hiz_downsample");
				compute.set_pipeline<PSOS::VSMDownsampleHiZBatch>();
				for (int mip = 0; mip < pyramid_mip_count - 1; mip++)
				{
					int dst_size = std::max(1, page_table.page_size >> (mip + 1));

					Slots::VSMDownsampleHiZBatch down;
					// Both sides narrowed to exactly one mip (array-spanning,
					// physical_page_count slices) instead of the base
					// handler's whole 7-mip-chain SRV -- same narrowing
					// reasoning as the copy step above, and this one runs
					// pyramid_mip_count-1 times per frame, not once.
					// src_mip is no longer needed for addressing (the view
					// itself is already narrowed to that mip -- Load's mip
					// component is always 0 relative to it) but is kept for
					// parity with the entry's own bookkeeping.
					down.GetSrc()         = page_hiz_mip_array_views[mip].texture2DArray;
					down.GetDst_mip()     = page_hiz_mip_array_views[mip + 1].rwTexture2DArray;
					down.GetDirty_slots() = data.VSM_DirtySlots->structuredBuffer;
					down.GetSrc_mip()     = (uint)mip;
					compute.set(down);

					compute.dispatch(ivec3(dst_size, dst_size, (int)dirty_slots.size()), ivec3(8, 8, 1));
				}
			}
		}

		// Phase 5.18 follow-up: feed + downsample VSM_LevelHiZ, right after
		// page_hiz's own rebuild above -- needs each just-rebuilt dirty
		// page's coarsest mip as its source for THAT page's entry (a
		// non-dirty page's entry just re-copies whatever page_hiz already
		// has, unchanged -- see the collection loop's own comment for why
		// this runs unconditionally instead of only on dirty). See
		// VSMDirtyPageInfo's own comment for why this needs its own
		// (level, page_x, page_y)-carrying list instead of reusing
		// VSM_DirtySlots.
		if (!dirty_pages.empty())
		{
			command_list->get_copy().update(*data.VSM_DirtyPages, 0, std::span{ dirty_pages });

			PROFILE(L"vsm_level_hiz_copy");
			compute.set_pipeline<PSOS::VSMCopyLevelHiZBatch>();
			{
				Slots::VSMCopyLevelHiZBatch copy;
				copy.GetPage_hiz_coarsest() = page_hiz_mip_array_views[pyramid_mip_count - 1].texture2DArray;
				copy.GetLevel_hiz_mip0()    = level_hiz_mip_array_views[0].rwTexture2DArray;
				copy.GetDirty_pages()       = data.VSM_DirtyPages->structuredBuffer;
				compute.set(copy);
			}
			compute.dispatch(ivec3(1, 1, (int)dirty_pages.size()), ivec3(1, 1, 1));
		}

		{
			PROFILE(L"vsm_level_hiz_downsample");
			// Unconditionally every active-page-touching frame, across every
			// LEVEL slice (not just ones with a dirty page this frame) --
			// see VSMDownsampleLevelHiZBatch's own comment for why that's
			// cheaper than tracking a separate per-level dirty list at this
			// resolution.
			compute.set_pipeline<PSOS::VSMDownsampleLevelHiZBatch>();
			for (int mip = 0; mip < level_pyramid_mip_count - 1; mip++)
			{
				int dst_size = std::max(1, pages_side >> (mip + 1));

				Slots::VSMDownsampleLevelHiZBatch down;
				down.GetSrc()     = level_hiz_mip_array_views[mip].texture2DArray;
				down.GetDst_mip() = level_hiz_mip_array_views[mip + 1].rwTexture2DArray;
				compute.set(down);

				compute.dispatch(ivec3(dst_size, dst_size, (int)VSM::MaxLevels), ivec3(4, 4, 1));
			}
		}
	};

	// ---- Blocker search (extracted from combine, see vsm.sig's ------------
	// ---- VSM_BlockerSearch PassNode comment) -------------------------------

	m_blockersearch_setup = [this](Passes::VSM_BlockerSearch::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		// Only meaningful (and only ever dispatched) under penumbra mode --
		// there's no blocker search to extract otherwise. m_combine_render's
		// own PSO-permutation gate already skips VSM_RTX_VERIFY/etc. the
		// same way; this pass just skips existing at all.
		if (!use_vsm_penumbra)
			return false;
		GBufferViewDesc::need(builder, data.gbuffer);
		builder.need(data.VSM_Atlas, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.VSM_PageTable, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.VSM_PageCameras, FrameGraph::ResourceFlags::ComputeRead);
		// Phase 5.18 Part A: vsm_search_blocker's classification step reads
		// this -- needs this frame's freshly-rebuilt pyramid, hence
		// VSM_HiZRebuild moving to run immediately before this pass (same
		// [Async2] queue, test.sig).
		builder.need(data.VSM_PageHiZ, FrameGraph::ResourceFlags::ComputeRead);
		// Phase 5.18 follow-up: the multi-page-per-level fallback -- same
		// freshness dependency as VSM_PageHiZ above (fed by the same
		// VSM_HiZRebuild pass, same ordering requirement).
		builder.need(data.VSM_LevelHiZ, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.BlueNoise, FrameGraph::ResourceFlags::ComputeRead);
		auto& frame = builder.graph->get_context<ViewportInfo>();
		builder.create(data.VSM_BlockerResult,
		    { ivec3(frame.frame_size, 0), HAL::Format::R32G32B32A32_UINT, 1, 1 },
		    FrameGraph::ResourceFlags::UnorderedAccess);
		return true;
	};

	m_blockersearch_render = [this](Passes::VSM_BlockerSearch::Context& data, FrameGraph::FrameContext& context)
	{
		GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);

		auto& list    = *context.get_list();
		auto& compute = list.get_compute();

		auto& caminfo = context.graph->get_context<CameraInfo>();
		auto  cam     = caminfo.cam;

		context.graph->set_slot(SlotID::FrameInfo, compute);

		camera light_cam = make_light_view_camera(frame_light_pos);
		float2 cam_pos_ls = (float4(cam->position, 1) * light_cam.get_view()).xy;

		{
			Slots::VSMLighting lighting;
			gbuffer.SetTable(lighting.GetGbuffer());
			lighting.GetVsm_atlas()      = data.VSM_Atlas->texture2DArray;
			lighting.GetPage_table()     = data.VSM_PageTable->texture2DArray;
			lighting.GetPage_cameras()   = data.VSM_PageCameras->structuredBuffer;
			lighting.GetBlue_noise()     = data.BlueNoise->texture2D;
			lighting.GetBlocker_result() = data.VSM_BlockerResult->rwTexture2D;
			// Result/rtx_shadow_mask stay unbound (default) -- this pass's
			// own shader (CS_BLOCKER_SEARCH) never reads either field.
			compute.set(lighting);
		}

		{
			// Phase 5.18 Part A: same one-liner m_renderpages_render already
			// uses to bind this for the mesh shader's occlusion test -- the
			// whole mip chain, vsm_search_blocker picks its own mip.
			Slots::VSMPageHiZ pageHiZ;
			pageHiZ.GetPage_hiz()  = data.VSM_PageHiZ->texture2DArray;
			pageHiZ.GetLevel_hiz() = data.VSM_LevelHiZ->texture2DArray;
			compute.set(pageHiZ);
		}

		{
			Slots::VSMConstants constants;
			constants.GetActive_min()          = active_min;
			constants.GetActive_max()          = active_max;
			constants.GetPage_size()           = page_table.page_size;
			constants.GetPages_per_level()     = page_table.clipmap.pages_per_level;
			constants.GetQuad_blocker_search()  = use_vsm_stochastic_blocker_search ? 2 : (use_vsm_quad_blocker_search ? 1 : 0);
			constants.GetHiz_blocker_classify() = use_vsm_hiz_blocker_classify ? 1 : 0;
			// TEMP DEBUG (live "still lots of holes" investigation):
			// vsm_search_blocker's own coverage-gap-vs-genuinely-ambiguous
			// diagnostic needs this here too, not just in m_combine_render --
			// see its own comment for what it does with it. Remove alongside
			// that diagnostic once confirmed.
			constants.GetDebug_hiz_classify()   = use_vsm_debug_hiz_classify ? 1 : 0;
			constants.GetLight_view()           = light_cam.get_view();
			// rtx_dual_blur/debug_rtx_reference are resolve-only concerns
			// (see m_combine_render) -- left at their zero-initialized
			// default here, this pass's shader never reads them.

			for (int level = 0; level < page_table.clipmap.level_count; level++)
			{
				float2 origin = page_table.clipmap.grid_origin(level, cam_pos_ls);
				constants.GetLevel_info()[level] = float4(origin.x, origin.y, page_table.clipmap.page_world_size(level), 0.0f);
			}

			compute.set(constants);
		}

		compute.set_pipeline<PSOS::VSMBlockerSearchCompute>();
		compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 16, 16 });
	};

	// ---- Combine lighting ------------------------------------------------

	m_combine_setup = [this](Passes::VSM_Combine::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		GBufferViewDesc::need(builder, data.gbuffer);
		builder.need(data.ResultTexture, FrameGraph::ResourceFlags::UnorderedAccess);
		builder.need(data.VSM_Atlas, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.VSM_PageTable, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.VSM_PageCameras, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.BlueNoise, FrameGraph::ResourceFlags::ComputeRead);
		// RTXShadow runs unconditionally every frame on RTX-capable
		// hardware, independent of PSSM/VSM -- but its own setup() can
		// still return false (no RTX hardware), in which case ShadowMask
		// never gets created this frame. Same defensive builder.exists()
		// guard PSSM_Combine already uses for the same resource.
		if (use_vsm_debug_rtx_reference && builder.exists(data.ShadowMask))
			builder.need(data.ShadowMask, FrameGraph::ResourceFlags::ComputeRead);
		// Only exists when use_vsm_penumbra is on (see
		// m_blockersearch_setup's own early-out) -- same defensive
		// builder.exists() guard as ShadowMask above.
		if (builder.exists(data.VSM_BlockerResult))
			builder.need(data.VSM_BlockerResult, FrameGraph::ResourceFlags::ComputeRead);
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

		camera light_cam = make_light_view_camera(frame_light_pos);
		float2 cam_pos_ls = (float4(cam->position, 1) * light_cam.get_view()).xy;

		{
			Slots::VSMLighting lighting;
			gbuffer.SetTable(lighting.GetGbuffer());
			lighting.GetVsm_atlas()    = data.VSM_Atlas->texture2DArray;
			lighting.GetPage_table()   = data.VSM_PageTable->texture2DArray;
			lighting.GetPage_cameras() = data.VSM_PageCameras->structuredBuffer;
			lighting.GetResult()       = data.ResultTexture->rwTexture2D;
			lighting.GetBlue_noise()   = data.BlueNoise->texture2D;
			if (data.ShadowMask)
				lighting.GetRtx_shadow_mask() = data.ShadowMask->texture2D;
			// Written by m_blockersearch_render above -- only exists when
			// use_vsm_penumbra is on (see m_blockersearch_setup's own
			// early-out and this pass's own builder.exists() guard).
			if (data.VSM_BlockerResult)
				lighting.GetBlocker_result() = data.VSM_BlockerResult->rwTexture2D;
			compute.set(lighting);
		}

		{
			Slots::VSMConstants constants;
			// Phase 5.7: the active window is two scalars, computed once per
			// frame in plan_frame() (via update_active_window()) -- get_vsm_level
			// walks exactly this contiguous range, finest (active_min) first.
			constants.GetActive_min()           = active_min;
			constants.GetActive_max()           = active_max;
			constants.GetPage_size()            = page_table.page_size;
			constants.GetPages_per_level()      = page_table.clipmap.pages_per_level;
			constants.GetRtx_dual_blur()        = use_vsm_rtx_dual_blur ? 1 : 0;
			// 3-way mode packed into one int (see VSM_impl.hlsl's own
			// comment on search_mode): 0 = full, 1 = quad-shared, 2 =
			// stochastic single tap. Stochastic takes priority if somehow
			// both toggles are on -- it's the more aggressive of the two.
			constants.GetQuad_blocker_search()  = use_vsm_stochastic_blocker_search ? 2 : (use_vsm_quad_blocker_search ? 1 : 0);
			// Only meaningful when ShadowMask actually exists this frame
			// (see m_combine_setup's builder.exists() guard) -- otherwise
			// rtx_shadow_mask was never bound to anything real above.
			constants.GetDebug_rtx_reference()  = (use_vsm_debug_rtx_reference && data.ShadowMask) ? 1 : 0;
			constants.GetDebug_hiz_classify()   = use_vsm_debug_hiz_classify ? 1 : 0;
			constants.GetDebug_page_grid()      = use_vsm_debug_page_grid ? 1 : 0;
			constants.GetLight_view()           = light_cam.get_view();

			// Propagates into RTXShadow::render (PassDefaults.cpp) via the
			// RTX singleton -- see debug_full_reference_shadow's own
			// comment in RTX.ixx for why it lives there instead of a
			// VSM-local flag. One-frame lag (RTXShadow already ran earlier
			// this frame) is fine for a debug toggle.
			RTX::get().debug_full_reference_shadow = use_vsm_debug_rtx_reference;

			for (int level = 0; level < page_table.clipmap.level_count; level++)
			{
				float2 origin = page_table.clipmap.grid_origin(level, cam_pos_ls);
				constants.GetLevel_info()[level] = float4(origin.x, origin.y, page_table.clipmap.page_world_size(level), 0.0f);
			}

			compute.set(constants);
		}

		bool rtx_capable    = use_vsm_penumbra && RenderSystem::get().device().get_properties().rtx;
		bool rtx_verify     = rtx_capable && use_vsm_rtx_verify;
		if (rtx_verify)
		{
			// Same binding VSM.cpp mirrors from PassDefaults.cpp's
			// RTXShadow::render -- Raytracing has its own dedicated
			// DefaultLayout root-signature slot, so this doesn't need a
			// FrameGraph-tracked field on VSM_Combine's PassNode.
			auto& scene_ctx = context.graph->get_context<SceneInfo>();
			Slots::Raytracing rtx;
			rtx.GetScene() = scene_ctx.scene->raytrace_scene->get_handle();
			compute.set(rtx);
		}

		compute.set_pipeline<PSOS::VSMApplyCompute>(
			PSOS::VSMApplyCompute::VsmPenumbra.Use(use_vsm_penumbra)
			| PSOS::VSMApplyCompute::VsmRtxVerify.Use(rtx_verify));
		compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 16, 16 });
	};

	// ---- Depth analysis (feeds active_min's hysteresis, see update_active_window()) --

	m_depth_analysis_setup = [this](Passes::VSM_DepthAnalysis::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		GBufferViewDesc::need(builder, data.gbuffer);

		// Static: cleared and re-measured every frame, but the buffer
		// itself persists so the readback (issued after this frame's
		// dispatch, consumed in a later frame's plan_frame()) always has
		// something valid to read. This is a single, non-Multiple PassNode
		// (like VSM_RenderPages since Phase 5.8) -- nothing else ever touches this resource,
		// so it's always the owner and always calls create(); Static is
		// what keeps the underlying allocation from being torn down and
		// rebuilt every frame, not a manual is_new()/create()-once branch
		// (calling .is_new() before anything has ever created/needed this
		// resource in this pass crashed -- it isn't valid to query cold).
		builder.create(data.VSM_DepthAnalysisResult, { (size_t)1 }, FrameGraph::ResourceFlags::UnorderedAccess | FrameGraph::ResourceFlags::Static);

		return true;
	};

	m_depth_analysis_render = [this](Passes::VSM_DepthAnalysis::Context& data, FrameGraph::FrameContext& context)
	{
		GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);

		auto& command_list = context.get_list();
		auto& compute = command_list->get_compute();
		auto& copy    = command_list->get_copy();

		compute.set_signature(Layouts::DefaultLayout);
		// The shader reads GetFrameInfo().GetCamera() for InvViewProj/
		// Position/Proj -- without this, that slot is never bound for this
		// pass's compute context and the shader reads whatever garbage was
		// last there, so world_texel_size comes out NaN/Inf-ish and never
		// improves the FLT_MAX clear value via InterlockedMin (its bit
		// pattern sorts above FLT_MAX's). Confirmed via diagnostic logging:
		// the readback callback fired correctly, but the value never moved
		// even standing right up against geometry -- this was why.
		context.graph->set_slot(SlotID::FrameInfo, compute);

		// No manual transitions anywhere in this pass -- update(), set(),
		// and read()/read_buffer() all self-transition internally
		// (confirmed by reading their implementations: update_buffer/
		// read_buffer both call base.add_resource_usage(...) themselves; set()'s
		// self-tracking is already noted elsewhere in this file, "graphics.
		// set() tracks the read state for this SRV bind itself"). Adding
		// redundant manual transitions to the same states right before each
		// call was the actual bug here: the readback callback never fired
		// at all (confirmed via diagnostic logging) until these were
		// removed -- this codebase's op-batching layer has hit exactly this
		// class of same-state-transition-elision bug before.

		// Clear to FLT_MAX's bit pattern so this frame's first InterlockedMin
		// always improves it -- 0 would read back as an impossibly tiny
		// texel size (0 sorts below every real positive float's bit
		// pattern), spuriously activating every adaptive tier.
		{
			uint32_t flt_max_bits = 0x7F7FFFFFu;
			copy.update(*data.VSM_DepthAnalysisResult, 0, std::span{ &flt_max_bits, 1 });
		}

		{
			Slots::VSMDepthAnalysis analysis;
			gbuffer.SetTable(analysis.GetGbuffer());
			analysis.GetResult() = data.VSM_DepthAnalysisResult->rwStructuredBuffer;
			compute.set(analysis);
		}
		compute.set_pipeline<PSOS::VSMDepthAnalysis>();

		// Full-frame-covering dispatch; the shader's own bounds check keeps
		// only the central 50% region's threads doing real work, so this
		// doesn't need to precompute/match that region on the CPU side too.
		compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2(8, 8));

		copy.read<uint>(*data.VSM_DepthAnalysisResult, 0, 1, [this](std::span<uint> result)
		{
			measured_texel_size_bits.store(result[0], std::memory_order_relaxed);
		});
	};
}
