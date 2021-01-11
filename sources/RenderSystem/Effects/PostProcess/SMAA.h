#pragma once
#include "Log/Events.h"
#include "FrameGraph.h"
#include "DX12/Texture.h"
class SMAA :Events::prop_handler, public FrameGraphGenerator
{
	DX12::Texture::ptr area_tex;
	DX12::Texture::ptr search_tex;
public:
	using ptr = std::shared_ptr<SMAA>;
	SMAA();
	virtual void generate(FrameGraph& graph) override;

};
