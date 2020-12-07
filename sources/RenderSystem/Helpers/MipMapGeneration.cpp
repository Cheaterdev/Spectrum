#include "pch.h"



MipMapGenerator::MipMapGenerator()
{


}



void MipMapGenerator::generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex)
{
	generate(compute_context, tex, tex->texture_2d());
}
void MipMapGenerator::generate_cube(Render::ComputeContext& compute_context, TextureView view)
{

	for (int i = 0; i <6; i++) 
		
		generate(compute_context,view.create_2d_slice(i, *compute_context.get_base().frame_resources));
}

void MipMapGenerator::generate(Render::ComputeContext& compute_context, TextureView  view)
{
	//return;
	PROFILE_GPU(L"downsampling");

	compute_context.set_signature(get_Signature(Layouts::DefaultLayout));
	uint32_t maps = view.get_mip_count()-1;
	auto size = view.get_size();
	uint32_t prev = 0;

	for (uint32_t TopMip = 0; TopMip < maps;)
	{
		
		uint32_t SrcWidth = uint32_t(size.x >> TopMip);
		uint32_t SrcHeight = uint32_t(size.y >> TopMip);
		uint32_t DstWidth = SrcWidth >> 1;
		uint32_t DstHeight = SrcHeight >> 1;
		uint32_t NonPowerOfTwo = (SrcWidth & 1) | (SrcHeight & 1) << 1;


		compute_context.set_pipeline(GetPSO<PSOS::MipMapping>(
			PSOS::MipMapping::NonPowerOfTwo(NonPowerOfTwo)
			| PSOS::MipMapping::Gamma.Use(DirectX::IsSRGB(view.get_desc().Format))
		));

		uint32_t AdditionalMips;
		_BitScanForward((unsigned long*)&AdditionalMips, DstWidth | DstHeight);
		uint32_t NumMips = 1 +(AdditionalMips > 3 ? 3 : AdditionalMips);

		if (TopMip + NumMips > maps)
			NumMips = maps - TopMip;

		// These are clamped to 1 after computing additional mips because clamped
		// dimensions should not limit us from downsampling multiple times.  (E.g.
		// 16x1 -> 8x1 -> 4x1 -> 2x1 -> 1x1.)
		if (DstWidth == 0)
			DstWidth = 1;

		if (DstHeight == 0)
			DstHeight = 1;

		Slots::MipMapping data;
		data.GetSrcMipLevel() = TopMip;
		data.GetNumMipLevels() = NumMips;
		data.GetTexelSize() = { 1.0f / DstWidth, 1.0f / DstHeight };

		for (uint32_t i = 0; i < NumMips; i++)
		{
			data.GetOutMip()[i] = Render::HLSL::RWTexture2D<float4>(view.create_mip(TopMip + 1 + i, *compute_context.get_base().frame_resources).get_uav());
		}
		data.GetSrcMip() = view.create_mip(TopMip, *compute_context.get_base().frame_resources).texture2D;

		data.set(compute_context, true);

		compute_context.dispach(ivec2(DstWidth, DstHeight), ivec2(8, 8));

		compute_context.get_base().transition_uav(view.resource.get());
		prev = TopMip;
		TopMip += NumMips;

	}
}

void MipMapGenerator::generate(Render::ComputeContext& compute_context, Render::Texture::ptr tex, Texture2DView::ptr view)
{

	compute_context.set_signature(get_Signature(Layouts::DefaultLayout));

	uint32_t maps = tex->get_desc().MipLevels - 1;
	uint32_t prev = 0;

	for (uint32_t TopMip = 0; TopMip < maps;)
	{
		uint32_t SrcWidth = uint32_t(tex->get_desc().Width >> TopMip);
		uint32_t SrcHeight = uint32_t(tex->get_desc().Height >> TopMip);
		uint32_t DstWidth = SrcWidth >> 1;
		uint32_t DstHeight = SrcHeight >> 1;
		uint32_t NonPowerOfTwo = (SrcWidth & 1) | (SrcHeight & 1) << 1;



		compute_context.set_pipeline(GetPSO<PSOS::MipMapping>(
			PSOS::MipMapping::NonPowerOfTwo(NonPowerOfTwo)
			| PSOS::MipMapping::Gamma.Use(DirectX::IsSRGB(tex->get_desc().Format))
			));


		uint32_t AdditionalMips;
		_BitScanForward((unsigned long*)& AdditionalMips, DstWidth | DstHeight);
		uint32_t NumMips = 1 + (AdditionalMips > 3 ? 3 : AdditionalMips);

		if (TopMip + NumMips > maps)
			NumMips = maps - TopMip;

		// These are clamped to 1 after computing additional mips because clamped
		// dimensions should not limit us from downsampling multiple times.  (E.g.
		// 16x1 -> 8x1 -> 4x1 -> 2x1 -> 1x1.)
		if (DstWidth == 0)
			DstWidth = 1;

		if (DstHeight == 0) 
			DstHeight = 1;

		Slots::MipMapping data;
		data.GetSrcMipLevel() = TopMip;
		data.GetNumMipLevels() = NumMips;
		data.GetTexelSize() = { 1.0f / DstWidth, 1.0f / DstHeight };

		for (uint32_t i = 0; i < NumMips; i++)
		{
			data.GetOutMip()[i] = view->rwTexture2D[TopMip + 1 + i];
		}
		data.GetSrcMip() = view->texture2DMips[TopMip];

		data.set(compute_context, true);

		compute_context.dispach(ivec2(DstWidth, DstHeight), ivec2(8, 8));

		prev = TopMip;
		TopMip += NumMips;
	}
}

