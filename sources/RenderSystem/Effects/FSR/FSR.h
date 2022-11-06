#pragma once
import Graphics;

import Events;

class FSR :public Events::prop_handler, public FrameGraph::GraphGenerator
{
public:
	using ptr = std::shared_ptr<FSR>;
	virtual void generate(FrameGraph::Graph& graph) override;
};
