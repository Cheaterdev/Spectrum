#pragma once
#include "FrameGraph/FrameGraph.h"

class SMAA :Events::prop_handler, public FrameGraph::GraphGenerator
{
	Render::Texture::ptr area_tex;
	Render::Texture::ptr search_tex;
public:
	using ptr = std::shared_ptr<SMAA>;
	SMAA();
	virtual void generate(FrameGraph::Graph& graph) override;

};
