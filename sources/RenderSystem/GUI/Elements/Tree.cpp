module GUI:Tree;
import :Renderer;


bool GUI::Elements::toogle_icon::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
    if (button == mouse_button::LEFT)
        if (action == mouse_action::UP)
			on_toogle();

	
    return true;
}

GUI::Elements::toogle_icon::toogle_icon()
{
  //  this->owner = owner;
	size = { 16,16 };
	width_size = size_type::FIXED;
	height_size = size_type::FIXED;

    clickable = true;

	tex_closed = Skin::get().TreeOpened;
	tex_opened = Skin::get().TreeClosed;
	texture = tex_opened.texture;

}

void GUI::Elements::toogle_icon::toogle(bool v)
{
	if (v) texture = tex_opened.texture;

	else
		texture = tex_closed.texture;

}

void GUI::Elements::line_base::draw(Context& c, bool selected)
{
	 if (dropping)
		 renderer->draw_color(c, float4(153, 114, 202, 255) / 255.0f, get_render_bounds());
	 else if (selected)
		 renderer->draw_color(c, float4(53, 114, 202, 255) / 255.0f, get_render_bounds());
}