module Graphics;
import Profiling;
import HAL;

using namespace HAL;

//void MipMapGenerator::generate(HAL::ComputeContext& compute_context, HAL::Texture::ptr tex)
//{
//	generate(compute_context, tex->texture_2d());
//}

void MipMapGenerator::generate_cube(HAL::ComputeContext& compute_context, HAL::CubeView view)
{
	for (int i = 0; i < 6; i++)
		generate(compute_context, view.get_face(i));
}

void MipMapGenerator::generate(HAL::ComputeContext& compute_context, HAL::TextureView  view)
{
	//return;
	PROFILE(L"MipMapGenerator");

	compute_context.set_signature(get_Signature(Layouts::DefaultLayout));
	uint32_t maps = view.get_mip_count() - 1;
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
			| PSOS::MipMapping::Gamma.Use(view.get_desc().as_texture().Format.is_srgb())
			));

		uint32_t AdditionalMips;
		_BitScanForward((unsigned long*)&AdditionalMips, DstWidth | DstHeight);
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
			data.GetOutMip()[i] = view.create_mip(TopMip + 1 + i, *compute_context.get_base().frame_resources).rwTexture2D;
		}
		data.GetSrcMip() = view.create_mip(TopMip, *compute_context.get_base().frame_resources).texture2D;

		data.set(compute_context);

		compute_context.dispach(ivec2(DstWidth, DstHeight), ivec2(8, 8));

		//compute_context.get_base().transition_uav(view.resource.get());
		prev = TopMip;
		TopMip += NumMips;

	}
}


void MipMapGenerator::downsample_depth(HAL::ComputeContext& compute_context, HAL::Texture::ptr tex, HAL::Texture::ptr& to)
{
	compute_context.set_pipeline(GetPSO<PSOS::DownsampleDepth>());

	Slots::DownsampleDepth data;
	data.GetSrcTex() = tex->texture_2d().texture2D;
	data.GetTargetTex() = to->texture_2d().rwTexture2D;
	data.set(compute_context);
	compute_context.dispach(ivec2(tex->get_desc().as_texture().Dimensions.x, tex->get_desc().as_texture().Dimensions.y), ivec2(8, 8));

}

void MipMapGenerator::downsample_depth(HAL::ComputeContext& compute_context, HAL::TextureView& tex, HAL::TextureView& to) {
	compute_context.set_pipeline(GetPSO<PSOS::DownsampleDepth>());


	Slots::DownsampleDepth data;
	data.GetSrcTex() = tex.texture2D;
	data.GetTargetTex() = to.rwTexture2D;
	data.set(compute_context);
	compute_context.dispach(ivec2(tex.get_size()), ivec2(8, 8));

}

void MipMapGenerator::generate_quality(HAL::GraphicsContext& list, camera* cam, GBuffer& buffer,
	HAL::TextureView tempColor)
{

	PROFILE_GPU(L"generate_quality");
	list.set_signature(get_Signature(Layouts::DefaultLayout));

	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
	list.set_viewport(tempColor.get_viewport());
	list.set_scissor(tempColor.get_scissor());


	{
		{
			Slots::GBuffer gbuffer;
			buffer.SetTable(gbuffer);
			gbuffer.set(list);
		}
		list.set_pipeline(GetPSO<PSOS::QualityColor>());
		list.set_rtv(1, tempColor.renderTarget, HAL::Handle());
		list.draw(4);
	}



	{

		Slots::GBufferQuality quality;
		quality.GetRef() = tempColor.texture2D;
		quality.set(list);

		list.get_base().clear_stencil(buffer.quality.depthStencil);
		list.set_pipeline(GetPSO<PSOS::QualityToStencil>());

		list.set_rtv(0, HAL::Handle(), buffer.quality.depthStencil);

		list.set_stencil_ref(1);
		list.draw(4);

		list.set_pipeline(GetPSO<PSOS::QualityToStencilREfl>());
		list.set_stencil_ref(2);
		list.draw(4);
	}
}
//
//void MipMapGenerator::copy_texture_2d_slow(HAL::GraphicsContext& list, HAL::Texture::ptr to, HAL::Texture::ptr from)
//{
//	auto hal_view = std::get<HAL::Views::RenderTarget>(to->texture_2d().renderTarget.get_resource_info()->view);
//	list.set_pipeline(GetPSO<PSOS::CopyTexture>(PSOS::CopyTexture::Format(hal_view.Format)));
//
//	auto& view = to->texture_2d();
//
//	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
//	list.set_viewport(view.get_viewport());
//	list.set_scissor(view.get_scissor());
//
//
//	Slots::CopyTexture data;
//	data.GetSrcTex() = from->texture_2d().texture2D;
//	data.set(list);
//
//	list.set_rtv(1, view.renderTarget, HAL::Handle());
//	list.draw(4);
//}


void MipMapGenerator::copy_texture_2d_slow(HAL::GraphicsContext& list, HAL::Texture::ptr to, HAL::TextureView from)
{
	auto hal_view = std::get<HAL::Views::RenderTarget>(to->texture_2d().renderTarget.get_resource_info()->view);
	list.set_pipeline(GetPSO<PSOS::CopyTexture>(PSOS::CopyTexture::Format(hal_view.Format)));


	auto& view = to->texture_2d();

	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
	list.set_viewport(view.get_viewport());
	list.set_scissor(view.get_scissor());

	Slots::CopyTexture data;
	data.GetSrcTex() = from.texture2D;
	data.set(list);

	list.set_rtv(1, view.renderTarget, HAL::Handle());
	list.draw(4);
}



void MipMapGenerator::render_texture_2d_slow(HAL::GraphicsContext& list, HAL::TextureView to, HAL::TextureView from)
{
	auto hal_view = std::get<HAL::Views::RenderTarget>(to.renderTarget.get_resource_info()->view);
	list.set_pipeline(GetPSO<PSOS::CopyTexture>(PSOS::CopyTexture::Format(hal_view.Format)));
	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

	Slots::CopyTexture data;
	data.GetSrcTex() = from.texture2D;
	data.set(list);

	list.set_rtv(1, to.renderTarget, HAL::Handle());
	list.draw(4);
}



void MipMapGenerator::write_to_depth(HAL::GraphicsContext& list, HAL::TextureView from, HAL::TextureView to)
{
	list.set_pipeline(GetPSO<PSOS::RenderToDS>());
	Slots::CopyTexture data;
	data.GetSrcTex() = from.texture2D;
	data.set(list);

	list.set_viewport(to.get_viewport());
	list.set_scissor(to.get_scissor());

	list.set_rtv(0, HAL::Handle(), to.depthStencil);

	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
	list.draw(4);
	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
}