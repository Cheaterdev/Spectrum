module GUI:Renderer;

import HAL;

import :Skin;

namespace GUI
{


    void Renderer::draw_area(base::ptr obj, base::Context& c)
    {
        draw(c, area_tex, obj->get_render_bounds());
    }

    void Renderer::draw_container(base::ptr obj, base::Context& c)
    {
        draw(c, container_tex, obj->get_render_bounds());
    }


    void Renderer::draw_virtual(base::ptr obj, base::Context& c)
    {
        draw(c, virtual_tex, obj->get_render_bounds());
    }


    void Renderer::draw_color(base::Context& c, float4 color, rect r)
    {
        nine_patch.draw_color(c, color, color, r);
    }

    void Renderer::draw_color(base::Context& c, float4 left, float4 right, rect r)
    {
        nine_patch.draw_color(c, left, right, r);
    }

    void Renderer::start()
    {
        nine_patch.counter = 0;
    }

    void Renderer::set(base::Context& c)
    {
    }
	void Renderer::flush(base::Context& c)
	{
		nine_patch.flush(c);
	}
    Renderer::Renderer()
    {
        area_tex = Skin::get().Edit;
        virtual_tex = Skin::get().Virtual;
        container_tex = Skin::get().Background;
    }

    void Renderer::draw(base::Context& c, HAL::PipelineState::ptr state, rect r)
    {
        nine_patch.draw(c, state, r);
    }

    void Renderer::draw(base::Context& c, GUI::Texture& item, rect r)
    {
        nine_patch.draw(c, item, r);
    }

}
