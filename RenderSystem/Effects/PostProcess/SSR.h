#pragma once

class ReflectionEffect
{
	Render::ComputePipelineState::ptr state;

public:

	ReflectionEffect();


	void process(MeshRenderContext::ptr& context, G_Buffer& buffer, Render::Texture::ptr cubemap, Render::Texture::ptr ao);
};



class ReflectionEffectPixel :public Events::prop_handler
{
	Render::PipelineState::ptr state;

	Render::PipelineState::ptr state_dilate;


	Render::PipelineState::ptr state_last;

	G_Buffer* buffer;
public:
	Render::Texture::ptr reflect_tex;
	Render::Texture::ptr reflect_tex_dilated;


	ReflectionEffectPixel(G_Buffer& buffer);


	void process(MeshRenderContext::ptr& context, G_Buffer& buffer, Render::CubemapArrayView::ptr cubemap);
};

