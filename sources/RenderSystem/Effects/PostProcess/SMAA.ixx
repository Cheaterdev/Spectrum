export module Graphics:SMAA;


import FrameGraph;
import HAL;
import :FrameGraphContext;
import :Texture;

export class SMAA
{
	HAL::Texture::ptr area_tex;
	HAL::Texture::ptr search_tex;

	// setup() is generated (smaa.sig's own [SetupCondition]) -- render only.
	Passes::SMAA::render_func_type m_smaa_render;

public:
	// Default constructor: loads the SMAA lookup textures from disk and
	// initialises the render function member.
	// Defined in SMAA.cpp so the EngineAsset declarations stay out of the interface.
	SMAA();

	// Template constructor: wires the render func onto the smaa pass
	// of any pipeline that exposes a 'smaa' member of type Passes::SMAA.
	// Delegates to SMAA() so the textures and function members are ready.
	template<typename TPipeline>
	explicit SMAA(TPipeline& pipeline) : SMAA()
	{
		pipeline.sMAA.render_func = m_smaa_render;
	}
};
