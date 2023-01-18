module Graphics:SMAA;
import :FrameGraphContext;

import HAL;

using namespace FrameGraph;


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
	area_tex = EngineAssets::AreaTex.get_asset()->get_texture();
	search_tex = EngineAssets::SearchTex.get_asset()->get_texture();

}

void SMAA::generate(Graph& graph)
{

	struct SMAAData
	{
		Handlers::Texture H(ResultTexture);
		Handlers::Texture H(SMAA_edges);
		Handlers::Texture H(SMAA_blend);

		Handlers::Texture ResultTextureNew = ResultTexture;
	};

	graph.add_pass<SMAAData>("SMAA", [this, &graph](SMAAData& data, TaskBuilder& builder) {
		builder.need(data.ResultTexture, ResourceFlags::RenderTarget);

		auto& frame = graph.get_context<ViewportInfo>();
		builder.create(data.SMAA_edges, { ivec3(frame.frame_size, 0), HAL::Format::R8G8_UNORM,1 ,1 }, ResourceFlags::RenderTarget);
		builder.create(data.SMAA_blend, { ivec3(frame.frame_size, 0),  HAL::Format::R8G8B8A8_UNORM,1 ,1 }, ResourceFlags::RenderTarget);

		builder.recreate(data.ResultTextureNew, ResourceFlags::RenderTarget);

		}, [this, &graph](SMAAData& data, FrameContext& _context) {
			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();

			auto& frame = graph.get_context<ViewportInfo>();

			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

			ivec2 size = frame.frame_size;

			graphics.set_pipeline<PSOS::EdgeDetect>();


			{
				RT::SingleColor rt;
				rt.GetColor() = data.SMAA_edges->renderTarget;
				rt.set(graphics);
			}

			graphics.get_base().clear_rtv(data.SMAA_edges->renderTarget);
			graphics.get_base().clear_rtv(data.SMAA_blend->renderTarget);

			{
				Slots::SMAA_Global slot_global;
				slot_global.GetColorTex() = data.ResultTexture->texture2D;
				slot_global.GetSubsampleIndices() = float4(0, 0, 0, 0);
				slot_global.GetSMAA_RT_METRICS() = float4(1.0f / size.x, 1.0f / size.y, size);
				slot_global.set(graphics);
			}

			graphics.draw(4);
			
			graphics.set_pipeline<PSOS::BlendWeight>();

			{
				RT::SingleColor rt;
				rt.GetColor() = data.SMAA_blend->renderTarget;
				rt.set(graphics);
			}

			{
				Slots::SMAA_Weights slot_edges;
				slot_edges.GetSearchTex() = search_tex->texture_2d().texture2D;
				slot_edges.GetAreaTex() = area_tex->texture_2d().texture2D;
				slot_edges.GetEdgesTex() = data.SMAA_edges->texture2D;
				slot_edges.set(graphics);
			}

			graphics.draw(4);

			

			graphics.set_pipeline<PSOS::Blending>();

			{
				RT::SingleColor rt;
				rt.GetColor() = data.ResultTextureNew->renderTarget;
				rt.set(graphics);
			}
			{
				Slots::SMAA_Blend slot_blend;
				slot_blend.GetBlendTex() = data.SMAA_blend->texture2D;
				slot_blend.set(graphics);
			}

			graphics.draw(4);

		});
}