void MipMapGenerator::downsample_depth(Render::ComputeContext& compute_context, Render::Texture::ptr tex, Render::Texture::ptr& to)
{
	compute_context.set_pipeline(GetPSO<PSOS::DownsampleDepth>());

	Slots::DownsampleDepth data;
	data.GetSrcTex() = tex->texture_2d()->texture2D;
	data.GetTargetTex() = to->texture_2d()->rwTexture2D[0];
	data.set(compute_context);
	compute_context.dispach(ivec2(tex->get_desc().Width, tex->get_desc().Height), ivec2(8, 8));

}

void MipMapGenerator::downsample_depth(Render::ComputeContext& compute_context, Render::TextureView& tex, Render::TextureView& to){
	compute_context.set_pipeline(GetPSO<PSOS::DownsampleDepth>());


	Slots::DownsampleDepth data;
	data.GetSrcTex() = tex.texture2D;
	data.GetTargetTex() = to.rwTexture2D;
	data.set(compute_context);
	compute_context.dispach(ivec2(tex.get_size()), ivec2(8, 8));

}

void MipMapGenerator::generate_quality(Render::GraphicsContext& list, camera* cam, GBuffer& buffer, TextureView tempColor)
{

	PROFILE_GPU(L"generate_quality");
	list.set_signature(get_Signature(Layouts::DefaultLayout));

	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.set_viewport(tempColor.get_viewport());
	list.set_scissor(tempColor.get_scissor());


	{
		{
			Slots::GBuffer gbuffer;
		buffer.SetTable(gbuffer);
		gbuffer.set(list);
		}
		list.set_pipeline(GetPSO<PSOS::QualityColor>());
		list.set_rtv(1, tempColor.get_rtv(),Handle());
		list.draw(4);
	}


	
	{

		Slots::GBufferQuality quality;
		quality.GetRef() = tempColor.texture2D;
		quality.set(list);

		list.get_base().clear_stencil(buffer.quality.get_dsv());
		list.set_pipeline(GetPSO<PSOS::QualityToStencil>());

		list.set_rtv(0, Handle(), buffer.quality.get_dsv());

		list.get_native_list()->OMSetStencilRef(1);
		list.draw(4);

		list.set_pipeline(GetPSO<PSOS::QualityToStencilREfl>());
		list.get_native_list()->OMSetStencilRef(2);
		list.draw(4);
	}
}


void MipMapGenerator::copy_texture_2d_slow(Render::GraphicsContext& list, Render::Texture::ptr to, Render::Texture::ptr from)
{
	list.set_pipeline(GetPSO<PSOS::CopyTexture>(PSOS::CopyTexture::Format(to->texture_2d()->get_rtv().resource_info->rtv.Format)));

	auto& view = to->texture_2d();

	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.set_viewport(view->get_viewport());
	list.set_scissor(view->get_scissor());


	Slots::CopyTexture data;
	data.GetSrcTex()  = from->texture_2d()->texture2D;
	data.set(list);

	list.set_rtv(1, view->get_rtv(), Handle());	
	list.draw(4);
}


void MipMapGenerator::copy_texture_2d_slow(Render::GraphicsContext& list, Render::Texture::ptr to, Render::TextureView from)
{
	list.set_pipeline(GetPSO<PSOS::CopyTexture>(PSOS::CopyTexture::Format(to->texture_2d()->get_rtv().resource_info->rtv.Format)));


	auto& view = to->texture_2d();

	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.set_viewport(view->get_viewport());
	list.set_scissor(view->get_scissor());

	Slots::CopyTexture data;
	data.GetSrcTex() = from.texture2D;
	data.set(list);

	list.set_rtv(1, view->get_rtv(), Handle());	
	list.draw(4);
}



void MipMapGenerator::render_texture_2d_slow(Render::GraphicsContext& list, Render::TextureView to, Render::TextureView from)
{
	list.set_pipeline(GetPSO<PSOS::CopyTexture>(PSOS::CopyTexture::Format(to.get_rtv().resource_info->rtv.Format)));
	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Slots::CopyTexture data;
	data.GetSrcTex() = from.texture2D;
	data.set(list);

	list.set_rtv(1,to.get_rtv(), Handle());
	list.draw(4);
}



void MipMapGenerator::write_to_depth(Render::GraphicsContext& list, Render::TextureView from, Render::TextureView to)
{
	list.set_pipeline(GetPSO<PSOS::RenderToDS>());
	Slots::CopyTexture data;
	data.GetSrcTex() = from.texture2D;
	data.set(list);

	list.set_viewport(to.get_viewport());
	list.set_scissor(to.get_scissor());

	list.set_rtv(0, Handle(), to.get_dsv());

	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	list.draw(4);
	list.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}