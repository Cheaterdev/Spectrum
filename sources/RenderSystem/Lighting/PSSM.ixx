export module Graphics:PSSM;

import :Materials.ShaderMaterial;
import :Camera;
import :Scene;
import :EngineAssets;
import :FrameGraphContext;
  import :BRDF;

import FrameGraph;
import HAL;

export class PSSM
{
	float2 pixel_size = float2(1, 1) / float2(1024, 1024);

	// Derived from the camera every time it is needed (cascade_scaler()) rather
	// than cached by PSSM_Global's setup -- that setup is generated now, and the
	// value is a one-line function of cam->z_far that only the cascade renders
	// read anyway.
	float cascade_scaler(const camera* cam) const;

	// Mirrors pssm.sig's own PSSM_RendersSize (PSSM_Cascade's ArrayCount/
	// buffer-size there) -- the many loop bounds/scaler math below need a
	// plain int, not a SIG field, but this keeps both sides reading the
	// same single source of truth instead of two independently hand-kept
	// numbers.
	static const int renders_size = Constants::PSSM_RendersSize;

	std::mutex pos_mutex;
	float3 position;
	size_t counter = 0;

	// Every PSSM pass is [RunAlways] (pssm.sig), so all four setups are
	// generated and only the render halves live here.
	Passes::PSSM_Global::render_func_type m_global_render;
	std::array<Passes::PSSM_Cascade::render_func_type, renders_size> m_cascade_render;
	Passes::PSSM_GenerateMask::render_func_type m_mask_render;
	Passes::PSSM_Combine::render_func_type m_combine_render;

public:

	float3 get_position();
	void set_position(float3 p);

	PSSM();

	template<typename TPipeline>
	explicit PSSM(TPipeline& pipeline) : PSSM()
	{
		pipeline.pSSM_Global.render_func = m_global_render;

		for (int i = 0; i < renders_size; i++)
			pipeline.pSSM_Cascade.render_funcs[i] = m_cascade_render[i];

		pipeline.pSSM_GenerateMask.render_func = m_mask_render;
		pipeline.pSSM_Combine.render_func      = m_combine_render;
	}
};
