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
	thinkable = true;   // turns the arrow
}

void GUI::Elements::toogle_icon::toogle(bool v)
{
	set_expanded(!v);
}

void GUI::Elements::toogle_icon::show()
{
	const float eased = turn * turn * (3.0f - 2.0f * turn);
	const int step = int(std::round(eased * 9.0f));

	if (step == shown_step)
		return;

	shown_step = step;
	set_icon("chevron_" + std::to_string(step * 10));
}

void GUI::Elements::toogle_icon::set_expanded(bool expanded)
{
	turn_target = expanded ? 1.0f : 0.0f;

	if (!user_ui || !user_ui->window_animations)
	{
		turn = turn_target;
		show();
	}
}

void GUI::Elements::toogle_icon::think(float dt)
{
	if (turn == turn_target)
		return;

	const float step = dt / 0.18f;   // the same time as the branch it belongs to
	turn = turn < turn_target ? std::min(turn + step, turn_target) : std::max(turn - step, turn_target);
	show();
}

void GUI::Elements::draw_selection_bar(base::Context& c, const rect& box, float content_y, const SlidingHighlight& bar, float x, float w, float strength)
{
	if (bar.alpha <= 0.0f)
		return;

	const float top    = std::max(content_y + bar.y, box.y);
	const float bottom = std::min(content_y + bar.y + bar.h, box.y + box.h);
	if (bottom <= top)
		return;

	float4 color = float4(53, 114, 202, 255) / 255.0f;
	color.w *= bar.alpha * strength;
	c.renderer->draw_color(c, color, rect(x, top, w, bottom - top));
}

void GUI::Elements::line_base::draw(Context& c)
{
	 if (dropping)
		 c.renderer->draw_color(c, float4(153, 114, 202, 255) / 255.0f, get_render_bounds());
}