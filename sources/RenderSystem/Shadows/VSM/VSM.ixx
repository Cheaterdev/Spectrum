export module Graphics:VSM;

import :Materials.ShaderMaterial;
import :Camera;
import :Scene;
import :EngineAssets;
import :FrameGraphContext;
import :BRDF;
import :VSMClipmap;
import :VSMPageTable;
import :VSMInvalidationTracker;

import FrameGraph;
import HAL;

// Virtual Shadow Map: directional-light clipmap, Phase 1a.
//
// Structural mirror of PSSM (sources/RenderSystem/Lighting/PSSM.ixx) -- same
// setup_func/render_func wiring pattern into MainPipeline -- but rendering a
// fixed set of clipmap levels into a shared physical atlas instead of a
// fixed set of cascades into a Texture2DArray. PSSM itself is untouched;
// this is a fully parallel, separately-registered pass set (construction-
// time toggle in main.cpp decides which one is actually instantiated).
//
// Phase 5.8: VSM_RenderPages is a SINGLE pass, not one per level -- plan_frame()
// builds a flat list of per-(level,mesh) indirect draw entries
// (Table::VSMDispatchCommandData, one DispatchMesh's worth of threadgroups
// each) and the pass issues exactly one exec_indirect() call over all of
// them, replacing the old "for level { for mesh { dispatch_mesh() } }" CPU
// loop. MaxLevels (26) is still the fixed, generously-sized STORAGE budget
// (level_info[]/page-table array sizing); the actually-active contiguous
// sub-range [active_min, active_max] is computed every frame (Phase 5.7)
// and determines which levels actually contribute entries this frame.
export class VSM
{
public:
	// Runtime A/B toggle for measuring Hi-Z occlusion culling's real cost:
	// when false, plan_level() forces every resident page's skip_occlusion
	// bit (the AS then never consults the pyramid, same as a page with no
	// valid history) and m_renderpages_render skips rebuilding it entirely
	// -- no copy/downsample dispatches at all, not just an untested pyramid.
	// Plain bool: only ever read/written from the render thread and the
	// UI's on_check callback (main thread), same low-ceremony treatment as
	// auto_rotate_sun in main.cpp.
	bool hiz_culling_enabled = true;

	// Runtime A/B toggle between the fixed single-tap 3x3 hardware-PCF grid
	// (get_shadow_vsm's default path) and the PCSS-style blocker-search +
	// penumbra-scaled variant (VSM_impl.hlsl, gated by VSM_PENUMBRA) -- picks
	// which VSMApplyCompute PSO permutation gets bound in m_combine_render.
	// Off by default: new, unvalidated shader math, kept separate from the
	// known-working fixed-tap baseline until confirmed visually correct.
	bool use_vsm_penumbra = true; // TEMP: flip back to false after validation

	// Runtime toggle for the RTX blocker-distance verification ray (Phase
	// 5.18 Part B) -- only has any effect when use_vsm_penumbra is also on
	// and the device supports RTX (both checked in m_combine_render before
	// selecting the VsmRtxVerify PSO permutation). Off by default: new,
	// unvalidated, same cautious rollout as use_vsm_penumbra above.
	bool use_vsm_rtx_verify = false;

	// Only meaningful when use_vsm_rtx_verify is also on. false = single
	// blur pass (uses the RTX-verified distance when the ray hit something,
	// VSM's own estimate otherwise -- cheaper). true = blur both distances
	// and take min() of the two resulting shadow values -- pricier (an
	// extra 16-tap blur whenever the ray hits) but confirmed visually to
	// remove the bright spots a single blended estimate left between
	// overlapping penumbras. Defaults to the confirmed-better option since
	// this is a quality/perf choice, not an unvalidated-math gate like the
	// two toggles above.
	bool use_vsm_rtx_dual_blur = true;

	// Runtime A/B switch for the quad-shared blocker search (splits the 16
	// Poisson-disc taps 4-per-thread across each 2x2 pixel quad instead of
	// every pixel doing all 16, merged via QuadReadAcrossX/Y/Diagonal).
	// Off by default -- new, not yet visually/perf verified against the
	// original full-per-pixel search, same cautious rollout as
	// use_vsm_rtx_verify above.
	bool use_vsm_quad_blocker_search = false;
private:
	// Tracks the previous frame's toggle state so plan_frame() can detect
	// an off->on transition and force a full pyramid rebuild -- see its
	// use there.
	bool hiz_culling_enabled_prev = true;
public:

