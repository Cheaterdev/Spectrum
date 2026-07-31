export module Graphics:VSM;

import :Materials.ShaderMaterial;
import :Camera;
import :Scene;
import :EngineAssets;
import :FrameGraphContext;
import :BRDF;
import :VSMClipmap;
import :VSMPageTable;

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

	std::mutex pos_mutex;
	float3 position;

	std::array<Passes::VSM_RenderPage::setup_func_type,  MaxLevels> m_level_setup;
	std::array<Passes::VSM_RenderPage::render_func_type, MaxLevels> m_level_render;

	Passes::VSM_Combine::setup_func_type  m_combine_setup;
	Passes::VSM_Combine::render_func_type m_combine_render;

public:

	float3 get_position();
	void set_position(float3 p);

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
