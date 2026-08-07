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
// VSM_RenderPage is one pass PER LEVEL (not per page): each level's whole
// pages_per_level^2 page grid is rendered in a single mesh-shader dispatch,
// routed via SV_ViewportArrayIndex (see mesh_shader_vsm.hlsl) -- MaxLevels
// is the [Multiple=26] PassNode budget, a fixed, generously-sized storage
// array; the actually-active contiguous sub-range [active_min, active_max]
// is computed every frame (Phase 5.7) and is what varies at runtime.
export class VSM
{
public:
	// Phase 5.7: one unified storage budget, no more separate "regular ring"
	// / "adaptive tier" split. LevelZeroSlot is the fixed index whose size
	// equals VSMClipmap::base_page_world_size; slots below are finer, above
	// are coarser (see VSMClipmap::page_world_size). Keep MaxLevels in step
	// with vsm.sig's [Multiple=26] on VSM_RenderPage and
	// VSMConstants::level_info[26].
	static constexpr int MaxLevels = 26;
	static constexpr int LevelZeroSlot = 12;
	static constexpr int MaxPagesPerLevel = 16;   // 4x4, matches VSMClipmap::pages_per_level
	static constexpr int MaxPages = MaxLevels * MaxPagesPerLevel;

private:
	VSMPageTable page_table;
	VSMInvalidationTracker tracker;

	std::mutex pos_mutex;
	float3 position;

	// Per-level VSM_RenderPage render() callbacks run concurrently (confirmed
	// via logging -- different Multiple-slot instances of the same PassNode
	// are not serialized). That makes any decision that spans levels (which
	// page gets a physical slot when demand exceeds supply -- see
	// plan_frame()) unsafe to make from inside render(): two levels could
	// race over the same slot, or a fine level's "steal from a coarser
	// level" decision could land on a page the coarser level had already
	// committed rendering commands for this same frame.
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

	std::array<Passes::VSM_RenderPage::setup_func_type,  MaxLevels> m_level_setup;
	std::array<Passes::VSM_RenderPage::render_func_type, MaxLevels> m_level_render;

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

	VSM();

	template<typename TPipeline>
	explicit VSM(TPipeline& pipeline) : VSM()
	{
		for (int i = 0; i < MaxLevels; i++)
		{
			pipeline.vSM_RenderPage.setup_funcs[i]  = m_level_setup[i];
			pipeline.vSM_RenderPage.render_funcs[i] = m_level_render[i];
		}

		pipeline.vSM_Combine.setup_func  = m_combine_setup;
		pipeline.vSM_Combine.render_func = m_combine_render;

		pipeline.vSM_DepthAnalysis.setup_func  = m_depth_analysis_setup;
		pipeline.vSM_DepthAnalysis.render_func = m_depth_analysis_render;
	}
};