	// Phase 5.7: one unified storage budget, no more separate "regular ring"
	// / "adaptive tier" split. LevelZeroSlot is the fixed index whose size
	// equals VSMClipmap::base_page_world_size; slots below are finer, above
	// are coarser (see VSMClipmap::page_world_size). Keep MaxLevels in step
	// with VSMConstants::level_info[26] (Phase 5.8: no longer also a
	// [Multiple=N] PassNode budget -- VSM_RenderPages is a single pass now).
	static constexpr int MaxLevels = 26;
	static constexpr int LevelZeroSlot = 12;
	static constexpr int MaxPagesPerLevel = 16;   // 4x4, matches VSMClipmap::pages_per_level
	static constexpr int MaxPages = MaxLevels * MaxPagesPerLevel;
	// Phase 5.8: upper bound on per-frame (active level x scene mesh)
	// indirect draw entries -- generous, not a measured real number (mirrors
	// physical_page_count's "moderate bump" philosophy). If a scene's mesh
	// count x active level count ever exceeds this, entries are clamped and
	// logged once per episode rather than overflowing the buffer.
	static constexpr int MaxDispatchEntries = MaxLevels * 2048;
	// VSM_Atlas's logical slice count, decoupled from physical_page_count
	// (the real, elastic VRAM budget -- see VSMPageTable's map/unmap
	// tracking). D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION is 2048, the
	// hardware ceiling for a Texture2DArray's ArraySize -- since VSM_Atlas
	// is a reserved resource now, an unused slice costs nothing but the
	// tiny per-slice tile bookkeeping, so there's no reason to size it any
	// smaller than the API allows.
	static constexpr int MaxPhysicalSlots = 2048;

private:
	HAL::Texture::ptr vsm_atlas_tex;

	VSMPageTable page_table;
	VSMInvalidationTracker tracker;

	// Last world bounds marked dirty for each tracked object (see
	// attach_scene()) -- a fast-moving object can leave a page's footprint
	// entirely between two move events, so its OLD bounds need marking too,
	// not just its current ones, or the vacated page's stale shadow content
	// never gets invalidated. Scene events can fire from arbitrary threads.
	std::mutex bounds_mutex;
	std::unordered_map<scene_object*, std::pair<vec3, vec3>> last_marked_bounds;

	std::mutex pos_mutex;
	float3 position;

	// Per-frame snapshot of `position`, taken once at the top of plan_frame()
	// and used by EVERY consumer for the rest of the frame (page planning, the
	// per-page rasterization cameras in m_renderpages_render, and the
	// light_view uploaded for sampling in m_combine_render).
	//
	// These three used to call get_position() independently. set_position() is
	// driven from the UI thread, so while the light is being dragged the three
	// reads can each observe a DIFFERENT direction within one frame: pages get
	// planned in one light basis, rasterized in a second, and sampled in a
	// third. The resulting mismatched depth content is then cached forever --
	// dirty_mask returns to 0 as soon as the light stops moving, so nothing
	// ever redraws it (confirmed live: resident=28 dirty=0 held for 20+
	// seconds while shadows stayed visibly wrong). That's why shadows were
	// correct the first time through a given light direction but permanently
	// broken after sweeping the light around and coming back.
	//
	// plan_frame() runs in Graph::compile()'s pre_run -- single-threaded and
	// strictly before any pass's render() -- so a snapshot taken there is
	// stable and consistent for the whole frame.
	float3 frame_light_pos;

