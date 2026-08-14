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

	float3 current_light_pos = get_position();
	frame_light_pos = current_light_pos;
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

VSM::VSM()
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

	// One Hi-Z pyramid slice per physical page, mip chain down to an 8x8
	// floor (1x1 wastes dispatches/VRAM for no real occluder benefit).
	const int physical_slots = page_table.physical_page_count;
	int pyramid_mip_count = 1;
	for (int s = page_table.page_size; s > 8; s >>= 1)
		pyramid_mip_count++;

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

	m_renderpages_setup = [this, physical_slots, pyramid_mip_count](Passes::VSM_RenderPages::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		builder.need(data.VSM_Atlas, FrameGraph::ResourceFlags::DepthStencil);
		builder.create(data.VSM_PageTable, { ivec3(page_table.clipmap.pages_per_level, page_table.clipmap.pages_per_level, 0), HAL::Format::R32_UINT, (UINT)page_table.clipmap.level_count, 1 }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
		builder.create(data.VSM_PageCameras, { (size_t)MaxPages }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
		// Static like VSM_Atlas: must survive until this page is next dirty.
		builder.create(data.VSM_PageHiZ, { ivec3(page_table.page_size, page_table.page_size, 0), HAL::Format::R32_FLOAT, (UINT)physical_slots, (UINT)pyramid_mip_count }, FrameGraph::ResourceFlags::UnorderedAccess | FrameGraph::ResourceFlags::Static);
		// Now GPU-appended by VSM_GatherDispatch -- this pass only reads it
		// via exec_indirect.
		builder.need(data.VSM_DispatchCommands, FrameGraph::ResourceFlags::ComputeRead);
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

	m_renderpages_render = [this, pages_side, pages_per_level, pyramid_mip_count](Passes::VSM_RenderPages::Context& data, FrameGraph::FrameContext& context)
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

		{
			std::vector<int> to_map, to_unmap;
			page_table.take_pending_tile_changes(to_map, to_unmap);
			if (!to_map.empty() || !to_unmap.empty())
			{
				auto& atlas_tiled = vsm_atlas_tex->resource->get_tiled_manager();
				uint3 tile_dims = atlas_tiled.get_tiles_count();
				uint3 to = uint3(tile_dims.x - 1, tile_dims.y - 1, 0);

				for (int slot : to_map)
					atlas_tiled.load_tiles(command_list.get(), uint3(0, 0, 0), to, (uint)slot);
				for (int slot : to_unmap)
					atlas_tiled.zero_tiles(command_list.get(), uint3(0, 0, 0), to, (uint)slot);
			}
		}

		auto& sceneinfo = context.graph->get_context<SceneInfo>();
		auto  scene     = sceneinfo.scene;

		bool any_dirty = false;
		bool hiz_cleared_this_frame = false;

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

			if (!hiz_cleared_this_frame && data.VSM_PageHiZ.is_new())
			{
				// Fresh/resized pyramid holds garbage -- clear to far (0,
				// reversed-Z) so a page's occlusion test can't falsely cull
				// against it before that page has ever rendered real depth.
				// One array-spanning UAV clear per mip (all physical slots
				// at once) -- only runs once ever (cold start / resize), not
				// once per level -- hiz_cleared_this_frame guards that since
				// there's no more level==0 to pin it to.
				for (int mip = 0; mip < pyramid_mip_count; mip++)
					command_list->clear_uav(data.VSM_PageHiZ->create_mip(mip, *command_list).rwTexture2DArray, vec4(0, 0, 0, 0));
				hiz_cleared_this_frame = true;
			}

			if (plan.dirty_mask == 0)
				continue;

			any_dirty = true;

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
		}

		if (!any_dirty)
			return;

		{
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
				// Draw target: a DSV spanning every slice, so the mesh shader
				// can route each primitive to its page with
				// SV_RenderTargetArrayIndex (needs the multi-slice DSV
				// support added in HAL.HLSL.ixx).
				RT::DepthOnly rt;
				rt.GetDepth() = data.VSM_Atlas->depthStencil;
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
		}

		// Rebuild each just-redrawn page's pyramid for next time it's dirty.
		// VSM_Atlas was just a DSV -- nudge readable for the copy shader
		// below, same one-off idiom PSSM.cpp uses; the next draw into it
		// transitions it back automatically via set_rtv.
		command_list->transition(data.VSM_Atlas->resource, HAL::ResourceStates::SHADER_RESOURCE);

		for (int level = 0; level < VSM::MaxLevels; level++)
		{
			const LevelPlan& plan = m_plan[level];
			if (!plan.valid || plan.dirty_mask == 0)
				continue;

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
		}
	};

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

		camera light_cam = make_light_view_camera(frame_light_pos);
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
			// Phase 5.7: the active window is two scalars, computed once per
			// frame in plan_frame() (via update_active_window()) -- get_vsm_level
			// walks exactly this contiguous range, finest (active_min) first.
			constants.GetActive_min()           = active_min;
			constants.GetActive_max()           = active_max;
			constants.GetPage_size()            = page_table.page_size;
			constants.GetPages_per_level()      = page_table.clipmap.pages_per_level;
			constants.GetLight_view()           = light_cam.get_view();

			for (int level = 0; level < page_table.clipmap.level_count; level++)
			{
				float2 origin = page_table.clipmap.grid_origin(level, cam_pos_ls);
				constants.GetLevel_info()[level] = float4(origin.x, origin.y, page_table.clipmap.page_world_size(level), 0.0f);
			}

			compute.set(constants);
		}

		compute.set_pipeline<PSOS::VSMApplyCompute>();
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
		// read_buffer both call base.transition(...) themselves; set()'s
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
