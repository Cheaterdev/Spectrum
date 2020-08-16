#pragma once

class SSGI :public Events::prop_handler
{
	Render::PipelineState::ptr state;
	Render::PipelineState::ptr state_last;

	G_Buffer* buffer;
	PlacedAllocator allocator;

	TextureSwapper textures;
public:
	Render::Texture::ptr reflect_tex;

	using ptr = std::shared_ptr<SSGI>;
	SSGI(G_Buffer& buffer);
	
	
	void process(MeshRenderContext::ptr& context, G_Buffer& buffer, CubemapArrayView::ptr &cube, bool clear = false, int repeat_count=1);
};