	// Phase 5.8 note: VSM_RenderPages is a single pass now, not one Multiple-
	// slot instance per level, so the original concurrency hazard here (two
	// levels' render() callbacks racing over the same slot) no longer
	// applies literally -- but the underlying reason plan_frame() exists is
	// unchanged: allocation decisions (which page gets a physical slot when
	// demand exceeds supply, priority-stealing across levels) still need to
	// be fully resolved BEFORE any GPU commands referencing slot assignments
	// are built, since the single render() call now builds ALL levels'
	// indirect draw entries in one pass and can't discover a slot changed
	// out from under it mid-build. Single-threaded, once-per-frame,
	// strictly before render() runs is still the right place for this.
	//
	// So all of that moves to plan_frame(), a Graph::add_slot_generator
	// callback (see main.cpp) that runs once per frame, single-threaded,
	// strictly before any pass's render() is dispatched to the thread pool
	// (Graph::compile() finishes -- including every pre_run entry -- before
	// Graph::render() begins). Every level is planned there, finest first,
	// so a fine level always gets first pick of a free slot and may steal
	// from an already-resident but strictly coarser page if none are free;
	// a coarser level then simply sees that page as gone and requests a
	// fresh one. No page ever changes owner while any level's render() is
	// running, so nothing here needs a lock.
	struct LevelPlan
	{
		std::array<int, MaxPagesPerLevel> slots{};
		uint32_t resident_mask       = 0;
		uint32_t newly_allocated_mask = 0;
		uint32_t dirty_mask          = 0;
		uint32_t skip_occlusion_mask = 0;
		float2   origin{};
		box      bounds_all{};
		bool     row_changed = false;
		// False until plan_frame() has run at least once for this level
		// (e.g. level_count < MaxLevels leaves the rest never planned).
		bool     valid = false;
	};
	std::array<LevelPlan, MaxLevels> m_plan;

	// "Is this the first time / has the clipmap grid recentered" tracking,
	// and last frame's local->slot assignment (to know whether the
	// indirection row needs rewriting -- not just on recenter, but also
	// when residency culling flips a cell in/out or a priority steal
	// reassigns a slot this level was pointing at). Both read and written
	// only from plan_frame(), which is single-threaded, so no lock needed.
	std::array<bool, MaxLevels>   level_initialized{};
	std::array<float2, MaxLevels> cached_origin{};
	std::array<std::array<int, MaxPagesPerLevel>, MaxLevels> cached_slots{};

	// Scene-dirty bits for pages that aren't resident yet (not needed this
	// frame, or needed but the pool was exhausted) -- held here directly
	// instead of being bounced back through VSMInvalidationTracker every
	// frame. The tracker's take_dirty() is destructive-on-read, so routing
	// these bits back into it via mark_pages() just to have them read right
	// back out next frame (since resident_mask hasn't changed) was a
	// permanent no-op busy-loop for any page outside the current frustum --
	// confirmed live (scene_mask sitting at a fixed nonzero value for an
	// entire session). Cleared for exactly the bits that become resident
	// each frame (folded into scene_mask there); otherwise persists untouched.
	std::array<uint32_t, MaxLevels> pending_scene_mask{};

	// Light direction/position isn't a Scene event -- set_position() (driven
	// by the sun-direction UI control) doesn't touch the scene at all, so
	// VSMInvalidationTracker never sees it. Without this, a light change
	// wouldn't just leave shadows stale: VSM_Combine picks pages using the
	// *current* light-space projection while sampling depth data rendered
	// under whatever light direction was active last time that page
	// rendered -- a real mismatch, not just staleness.
	//
	// Set atomically in set_position (guarded by pos_mutex) since the UI
	// thread can call it concurrently with plan_frame() reading it; read
	// and cleared once per level, per frame, in plan_frame().
	std::array<bool, MaxLevels> light_change_pending{};

	// Logical clock for VSMPageTable's LRU touch/eviction (used to break
	// ties among multiple steal candidates -- see acquire_slot_with_priority).
	// One per frame now (plan_frame() runs single-threaded), not per level.
	uint64_t m_frame_id = 0;

	// Diagnostics for allocate_or_touch failing closed (no free slot AND
	// nothing coarser to steal from -- see VSMPageTable's class comment).
	// Set true on the first failed allocation, cleared on the next
	// successful one, so plan_frame() can log the start of an exhaustion
	// episode without spamming once per failed cell per frame. Only ever
	// touched from plan_frame() (single-threaded), atomic only so a future
	// debug UI could read them from another thread without racing.
	std::atomic<bool>     allocation_exhausted{ false };
	std::atomic<uint64_t> total_failed_allocations{ 0 };

	// Phase 5.7: the active range is two committed scalars, not a per-level
	// flag -- activation is always a contiguous [active_min, active_max]
	// (never a gap in the middle). active_max is a correctness floor (must
	// cover the camera's z_far, or a visible patch of frustum goes
	// unshadowed) so it grows instantly and only shrinks after a debounce;
	// active_min is a quality knob (driven by the depth-analysis MIN-texel
	// signal, same as Phase 5.6's tiers) with the same instant-grow/
	// debounced-shrink shape. See update_active_window().
	static constexpr int ShrinkFrames = 90; // biased toward staying active -- flapping is worse than a few extra frames of unnecessary coverage
	int active_min = LevelZeroSlot;
	int active_max = LevelZeroSlot;
	int min_shrink_run = 0;
	int max_shrink_run = 0;

