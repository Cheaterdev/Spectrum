#pragma once


class CubeMapEnviromentProcessor : public Singleton<CubeMapEnviromentProcessor>, public FrameGraph::GraphGenerator
{
public:

	CubeMapEnviromentProcessor();

	virtual void generate(FrameGraph::Graph& graph) override;
};



class SkyRender :public Events::prop_handler, public FrameGraph::GraphGenerator
{
	Render::Texture::ptr transmittance;
	Render::Texture::ptr irradiance;
	Render::Texture::ptr inscatter;
	
	bool processed = false;

	float3 dir = float3(0, 0, -1);
public:
	using ptr = std::shared_ptr<SkyRender>;

	SkyRender();

	virtual void generate(FrameGraph::Graph& graph) override;
	 void generate_sky(FrameGraph::Graph& graph) ;

};
