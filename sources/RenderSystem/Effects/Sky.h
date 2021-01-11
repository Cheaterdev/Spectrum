#pragma once
#include "FrameGraph.h"
#include "DX12/Texture.h"
#include "Log/Events.h"


class CubeMapEnviromentProcessor : public Singleton<CubeMapEnviromentProcessor>, public FrameGraphGenerator
{
public:

	CubeMapEnviromentProcessor();

	virtual void generate(FrameGraph& graph) override;
};



class SkyRender :public Events::prop_handler, public FrameGraphGenerator
{
	DX12::Texture::ptr transmittance;
	DX12::Texture::ptr irradiance;
	DX12::Texture::ptr inscatter;
	
	bool processed = false;

	float3 dir = float3(0, 0, -1);
public:
	using ptr = std::shared_ptr<SkyRender>;

	SkyRender();

	virtual void generate(FrameGraph& graph) override;

};