	// Written from VSM_DepthAnalysis's GPU->CPU readback callback (may fire
	// on a different thread than plan_frame()'s single-threaded pre_run --
	// see VSM_DepthAnalysis.cpp), read once per frame in plan_frame().
	// Starts at FLT_MAX's bit pattern ("nothing needs to be finer yet") so
	// a cold start before the first real measurement arrives doesn't
	// spuriously activate a tier.
	std::atomic<uint32_t> measured_texel_size_bits{ 0x7F7FFFFFu };

	// Phase 5.12: one entry per active+dirty LEVEL this frame (bounded by
	// level count, not mesh count), built in VSM_GatherDispatch's render()
	// and uploaded to VSM_LevelDispatchInfo -- the GPU gather compute shader
	// tests every scene mesh's AABB against every entry and appends the
	// actual per-(level,mesh) VSMDispatchCommandData entries itself.
	// Rebuilt fresh every frame, same as the old m_dispatch_entries it
	// replaces.
	std::vector<Table::VSMLevelDispatchInfo> m_level_dispatch_info;

	// Phase 5.13: every per-slot view VSM_RenderPages' render() would
	// otherwise call create_2d_slice() for -- once per dirty page, every
	// frame, for the atlas DSV clear -- is instead built ONCE, eagerly,
	// for every physical slot, from the engine's permanent descriptor
	// storage (Device::get_static_gpu_data(), not the per-frame resettable
	// one create_2d_slice normally allocates from). A slot's subresource
	// identity never changes -- only which logical page currently owns it
	// does -- so these views stay valid for the whole lifetime of
	// VSM_Atlas. Built on VSM_RenderPages' first real render()
	// (data.VSM_PageHiZ doesn't exist before its own setup() runs, so this
	// can't happen at VSM construction time); see build_slot_views().
	// Phase 5.17: VSM_RenderPages (direct queue) and VSM_HiZRebuild (async
	// compute queue) now record on genuinely different threads -- the GPU-
	// level dependency via VSM_Atlas guarantees the *GPU work* orders
	// correctly, but says nothing about which pass's render() callback the
	// CPU records FIRST. A plain bool "built" flag raced (confirmed live:
	// a null-deref crash the very first frame VSM_HiZRebuild had real work,
	// landing right where it first touched atlas_array_view -- built by
	// VSM_RenderPages' render(), which this assumed always ran first).
	// std::call_once makes "build lazily, exactly once, safe from either
	// thread" actually true instead of assumed.
	std::once_flag atlas_views_once;
	std::vector<HAL::Texture2DView> atlas_slot_views; // [slot], DSV clear only

	// Phase 5.14: batched Hi-Z copy source, narrowed to physical_page_count
	// slices -- NOT VSM_Atlas's own base handler view, which spans the
	// full MaxPhysicalSlots=2048 reserved-resource array. Originally narrowed
	// to dodge HAL::Transitions::stop_using()'s O(subresources in view) x
	// O(total resource subresource count) teardown cost (~2ms/call on the
	// wide view); stop_using() has since been removed entirely (it only fed
	// a dead split-barrier mechanism),
	// so that specific cost no longer applies, but the narrower binding is
	// still the right shape for this dispatch. See build_slot_views().
	HAL::Texture2DView atlas_array_view;

	// Phase 5.14: the Hi-Z pyramid rebuild is batched across every dirty
	// page at once (one dispatch per mip level, not one per dirty page per
	// mip). Array-spanning (every physical slot at once) but narrowed to
	// exactly ONE mip per entry -- both SRV and UAV -- instead of
	// VSM_PageHiZ's base handler view, which spans the WHOLE mip chain
	// (pyramid_mip_count x physical_page_count subresources). Same
	// stop_using()-avoidance reasoning as atlas_array_view above (see that
	// comment); stop_using() is gone now, but this stayed narrow since it's
	// rebound on every one of the pyramid_mip_count-1 downsample dispatches,
	// not just once. Phase 5.17: only VSM_HiZRebuild's render() ever touches
	// this now (the whole rebuild moved there), so it doesn't need the
	// cross-pass synchronization atlas_views_once exists for -- but it's
	// still built lazily via its own once_flag for the same "exactly once"
	// guarantee, cheap insurance against a future second caller.
	std::once_flag page_hiz_views_once;
	std::vector<HAL::Texture2DView> page_hiz_mip_array_views; // [mip]

