export module Graphics:NRDDenoiser;

import Core;
import HAL;
import FrameGraph;

export class NRDDenoiser :public Events::prop_handler, public FrameGraph::GraphGenerator
{
	
public:
	using ptr = std::shared_ptr<NRDDenoiser>;

	NRDDenoiser();

	virtual void generate(FrameGraph::Graph& graph) override;
};
