module Graphics:MipMapGenerator;


import Core;
import HAL;

using namespace HAL;

//void MipMapGenerator::generate(HAL::ComputeContext& compute_context, HAL::Texture::ptr tex)
//{
//	generate(compute_context, tex->texture_2d());
//}

// Cube mip chain in one dispatch per 4-mip batch (the shader's structural
// limit), instead of one batch per face: every mip view spans all six slices
// and the dispatch carries the face in Z.
void MipMapGenerator::generate_cube(HAL::ComputeContext& compute_context, HAL::CubeView view)
{
	PROFILE(L"MipMapGenerator_cube");

	compute_context.set_signature(Layouts::DefaultLayout);

	auto& desc = view.get_desc().as_texture();

	const uint32_t slices = desc.ArraySize;
	const uint32_t maps = desc.MipLevels - 1;
	const auto size = view.get_size();

	auto slice_span = [&](uint32_t mip)
	{
		HAL::TextureViewDesc vdesc;
		vdesc.MipSlice = mip;
		vdesc.MipLevels = 1;
		vdesc.FirstArraySlice = 0;
		vdesc.ArraySize = slices;

		return view.resource->create_view<HAL::Texture2DView>(compute_context.get_base(), vdesc);
	};

	for (uint32_t TopMip = 0; TopMip < maps;)
	{
		uint32_t SrcWidth = uint32_t(size.x >> TopMip);
		uint32_t SrcHeight = uint32_t(size.y >> TopMip);
		uint32_t DstWidth = SrcWidth >> 1;
		uint32_t DstHeight = SrcHeight >> 1;
		uint32_t NonPowerOfTwo = (SrcWidth & 1) | (SrcHeight & 1) << 1;

		{
			PROFILE(L"set_pipeline");

			compute_context.set_pipeline<PSOS::MipMapping>(
				PSOS::MipMapping::NonPowerOfTwo(NonPowerOfTwo)
				| PSOS::MipMapping::Gamma.Use(desc.Format.is_srgb())
				| PSOS::MipMapping::Slices.Use(true)
				);
		}

		uint32_t AdditionalMips;
		_BitScanForward((unsigned long*)&AdditionalMips, DstWidth | DstHeight);
		uint32_t NumMips = 1 + (AdditionalMips > 3 ? 3 : AdditionalMips);

		if (TopMip + NumMips > maps)
			NumMips = maps - TopMip;

		if (DstWidth == 0)
			DstWidth = 1;

		if (DstHeight == 0)
			DstHeight = 1;

		// Value-initialised: the 2D fields and the unused array mips stay zero.
		Slots::MipMapping data{};
		data.GetSrcMipLevel() = TopMip;
		data.GetNumMipLevels() = NumMips;
		data.GetTexelSize() = { 1.0f / DstWidth, 1.0f / DstHeight };
		{
			PROFILE(L"create_mip");
			for (uint32_t i = 0; i < NumMips; i++)
			{
				data.GetOutMipArray()[i] = slice_span(TopMip + 1 + i).rwTexture2DArray;
			}
			data.GetSrcMipArray() = slice_span(TopMip).texture2DArray;
		}
		compute_context.set(data);

		compute_context.dispatch(ivec3(DstWidth, DstHeight, slices), ivec3(8, 8, 1));

		TopMip += NumMips;
	}
}

void MipMapGenerator::generate(HAL::ComputeContext& compute_context, HAL::Texture2DView  view)
{
	//return;
	PROFILE(L"MipMapGenerator");

	compute_context.set_signature(Layouts::DefaultLayout);
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

		{
			PROFILE(L"set_pipeline");

			compute_context.set_pipeline<PSOS::MipMapping>(
				PSOS::MipMapping::NonPowerOfTwo(NonPowerOfTwo)
				| PSOS::MipMapping::Gamma.Use(view.get_desc().as_texture().Format.is_srgb())
				);
		}
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
		{
			PROFILE(L"create_mip");
			for (uint32_t i = 0; i < NumMips; i++)
			{
				data.GetOutMip()[i] = view.create_mip(TopMip + 1 + i, compute_context.get_base()).rwTexture2D;
			}
			data.GetSrcMip() = view.create_mip(TopMip, compute_context.get_base()).texture2D;
		}
		compute_context.set(data);

		compute_context.dispatch(ivec2(DstWidth, DstHeight), ivec2(8, 8));

		//compute_context.get_base().transition_uav(view.resource.get());
		prev = TopMip;
		TopMip += NumMips;

	}
}


