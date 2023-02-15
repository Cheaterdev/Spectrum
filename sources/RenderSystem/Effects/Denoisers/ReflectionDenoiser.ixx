export module Graphics:ReflectionDenoiser;

import Core;
import HAL;

export class ReflectionDenoiser :public Events::prop_handler, public FrameGraph::GraphGenerator
{
	
public:
	using ptr = std::shared_ptr<ReflectionDenoiser>;

	ReflectionDenoiser();

	virtual void generate(FrameGraph::Graph& graph) override;
};
