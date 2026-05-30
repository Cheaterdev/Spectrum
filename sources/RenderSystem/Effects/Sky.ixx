export module Graphics:Sky;


import :FrameGraphContext;
import FrameGraph;
import HAL;
import :Texture;


export class SkyRender
{
	HAL::Texture::ptr transmittance;
	HAL::Texture::ptr irradiance;
	HAL::Texture::ptr inscatter;

	float3 dir = float3(0, 0, -1);

	Passes::CubeSky::setup_func_type  m_cubesky_setup;
	Passes::CubeSky::render_func_type m_cubesky_render;
	Passes::Sky::setup_func_type      m_sky_setup;
	Passes::Sky::render_func_type     m_sky_render;

public:
	using ptr = std::shared_ptr<SkyRender>;

	// Default constructor: loads sky lookup textures from disk and
	// initialises the setup/render function members.
	// Defined in Sky.cpp so the EngineAsset declarations stay out of the interface.
	SkyRender();

	// Template constructor: wires CubeSky and Sky pass funcs onto any pipeline
	// that exposes matching pass members.  Delegates to SkyRender() so the
	// lookup textures and function members are always ready.
	template<typename TPipeline>
	explicit SkyRender(TPipeline& pipeline) : SkyRender()
	{
		pipeline.cubeSky.setup_func  = m_cubesky_setup;
		pipeline.cubeSky.render_func = m_cubesky_render;
		pipeline.sky.setup_func      = m_sky_setup;
		pipeline.sky.render_func     = m_sky_render;
	}
};
