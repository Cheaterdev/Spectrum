export module Graphics:ShadowDenoiser;

import Core;
import HAL;
import FrameGraph;

export class ShadowDenoiser :public Events::prop_handler, public FrameGraph::GraphGenerator
{
	
public:
	using ptr = std::shared_ptr<ShadowDenoiser>;

	ShadowDenoiser();

	virtual void generate(FrameGraph::Graph& graph) override;
};
