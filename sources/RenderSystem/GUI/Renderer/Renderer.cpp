#include "pch_render.h"
#include "Renderer.h"

namespace GUI
{


    void Renderer::draw_area(base::ptr obj, Graphics::context& c)
    {
        draw(c, area_tex, obj->get_render_bounds());
    }

    void Renderer::draw_container(base::ptr obj, Graphics::context& c)
    {
        draw(c, container_tex, obj->get_render_bounds());
    }


    void Renderer::draw_virtual(base::ptr obj, Graphics::context& c)
    {
        draw(c, virtual_tex, obj->get_render_bounds());
    }


    void Renderer::draw_color(Graphics::context& c, float4 color, rect r)
    {
		flush(c);
        simple_rect->draw(c, color, r);
    }

    void Renderer::set(Graphics::context& c)
    {
    }
	void Renderer::flush(Graphics::context& c)
	{
		nine_patch->flush(c);
	}
    Renderer::Renderer()
    {
     
        nine_patch.reset(new NinePatch());
        simple_rect.reset(new SimpleRect());
        area_tex.texture = Graphics::Texture::get_resource({ to_path(L"textures/gui/edit.png"), false, false });
        virtual_tex.texture = Graphics::Texture::get_resource({ to_path(L"textures/gui/virtual.png"), false , false });
        container_tex.texture = Graphics::Texture::get_resource({ to_path(L"textures/gui/background.png"), false, false });
        area_tex.padding = { 5, 5, 5, 5 };
        virtual_tex.padding = { 5, 5, 5, 5 };
		container_tex.padding = {2,2,2,2};
    }

    void Renderer::draw(Graphics::context& c, Graphics::PipelineState::ptr state, rect r)
    {
        nine_patch->draw(c, state, r);
    }

    void Renderer::draw(Graphics::context& c, GUI::Texture& item, rect r)
    {
        nine_patch->draw(c, item, r);
    }

    SimpleRect::SimpleRect()
    {
    }

    void SimpleRect::draw(Graphics::context& c, float4 color, rect r)
	{
		Slots::ColorRect color_data;


        auto vertexes = (vec2*)color_data.GetPos();

        vertexes[0] = float2(0, 0);
        vertexes[1] = float2(0, r.size.y);
        vertexes[2] = float2(r.size.x, 0);
        vertexes[3] = float2(r.size.x, r.size.y);

		auto &clip = c.ui_clipping;


		for (int i = 0; i < 4; i++)
		{
			vertexes[i] += float2(r.pos) + c.offset;
			vertexes[i] = float2::max(vertexes[i], float2(clip.left_top));
			vertexes[i] = float2::min(vertexes[i], float2(clip.right_bottom));

		}
           
        for (int i = 0; i < 4; i++)
        {
            float2 t = 2 * vertexes[i] / c.window_size - float2(1, 1);
            vertexes[i] = { t.x, -t.y };
        }

      
        color_data.GetColor() = color;

        color_data.set(c.command_list->get_graphics());

        c.command_list->get_graphics().set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
        c.command_list->get_graphics().set_pipeline(GetPSO<PSOS::SimpleRect>());
        c.command_list->get_graphics().draw(4);

    }

}
