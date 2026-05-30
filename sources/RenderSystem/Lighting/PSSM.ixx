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
	ivec2 size = { 1024, 1024 };
	float2 pixel_size = float2(1, 1) / float2(1024, 1024);

	float scaler = 1;

	static const int renders_size = 5;

	std::mutex pos_mutex;
	float3 position;
	size_t counter = 0;

	Passes::PSSM_Global::setup_func_type  m_global_setup;
	Passes::PSSM_Global::render_func_type m_global_render;

	std::array<Passes::PSSM_Cascade::setup_func_type,  renders_size> m_cascade_setup;
	std::array<Passes::PSSM_Cascade::render_func_type, renders_size> m_cascade_render;

	Passes::PSSM_GenerateMask::setup_func_type  m_mask_setup;
	Passes::PSSM_GenerateMask::render_func_type m_mask_render;

	Passes::PSSM_Combine::setup_func_type  m_combine_setup;
	Passes::PSSM_Combine::render_func_type m_combine_render;

public:

	float3 get_position();
	void set_position(float3 p);

	PSSM();

	template<typename TPipeline>
	explicit PSSM(TPipeline& pipeline) : PSSM()
	{
		pipeline.pSSM_Global.setup_func  = m_global_setup;
		pipeline.pSSM_Global.render_func = m_global_render;

		for (int i = 0; i < renders_size; i++)
		{
			pipeline.pSSM_Cascade.setup_funcs[i]  = m_cascade_setup[i];
			pipeline.pSSM_Cascade.render_funcs[i] = m_cascade_render[i];
		}

		pipeline.pSSM_GenerateMask.setup_func  = m_mask_setup;
		pipeline.pSSM_GenerateMask.render_func = m_mask_render;

		pipeline.pSSM_Combine.setup_func  = m_combine_setup;
		pipeline.pSSM_Combine.render_func = m_combine_render;
	}
};
