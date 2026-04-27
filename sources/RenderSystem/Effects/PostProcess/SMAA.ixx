export module Graphics:SMAA;

import <RenderSystem.h>;
import FrameGraph;
import HAL;
import :FrameGraphContext;

export class SMAA
{
	HAL::Texture::ptr area_tex;
	HAL::Texture::ptr search_tex;

public:
	// Default constructor: loads the SMAA lookup textures from disk.
	// Defined in SMAA.cpp so the EngineAsset declarations stay out of the interface.
	SMAA();

	// Template constructor: wires setup/render funcs onto the smaa pass
	// of any pipeline that exposes a 'smaa' member of type Passes::SMAA.
	// Delegates to SMAA() first so the textures are always ready.
	template<typename TPipeline>
	explicit SMAA(TPipeline& pipeline) : SMAA()
	{
		pipeline.sMAA.setup_func = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
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

		pipeline.sMAA.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& frame = context.graph->get_context<ViewportInfo>();
			auto& graphics = context.get_list()->get_graphics();
			ivec2 size = frame.frame_size;

			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

			graphics.set_pipeline<PSOS::EdgeDetect>();
			{
				RT::SingleColor rt;
				rt.GetColor() = data.SMAA_edges->renderTarget;
				graphics.set_rtv(rt, HAL::RTOptions::Default | HAL::RTOptions::ClearAll);
			}
			{
				Slots::SMAA_Global slot_global;
				slot_global.GetColorTex()          = data.ResultTexture->texture2D;
				slot_global.GetSubsampleIndices()  = float4(0, 0, 0, 0);
				slot_global.GetSMAA_RT_METRICS()   = float4(1.0f / size.x, 1.0f / size.y, size);
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
};
