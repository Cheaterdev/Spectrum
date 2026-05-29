export module Graphics:SMAA;

import <RenderSystem.h>;
import FrameGraph;
import HAL;
import :FrameGraphContext;
import :Texture;

export class SMAA
{
	HAL::Texture::ptr area_tex;
	HAL::Texture::ptr search_tex;

	Passes::SMAA::setup_func_type  m_smaa_setup;
	Passes::SMAA::render_func_type m_smaa_render;

public:
	// Default constructor: loads the SMAA lookup textures from disk and
	// initialises the setup/render function members.
	// Defined in SMAA.cpp so the EngineAsset declarations stay out of the interface.
	SMAA();

	// Template constructor: wires setup/render funcs onto the smaa pass
	// of any pipeline that exposes a 'smaa' member of type Passes::SMAA.
	// Delegates to SMAA() so the textures and function members are ready.
	template<typename TPipeline>
	explicit SMAA(TPipeline& pipeline) : SMAA()
	{
		pipeline.sMAA.setup_func  = m_smaa_setup;
		pipeline.sMAA.render_func = m_smaa_render;
	}
};
