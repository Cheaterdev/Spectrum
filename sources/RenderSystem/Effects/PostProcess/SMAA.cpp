#include "pch_render.h"
#include "SMAA.h"
import HAL;

using namespace FrameGraph;

SMAA::SMAA()
{
	area_tex = HAL::Texture::get_resource({ to_path(L"textures\\AreaTex.dds"), false, false });
	search_tex = HAL::Texture::get_resource({ to_path(L"textures\\SearchTex.dds"), false, false });

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

		builder.create(data.SMAA_edges, { ivec3(graph.frame_size, 0), HAL::Format::R8G8_UNORM,1 ,1 }, ResourceFlags::RenderTarget);
		builder.create(data.SMAA_blend, { ivec3(graph.frame_size, 0),  HAL::Format::R8G8B8A8_UNORM,1 ,1 }, ResourceFlags::RenderTarget);

		builder.recreate(data.ResultTextureNew, ResourceFlags::RenderTarget);

		}, [this, &graph](SMAAData& data, FrameContext& _context) {
			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();


			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

			ivec2 size = graph.frame_size;

			graphics.set_pipeline(GetPSO<PSOS::EdgeDetect>());

			graphics.set_viewport(data.SMAA_edges->get_viewport());
			graphics.set_scissor(data.SMAA_edges->get_scissor());
			graphics.set_rtv(1, data.SMAA_edges->renderTarget, HAL::Handle());
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

			{

				Slots::SMAA_Weights slot_edges;
				slot_edges.GetSearchTex() = search_tex->texture_2d().texture2D;
				slot_edges.GetAreaTex() = area_tex->texture_2d().texture2D;
				slot_edges.GetEdgesTex() = data.SMAA_edges->texture2D;

				slot_edges.set(graphics);
			}
			graphics.set_pipeline(GetPSO<PSOS::BlendWeight>());
			graphics.set_rtv(1, data.SMAA_blend->renderTarget, HAL::Handle());
			graphics.draw(4);



			{

				Slots::SMAA_Blend slot_blend;

				slot_blend.GetBlendTex() = data.SMAA_blend->texture2D;

				slot_blend.set(graphics);
			}

			graphics.set_pipeline(GetPSO<PSOS::Blending>());
			graphics.set_rtv(1, data.ResultTextureNew->renderTarget, HAL::Handle());
			graphics.draw(4);

		});

	//resolving
	/*    {
	list.set_pipeline(state_resolve);
	buffer->result_tex.swap(context->list, HAL::ResourceState::RENDER_TARGET, HAL::ResourceState::PIXEL_SHADER_RESOURCE);
	list.set(2, buffer->result_tex.second()->texture_2d().get_srv());
	temporal.set(context->list, 3);
	list.set_rtv(1, buffer->result_tex.first()->texture_2d().get_rtv(0), HAL::Handle());
	list.draw(4);
	}*/
	//	list.transition(buffer->light_tex.get(), HAL::ResourceState::PIXEL_SHADER_RESOURCE);
}

