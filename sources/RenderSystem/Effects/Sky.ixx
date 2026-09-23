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

	// No `dir` member any more: the previous sun direction lives in
	// Table::SkyState (sky.prism), which is per-Graph -- so the main and asset
	// pipelines still keep separate histories even though the diff now runs in
	// a static [PreSetup] hook. Both passes' setups are generated, so only the
	// render halves are members.
	Passes::CubeSky::render_func_type m_cubesky_render;
	Passes::Sky::render_func_type     m_sky_render;

public:
	using ptr = std::shared_ptr<SkyRender>;

	// Default constructor: loads sky lookup textures from disk and
	// initialises the render function members.
	// Defined in Sky.cpp so the EngineAsset declarations stay out of the interface.
	SkyRender();

	// Template constructor: wires CubeSky and Sky render funcs onto any
	// pipeline that exposes matching pass members.  Delegates to SkyRender() so
	// the lookup textures and function members are always ready.
	template<typename TPipeline>
	explicit SkyRender(TPipeline& pipeline) : SkyRender()
	{
		pipeline.cubeSky.render_func = m_cubesky_render;
		pipeline.sky.render_func     = m_sky_render;
	}
};
