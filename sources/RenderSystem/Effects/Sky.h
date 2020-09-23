#pragma once


class CubeMapEnviromentProcessor : public Singleton<CubeMapEnviromentProcessor>, public FrameGraphGenerator
{
public:

	CubeMapEnviromentProcessor();

	virtual void generate(FrameGraph& graph) override;
};



class SkyRender :public Events::prop_handler, public FrameGraphGenerator
{
	Render::Texture::ptr transmittance;
	Render::Texture::ptr irradiance;
	Render::Texture::ptr inscatter;
	
	bool processed = false;
public:
	using ptr = std::shared_ptr<SkyRender>;

	SkyRender();

	virtual void generate(FrameGraph& graph) override;

};
