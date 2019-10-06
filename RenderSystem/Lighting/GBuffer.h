#pragma once

template<class T>
class Swapper
{
	int current = 0;

protected:
	T data[2];

public:
	virtual ~Swapper()
	{
	}

	Swapper()
	{
	}
	virtual void reset()
	{
		current = 0;
	}
	T& first()
	{
		return data[current];
	}

	T& second()
	{
		return data[(current + 1) % 2];
	}


	void swap()
	{
		current = (current + 1) % 2;
	}
};

class TextureSwapper : public Swapper<Render::Texture::ptr>
{
public:

	TextureSwapper();

	void resize(DXGI_FORMAT format, ivec2 size);


	void reset(Render::CommandList::ptr& list);

	void swap(Render::CommandList::ptr& list, Render::ResourceState state1 = Render::ResourceState::RENDER_TARGET, Render::ResourceState state2 = Render::ResourceState::PIXEL_SHADER_RESOURCE);
};
class G_Buffer:public Events::prop_handler
{
	//	Render::Texture::ptr specular_tex;
	std::vector<Render::Viewport> vps;
	std::vector<sizer_long> scissors;
	bool first = true;

	void resize(ivec2 size);
public:
	Render::HandleTable srv_table;
	Render::Texture::ptr albedo_tex;
	Render::Texture::ptr normal_tex;
	Render::Texture::ptr specular_tex;
	Render::Texture::ptr speed_tex;

	//   Render::Texture::ptr light_tex;
	//     Render::Texture::ptr light_tex2;

	Render::Texture::ptr depth_tex;
	Render::Texture::ptr depth_tex_downscaled;
	Render::Texture::ptr depth_tex_downscaled_uav;

	RenderTargetTable table;
	RenderTargetTable table_downscaled_depth;

	//RenderTargetTable table_light;
	Events::prop<ivec2> size;

	Render::Texture::ptr depth_tex_mips_pow2;



	Render::Texture::ptr depth_tex_mips;

	Render::Texture::ptr depth_tex_mips_prev;

	TextureSwapper result_tex;

	Render::Texture::ptr quality_color;
	Render::Texture::ptr quality_stencil;
	RenderTargetTable quality_table;
	RenderTargetTable quality_color_table;

	Render::PipelineState::ptr render_depth_state;

	G_Buffer();
	void init(Render::RootSignature::ptr sig);
	
	void set_lights(MeshRenderContext::ptr& context);


	void set_downsapled(MeshRenderContext::ptr& context);
	void set_downsapled(Render::CommandList::ptr& list);
	void set_original(MeshRenderContext::ptr& context);

	void reset(MeshRenderContext::ptr& context);
	void set(MeshRenderContext::ptr& context, bool copy_prev = false);
	void downsample_depth(Render::CommandList::ptr& list);
	void end(MeshRenderContext::ptr& context);

	void generate_quality(MeshRenderContext::ptr& context);
};