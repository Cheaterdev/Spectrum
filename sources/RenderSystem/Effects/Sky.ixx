export module Graphics:Sky;



import Core;
import HAL;
import FrameGraph;

export class CubeMapEnviromentProcessor : public Singleton<CubeMapEnviromentProcessor>, public FrameGraph::GraphGenerator
{
public:

	CubeMapEnviromentProcessor();

	virtual void generate(FrameGraph::Graph& graph) override;
};



export class SkyRender :public Events::prop_handler, public FrameGraph::GraphGenerator
{
	HAL::Texture::ptr transmittance;
	HAL::Texture::ptr irradiance;
	HAL::Texture::ptr inscatter;
	
	bool processed = false;

	float3 dir = float3(0, 0, -1);
public:
	using ptr = std::shared_ptr<SkyRender>;

	SkyRender();

	virtual void generate(FrameGraph::Graph& graph) override;
	 void generate_sky(FrameGraph::Graph& graph) ;

};
