#include "pch.h"


SMAA::SMAA()
{
	area_tex = Render::Texture::get_resource({ "textures\\AreaTex.dds", false, false });
	search_tex = Render::Texture::get_resource({ "textures\\SearchTex.dds", false, false });

}

void SMAA::generate(FrameGraph& graph)
{

	struct SkyData
	{
		ResourceHandler* source_tex;

		ResourceHandler* target_tex;


		ResourceHandler* edges;
		ResourceHandler* blend;


	};

	graph.add_pass<SkyData>("Sky", [this, &graph](SkyData& data, TaskBuilder& builder) {
		data.source_tex = builder.need_texture("ResultTexture", ResourceFlags::RenderTarget);
		data.edges = builder.create_texture("SMAA_edges", graph.frame_size, 1, DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM, ResourceFlags::RenderTarget);
		data.blend = builder.create_texture("SMAA_blend", graph.frame_size, 1, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, ResourceFlags::RenderTarget);

		data.target_tex = builder.recreate_texture("ResultTexture", ResourceFlags::RenderTarget);

		}, [this, &graph](SkyData& data, FrameContext& _context) {
			auto edges = _context.get_texture(data.edges);
			auto blend = _context.get_texture(data.blend);
			auto target_tex = _context.get_texture(data.target_tex);
			auto source_tex = _context.get_texture(data.source_tex);

			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();


			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			ivec2 size = graph.frame_size;

			graphics.set_pipeline(GetPSO<PSOS::EdgeDetect>());

			graphics.set_viewport(edges.get_viewport());
			graphics.set_scissor(edges.get_scissor());
			graphics.set_rtv(1, edges.get_rtv(), Render::Handle());
			graphics.get_base().clear_rtv(edges.get_rtv());
			graphics.get_base().clear_rtv(blend.get_rtv());

			{

				Slots::SMAA_Global slot_global;
				slot_global.GetColorTex() = source_tex.texture2D;
				slot_global.GetSubsampleIndices() = float4(0,0,0,0);
				slot_global.GetSMAA_RT_METRICS() = float4(1.0f / size.x, 1.0f / size.y, size);

				slot_global.set(graphics);
			}
		

			graphics.draw(4);

			{

				Slots::SMAA_Weights slot_edges;
				slot_edges.GetSearchTex() = search_tex->texture_2d()->texture2D;
				slot_edges.GetAreaTex() = area_tex->texture_2d()->texture2D;
				slot_edges.GetEdgesTex() = edges.texture2D;

				slot_edges.set(graphics);
			}
			graphics.set_pipeline(GetPSO<PSOS::BlendWeight>());
			graphics.set_rtv(1, blend.get_rtv(), Render::Handle());
			graphics.draw(4);



			{

				Slots::SMAA_Blend slot_blend;

				slot_blend.GetBlendTex() = blend.texture2D;

				slot_blend.set(graphics);
			}

			graphics.set_pipeline(GetPSO<PSOS::Blending>());
			graphics.set_rtv(1, target_tex.get_rtv(), Render::Handle());
			graphics.draw(4);

		});

	//resolving
	/*    {
	list.set_pipeline(state_resolve);
	buffer->result_tex.swap(context->list, Render::ResourceState::RENDER_TARGET, Render::ResourceState::PIXEL_SHADER_RESOURCE);
	list.set(2, buffer->result_tex.second()->texture_2d()->get_srv());
	temporal.set(context->list, 3);
	list.set_rtv(1, buffer->result_tex.first()->texture_2d()->get_rtv(0), Render::Handle());
	list.draw(4);
	}*/
	//	list.transition(buffer->light_tex.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE);
}

