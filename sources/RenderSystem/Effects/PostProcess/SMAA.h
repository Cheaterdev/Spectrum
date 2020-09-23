#pragma once
class SMAA :Events::prop_handler, public FrameGraphGenerator
{
	Render::Texture::ptr area_tex;
	Render::Texture::ptr search_tex;
public:
	using ptr = std::shared_ptr<SMAA>;
	SMAA();
	virtual void generate(FrameGraph& graph) override;

};