	// Builds atlas_slot_views/atlas_array_view -- needs only vsm_atlas_tex,
	// no pass Context, so it's callable identically from either
	// VSM_RenderPages' or VSM_HiZRebuild's render() (both need
	// atlas_array_view; only VSM_RenderPages needs atlas_slot_views for its
	// per-dirty-page DSV clears). Thread-safe via atlas_views_once -- see
	// its declaration for why that's required, not just tidy.
	void build_atlas_views();

	// Builds page_hiz_mip_array_views from VSM_HiZRebuild's OWN Context
	// (that PassNode need()s VSM_PageHiZ too, alongside VSM_RenderPages,
	// which still create()s it for the once-ever cold-start clear -- see
	// vsm.sig's VSM_HiZRebuild comment).
	void build_page_hiz_views(Passes::VSM_HiZRebuild::Context& data, int pyramid_mip_count);

	Passes::VSM_GatherDispatch::setup_func_type  m_gatherdispatch_setup;
	Passes::VSM_GatherDispatch::render_func_type m_gatherdispatch_render;

	Passes::VSM_RenderPages::setup_func_type  m_renderpages_setup;
	Passes::VSM_RenderPages::render_func_type m_renderpages_render;

	// Phase 5.17: split off VSM_RenderPages so the per-frame Hi-Z pyramid
	// rebuild (copy + downsample dispatches) runs on the async compute
	// queue instead of serializing into VSM_RenderPages' own direct-queue
	// pass -- nothing else this frame reads VSM_PageHiZ, only next frame's
	// draw does. See vsm.sig's VSM_HiZRebuild PassNode comment.
	Passes::VSM_HiZRebuild::setup_func_type  m_hizrebuild_setup;
	Passes::VSM_HiZRebuild::render_func_type m_hizrebuild_render;

	Passes::VSM_Combine::setup_func_type  m_combine_setup;
	Passes::VSM_Combine::render_func_type m_combine_render;

	Passes::VSM_DepthAnalysis::setup_func_type  m_depth_analysis_setup;
	Passes::VSM_DepthAnalysis::render_func_type m_depth_analysis_render;

	// Shared body for planning one storage level (regular or adaptive) --
	// see plan_frame()'s definition for the full walkthrough of what this
	// does and why. Writes m_plan[level].
	void plan_level(int level, float2 cam_pos_ls, const box& bounds_all, uint64_t tick,
	                 int pages_side, int pages_per_level, uint32_t all_pages_mask);

	// Reads measured_texel_size_bits and cam->z_far, applies the asymmetric
	// grow/shrink hysteresis described above, updates active_min/active_max.
	// Called once at the top of plan_frame(), before any level is planned.
	void update_active_window(float z_far);

public:

	float3 get_position();
	void set_position(float3 p);

	// Call once, after the Scene exists (VSM is a class member wired at
	// construction time, before main.cpp creates the scene -- mirrors
	// stenciler->scene = scene; being assigned post-construction too).
	void attach_scene(std::shared_ptr<Scene> scene);

	// Once-per-frame, single-threaded planning pass -- see the LevelPlan
	// comment above for why this can't live inside render(). Register via
	// graph.add_slot_generator in main.cpp, alongside FrameInfo's.
	void plan_frame(FrameGraph::Graph& graph);

	void pass_data(FrameGraph::TaskBuilder& builder);

	VSM();

	template<typename TPipeline>
	explicit VSM(TPipeline& pipeline) : VSM()
	{
		pipeline.vSM_GatherDispatch.setup_func  = m_gatherdispatch_setup;
		pipeline.vSM_GatherDispatch.render_func = m_gatherdispatch_render;

		pipeline.vSM_RenderPages.setup_func  = m_renderpages_setup;
		pipeline.vSM_RenderPages.render_func = m_renderpages_render;

		pipeline.vSM_HiZRebuild.setup_func  = m_hizrebuild_setup;
		pipeline.vSM_HiZRebuild.render_func = m_hizrebuild_render;

		pipeline.vSM_Combine.setup_func  = m_combine_setup;
		pipeline.vSM_Combine.render_func = m_combine_render;

		pipeline.vSM_DepthAnalysis.setup_func  = m_depth_analysis_setup;
		pipeline.vSM_DepthAnalysis.render_func = m_depth_analysis_render;
	}
};
