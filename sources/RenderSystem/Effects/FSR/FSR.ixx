export module Graphics:FSR;

import HAL;

import Events;

export class FSR :public Events::prop_handler, public FrameGraph::GraphGenerator
{
public:
	using ptr = std::shared_ptr<FSR>;
	virtual void generate(FrameGraph::Graph& graph) override;
};
