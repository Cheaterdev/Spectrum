module GUI:Tree;
import :Renderer;


bool GUI::Elements::toogle_icon::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
    if (button == mouse_button::LEFT)
        if (action == mouse_action::UP)
			on_toogle();

	
    return true;
}

GUI::Elements::toogle_icon::toogle_icon() : vector_icon("chevron_right")
{
	size = { 16,16 };
	width_size = size_type::FIXED;
	height_size = size_type::FIXED;

    clickable = true;
}

void GUI::Elements::toogle_icon::toogle(bool v)
{
	set_icon(v ? "chevron_right" : "chevron_down");
}

void GUI::Elements::line_base::draw(Context& c, bool selected)
{
	 if (dropping)
		 c.renderer->draw_color(c, float4(153, 114, 202, 255) / 255.0f, get_render_bounds());
	 else if (selected)
		 c.renderer->draw_color(c, float4(53, 114, 202, 255) / 255.0f, get_render_bounds());
}