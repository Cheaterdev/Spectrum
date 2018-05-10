#include "pch.h"



namespace GUI
{
	namespace Elements
	{
		circle_selector::circle_selector()
		{
			draw_helper = true;

			center.reset(new image);

			center->size = {7,7 };
			center->pos = { 10,10 };
		//	center->draw_helper = true;
			center->clickable = false;

			center->texture = Skin::get().DefaultCircleSelector.Inner;
			texture = Skin::get().DefaultCircleSelector.Main;
			add_child(center);
			clickable = true;
			//catch_clicks = true;
			size = { 64,64 };
			width_size = GUI::size_type::FIXED;
			height_size = GUI::size_type::FIXED;
		}

		bool circle_selector::on_mouse_move(vec2 pos)
		{
			if (movable)
			{
				vec2 delta = pos - get_render_bounds().pos;

				delta -= float2( get_render_bounds().size) / 2;

				float l = delta.length();
				if (l > size->y / 2)
				{
					delta *= (size->y / 2)/l;
				}

				float2 value = delta + float2(get_render_bounds().size) / 2;
				center->pos = value- center->size.get()/2;
				on_change(delta/ (float2(get_render_bounds().size) / 2));
			}

			return true;
		}

		bool circle_selector::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
		{
			if (action == mouse_action::DOWN && button == mouse_button::LEFT)

			{
				set_movable(true); movable = true;
			}
			if (action != mouse_action::DOWN && button == mouse_button::LEFT)
			{
				set_movable(false); movable = false;
			}
			return true;
		}

		void circle_selector::set_value(float2 value)
		{
			center->pos = value*size.get() + size.get() / 2 - center->size.get() / 2;
			on_change(value);
		}

	}
}