void MipMapGenerator::build_hiz_pyramid(HAL::ComputeContext& compute_context, HAL::Texture2DView view)
{
	PROFILE(L"build_hiz_pyramid");

	compute_context.set_signature(Layouts::DefaultLayout);
	compute_context.set_pipeline<PSOS::DownsampleDepthMip>();

	uint32_t maps = view.get_mip_count() - 1;
	auto size = view.get_size();

	// One dispatch per mip -- shallow enough (~6-7 mips) not to need
	// generate()'s 4-mips-per-dispatch batching.
	for (uint32_t mip = 0; mip < maps; mip++)
	{
		uint32_t DstWidth = uint32_t(size.x >> (mip + 1));
		uint32_t DstHeight = uint32_t(size.y >> (mip + 1));

		if (DstWidth == 0)
			DstWidth = 1;

		if (DstHeight == 0)
			DstHeight = 1;

		Slots::DownsampleDepthMip data;
		data.GetSrcMip() = view.create_mip(mip, compute_context.get_base()).texture2D;
		data.GetDstMip() = view.create_mip(mip + 1, compute_context.get_base()).rwTexture2D;
		compute_context.set(data);

		compute_context.dispatch(ivec2(DstWidth, DstHeight), ivec2(8, 8));
	}
}


void MipMapGenerator::downsample_depth(HAL::ComputeContext& compute_context, HAL::Texture::ptr tex, HAL::Texture::ptr& to)
{
	compute_context.set_pipeline<PSOS::DownsampleDepth>();

	Slots::DownsampleDepth data;
	data.GetSrcTex() = tex->texture_2d().texture2D;
	data.GetTargetTex() = to->texture_2d().rwTexture2D;
	compute_context.set(data);
	compute_context.dispatch(ivec2(tex->get_desc().as_texture().Dimensions.x, tex->get_desc().as_texture().Dimensions.y), ivec2(8, 8));

}

void MipMapGenerator::downsample_depth(HAL::ComputeContext& compute_context, HAL::Texture2DView& tex, HAL::Texture2DView& to) {
	compute_context.set_pipeline<PSOS::DownsampleDepth>();


	Slots::DownsampleDepth data;
	data.GetSrcTex() = tex.texture2D;
	data.GetTargetTex() = to.rwTexture2D;
compute_context.set(data);
	compute_context.dispatch(ivec2(tex.get_size()), ivec2(8, 8));

}

void MipMapGenerator::generate_quality(HAL::GraphicsContext& list, camera* cam, GBuffer& buffer,
	HAL::Texture2DView tempColor)
{

	PROFILE_GPU(L"generate_quality");
	list.set_signature(Layouts::DefaultLayout);

	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
	list.set_viewport(tempColor.get_viewport());
	list.set_scissor(tempColor.get_scissor());


	{
		{
			Slots::GBuffer gbuffer;
			buffer.SetTable(gbuffer);
			list.set(gbuffer);
		}
		list.set_pipeline<PSOS::QualityColor>();

		{
			RT::SingleColor rt;
			rt.GetColor() = tempColor.renderTarget;
			list.set_rtv(rt);
		}

		list.draw(4);
	}



	{

		Slots::GBufferQuality quality;
		quality.GetRef() = tempColor.texture2D;
		list.set(quality);

		list.set_pipeline<PSOS::QualityToStencil>();


		{
			RT::DepthOnly rt;
			rt.GetDepth() = buffer.quality.depthStencil;
			list.set_rtv(rt,RTOptions::Default| RTOptions::ClearAll);
		}


		list.set_stencil_ref(1);
		list.draw(4);

		list.set_pipeline<PSOS::QualityToStencilREfl>();
		list.set_stencil_ref(2);
		list.draw(4);
	}
}



void MipMapGenerator::copy_texture_2d_slow(HAL::GraphicsContext& list, HAL::Texture::ptr to, HAL::Texture2DView from)
{
	auto hal_view = std::get<HAL::Views::RenderTarget>(to->texture_2d().renderTarget.get_resource_info().view);
	list.set_pipeline<PSOS::CopyTexture>(PSOS::CopyTexture::Format(hal_view.Format));


	auto& view = to->texture_2d();

	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
	list.set_viewport(view.get_viewport());
	list.set_scissor(view.get_scissor());

	Slots::CopyTexture data;
	data.GetSrcTex() = from.texture2D;
	list.set(data);
	{
		RT::SingleColor rt;
		rt.GetColor() = view.renderTarget;
		list.set_rtv(rt);
	}

	list.draw(4);
}



void MipMapGenerator::render_texture_2d_slow(HAL::GraphicsContext& list, HAL::Texture2DView to, HAL::Texture2DView from)
{
	auto hal_view = std::get<HAL::Views::RenderTarget>(to.renderTarget.get_resource_info().view);
	list.set_pipeline<PSOS::CopyTexture>(PSOS::CopyTexture::Format(hal_view.Format));
	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

	Slots::CopyTexture data;
	data.GetSrcTex() = from.texture2D;
list.set(data);
	{
		RT::SingleColor rt;
		rt.GetColor() = to.renderTarget;
		list.set_rtv(rt);
	}
	list.draw(4);
}



void MipMapGenerator::write_to_depth(HAL::GraphicsContext& list, HAL::Texture2DView from, HAL::Texture2DView to)
{
	list.set_pipeline<PSOS::RenderToDS>();
	Slots::CopyTexture data;
	data.GetSrcTex() = from.texture2D;
list.set(data);

	list.set_viewport(to.get_viewport());
	list.set_scissor(to.get_scissor());

	{
		RT::DepthOnly rt;
		rt.GetDepth() = to.depthStencil;
		list.set_rtv(rt);
	}

	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
	list.draw(4);
	list.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
}