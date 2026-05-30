module Graphics:SMAA;


import :FrameGraphContext;

import HAL;

#define STRINGIFY(x) #x
#define PPCAT_NX(A, B) A ## B

#define SMAA_TEXTURE(name) \
    EngineAsset<TextureAsset> name(PPCAT_NX(L,#name), [] { \
		return new TextureAsset(to_path(L"textures/" STRINGIFY(name) ".dds")); \
    })

namespace EngineAssets
{
	SMAA_TEXTURE(AreaTex);
	SMAA_TEXTURE(SearchTex);
}

SMAA::SMAA()
{
	area_tex   = EngineAssets::AreaTex.get_asset()->get_texture();
	search_tex = EngineAssets::SearchTex.get_asset()->get_texture();

	// ---- Pass function members ------------------------------------------------

	m_smaa_setup = [this](Passes::SMAA::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		auto& frame = builder.graph->get_context<ViewportInfo>();
		builder.need(data.ResultTexture, FrameGraph::ResourceFlags::RenderTarget);
		builder.create(data.SMAA_edges,
			{ ivec3(frame.frame_size, 0), HAL::Format::R8G8_UNORM, 1, 1 },
			FrameGraph::ResourceFlags::RenderTarget);
		builder.create(data.SMAA_blend,
			{ ivec3(frame.frame_size, 0), HAL::Format::R8G8B8A8_UNORM, 1, 1 },
			FrameGraph::ResourceFlags::RenderTarget);
		builder.recreate(data.ResultTextureNew, FrameGraph::ResourceFlags::RenderTarget);
		return true;
	};

	m_smaa_render = [this](Passes::SMAA::Context& data, FrameGraph::FrameContext& context)
	{
		auto& frame    = context.graph->get_context<ViewportInfo>();
		auto& graphics = context.get_list()->get_graphics();
		ivec2 size     = frame.frame_size;

		graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

		graphics.set_pipeline<PSOS::EdgeDetect>();
		{
			RT::SingleColor rt;
			rt.GetColor() = data.SMAA_edges->renderTarget;
			graphics.set_rtv(rt, HAL::RTOptions::Default | HAL::RTOptions::ClearAll);
		}
		{
			Slots::SMAA_Global slot_global;
			slot_global.GetColorTex()         = data.ResultTexture->texture2D;
			slot_global.GetSubsampleIndices() = float4(0, 0, 0, 0);
			slot_global.GetSMAA_RT_METRICS()  = float4(1.0f / size.x, 1.0f / size.y, size);
			graphics.set(slot_global);
		}
		graphics.draw(4);

		graphics.set_pipeline<PSOS::BlendWeight>();
		{
			RT::SingleColor rt;
			rt.GetColor() = data.SMAA_blend->renderTarget;
			graphics.set_rtv(rt, HAL::RTOptions::Default | HAL::RTOptions::ClearAll);
		}
		{
			Slots::SMAA_Weights slot_edges;
			slot_edges.GetSearchTex() = search_tex->texture_2d().texture2D;
			slot_edges.GetAreaTex()   = area_tex->texture_2d().texture2D;
			slot_edges.GetEdgesTex()  = data.SMAA_edges->texture2D;
			graphics.set(slot_edges);
		}
		graphics.draw(4);

		graphics.set_pipeline<PSOS::Blending>();
		{
			RT::SingleColor rt;
			rt.GetColor() = data.ResultTextureNew->renderTarget;
			graphics.set_rtv(rt);
		}
		{
			Slots::SMAA_Blend slot_blend;
			slot_blend.GetBlendTex() = data.SMAA_blend->texture2D;
			graphics.set(slot_blend);
		}
		graphics.draw(4);
	};
}
