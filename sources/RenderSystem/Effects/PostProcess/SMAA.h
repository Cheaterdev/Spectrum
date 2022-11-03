#pragma once
#include "FrameGraph/FrameGraph.h"
import Events;
import HAL;

class SMAA :Events::prop_handler, public FrameGraph::GraphGenerator
{
	HAL::Texture::ptr area_tex;
	HAL::Texture::ptr search_tex;
public:
	using ptr = std::shared_ptr<SMAA>;
	SMAA();
	virtual void generate(FrameGraph::Graph& graph) override;

};
