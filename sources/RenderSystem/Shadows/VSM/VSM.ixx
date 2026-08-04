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
// is the [Multiple=8] PassNode budget, level_count (runtime, currently 3)
// is how many of those slots actually get wired, matching how PSSM_Cascade
// only wires renders_size of its MaxCount=6 slots.
export class VSM
{
public:
	static constexpr int MaxLevels = 8;
	static constexpr int MaxPagesPerLevel = 16;   // 4x4, matches VSMClipmap::pages_per_level
	static constexpr int MaxPages = MaxLevels * MaxPagesPerLevel;

private:
	VSMPageTable page_table;
	VSMInvalidationTracker tracker;

	std::mutex pos_mutex;
	float3 position;

	// Phase 2 caching state: per-level "is this the first time / has the
	// clipmap grid recentered since last frame" tracking. Written and read
	// only from within VSM_RenderPage's render() callbacks, which the
	// FrameGraph executes single-threaded per frame, so no separate lock is
	// needed beyond pos_mutex (which already guards `position`, read here
	// too via get_position()).
	std::array<bool, MaxLevels>   level_initialized{};
	std::array<float2, MaxLevels> cached_origin{};

	// Light direction/position isn't a Scene event -- set_position() (driven
	// by the sun-direction UI control) doesn't touch the scene at all, so
	// VSMInvalidationTracker never sees it. Without this, a light change
	// wouldn't just leave shadows stale: VSM_Combine picks pages using the
	// *current* light-space projection while sampling depth data rendered
	// under whatever light direction was active last time that page
	// rendered -- a real mismatch, not just staleness.
	//
	// Push-based (set atomically in set_position, guarded by pos_mutex),
	// not poll-and-compare against a shared "last position": the UI thread
	// can mutate `position` between two levels' renders in the same frame,
	// so independent comparisons let levels disagree about whether the
	// light moved. Each level clears only its own entry once consumed.
	std::array<bool, MaxLevels> light_change_pending{};

	std::array<Passes::VSM_RenderPage::setup_func_type,  MaxLevels> m_level_setup;
	std::array<Passes::VSM_RenderPage::render_func_type, MaxLevels> m_level_render;

	Passes::VSM_Combine::setup_func_type  m_combine_setup;
	Passes::VSM_Combine::render_func_type m_combine_render;

public:

	float3 get_position();
	void set_position(float3 p);

	// Call once, after the Scene exists (VSM is a class member wired at
	// construction time, before main.cpp creates the scene -- mirrors
	// stenciler->scene = scene; being assigned post-construction too).
	void attach_scene(std::shared_ptr<Scene> scene);

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
	}
};
