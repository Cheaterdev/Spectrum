#include "pch_render.h"
#include "EditText.h"
#include "GUI/Skin.h"
import GUI;


void GUI::Elements::edit_text::on_key_action(long key)
{
	std::lock_guard<std::mutex> guard(m);
	keys.push_back(key);
}

bool GUI::Elements::edit_text::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
	std::lock_guard<std::mutex> guard(m);
	base::on_mouse_action(action, button, pos);
	cursor_pos = label_text->get_index(pos);
	focus();
	return true;
}

GUI::Elements::edit_text::edit_text()
{
	text = "edit text";
	clickable = true;
	cursor_pos = 0;
	label_text.reset(new label());
	label_text->text = text;
	label_text->color = rgba8(40, 40, 40, 255);
	label_text->docking = dock::FILL;
	add_child(label_text);
	label_cursor.reset(new edit_cursor());
	//label_text->text = text;
	//	label_text->color = rgba8(40, 40, 40, 255);
	//label_text->docking = dock::FILL;
	add_child(label_cursor);
	padding = { 5, 5, 5, 5 };
}
void GUI::Elements::edit_text::draw(Graphics::context& c)
{
	process_keys();
	label_text->recalculate(c);
	update_caret();
	renderer->draw(c, Skin::get().DefaultEditBox.Normal, get_render_bounds());
}
void GUI::Elements::edit_text::on_mouse_enter(vec2 pos)
{
	cursor = cursor_style::BEAM;
}
void GUI::Elements::edit_text::on_mouse_leave(vec2 pos)
{
	cursor = cursor_style::ARROW;
}
void GUI::Elements::edit_text::update_caret()
{
	if (!text.empty() && cursor_pos <= text.size())
		label_cursor->pos = to_local(label_text->get_geometry()->get_pos(cursor_pos)) - vec2(label_cursor->size.get().x / 2, label_cursor->size.get().y / 2);
}
void GUI::Elements::edit_text::process_keys()
{
	std::lock_guard<std::mutex> guard(m);

	for (auto key : keys)
	{
		if (key == VK_UP)
			cursor_pos = label_text->get_index(label_text->get_geometry()->get_pos(cursor_pos) - vec2(0, label_text->get_geometry()->get_size()));
		else if (key == VK_DOWN)
			cursor_pos = label_text->get_index(label_text->get_geometry()->get_pos(cursor_pos) + vec2(0, label_text->get_geometry()->get_size()));
		else if (key == VK_RIGHT)
		{
			if (cursor_pos < text.size())
				++cursor_pos;
		}

		else if (key == VK_LEFT)
		{
			if (cursor_pos > 0)
				--cursor_pos;
		}

		else if (key == VK_BACK)
		{
			if (text.size() && cursor_pos > 0)
				text.erase(--cursor_pos, 1);
		}

		else if (key == VK_DELETE)
		{
			if (cursor_pos < text.size())
				text.erase(cursor_pos, 1);
		}

		else if (isprint(key))
		{
			text.insert(text.begin() + cursor_pos, char(key));
			cursor_pos++;
		}

		/*	else if (key == VK_RETURN)
			{
			text.insert(text.begin() + cursor_pos, '\n');
			cursor_pos++;
			}*/
	}

	label_text->text = text;
	keys.clear();
}
void GUI::Elements::edit_cursor::draw(Graphics::context& c)
{
	if (!test_local_visible())
		return;

	/*  c.dx_context.get_shader_state<DX11::vertex_shader>().const_buffer_state[0] = cb_vertex;
	  c.dx_context.get_shader_state<DX11::pixel_shader>().const_buffer_state[0] = cb_pixel;
	  c.dx_context.set(v_shader);
	  c.dx_context.set(p_shader);
	  c.dx_context.set(DX11::geometry_shader::null);
	  //	c.dx_context.set(blend);
	  //	c.dx_context.set(depth);
	  c.dx_context.draw(4, 0);*/
	renderer->draw_color(c, float4(0, 0, 0, 1), get_render_bounds());
}
GUI::Elements::edit_cursor::edit_cursor()
{
	size = { 4, 20 };
	time = 0;
}