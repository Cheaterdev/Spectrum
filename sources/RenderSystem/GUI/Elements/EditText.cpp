module GUI:EditText;

import <windows/windows.h>;

import :Renderer;

import GUI;

namespace
{
	// This class indexes text in bytes while label's caret queries count
	// codepoints; ASCII keeps the two equal until editing moves to skb_editor.
	bool is_insertable(char32_t ch)
	{
		return ch >= 0x20 && ch <= 0x7E;
	}

	bool is_word_char(char c)
	{
		return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
	}
}

void GUI::Elements::edit_text::on_key_action(key_action action, long key, key_mods mods)
{
	if (action != key_action::DOWN) return;
	std::lock_guard<std::mutex> guard(m);
	events.emplace_back(key_input{ key, mods });
}

void GUI::Elements::edit_text::on_char(char32_t ch)
{
	std::lock_guard<std::mutex> guard(m);
	events.emplace_back(ch);
}

void GUI::Elements::edit_text::set_text(const std::string& t)
{
	std::lock_guard<std::mutex> guard(m);
	text       = t;
	cursor_pos = (unsigned int)text.size();
	anchor_pos = cursor_pos;
	label_text->text           = text;
	placeholder_label->visible = text.empty();
}

bool GUI::Elements::edit_text::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
	std::lock_guard<std::mutex> guard(m);
	base::on_mouse_action(action, button, pos);

	if (button == mouse_button::LEFT)
	{
		if (action == mouse_action::DOWN)
		{
			move_cursor(label_text->get_index(to_text_local(pos)), false);
			mouse_selecting = true;
		}
		else
			mouse_selecting = false;
	}

	focus();
	return true;
}

bool GUI::Elements::edit_text::on_mouse_move(vec2 pos)
{
	std::lock_guard<std::mutex> guard(m);
	if (mouse_selecting)
		move_cursor(label_text->get_index(to_text_local(pos)), true);
	return base::on_mouse_move(pos);
}

// Mouse positions arrive in window pixels, the same space as render bounds.
vec2 GUI::Elements::edit_text::to_text_local(vec2 window_pos)
{
	return (window_pos - vec2(label_text->get_render_bounds().pos)) / result_scale;
}

GUI::Elements::edit_text::edit_text()
{
	text = "";
	clickable = true;
	cursor_pos = 0;
	anchor_pos = 0;

	// Children draw in insertion order; this must precede the labels so the
	// highlight sits under the text.
	selection_layer.reset(new base());
	selection_layer->docking = dock::FILL;
	add_child(selection_layer);

	placeholder_label.reset(new label());
	placeholder_label->text    = placeholder;
	placeholder_label->color   = rgba8(120, 120, 120, 180);
	placeholder_label->docking = dock::FILL;
	placeholder_label->visible = true; // text starts empty, so placeholder is visible
	add_child(placeholder_label);

	label_text.reset(new label());
	label_text->text    = text;
	label_text->color   = rgba8(40, 40, 40, 255);
	label_text->docking = dock::FILL;
	add_child(label_text);

	label_cursor.reset(new edit_cursor());
	add_child(label_cursor);
	padding = { 5, 5, 5, 5 };
}

void GUI::Elements::edit_text::draw(Context& c)
{
	process_events();
	{
		std::lock_guard<std::mutex> guard(m);
		update_caret();
		update_selection();
	}
	c.renderer->draw(c, Skin::get().DefaultEditBox.Normal, get_render_bounds());
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
	label_cursor->visible = is_focused();

	// The label is docked FILL at the top-left of this element's content box,
	// which is also the origin of child positions, so text-local caret
	// coordinates are used as-is.
	label_cursor->pos = label_text->get_caret_pos(cursor_pos) - label_cursor->size.get() / 2;
}

void GUI::Elements::edit_text::update_selection()
{
	size_t used = 0;

	if (has_selection())
	{
		const unsigned int end  = std::min<unsigned int>(selection_end(), (unsigned int)text.size());
		const float        line = label_text->get_line_height();

		// One rect per visual line: the label may have wrapped the selected span.
		unsigned int run = selection_start();
		while (run < end)
		{
			const vec2 from = label_text->get_caret_pos(run);

			unsigned int next = run + 1;
			while (next < end && label_text->get_caret_pos(next).y == from.y)
				++next;

			const vec2 to    = label_text->get_caret_pos(next);
			const float to_x = (to.y == from.y) ? to.x : label_text->get_caret_pos(next - 1).x + line * 0.5f;

			if (used == selection_rects.size())
			{
				auto r = std::make_shared<colored_rect>();
				r->color = float4(0.25f, 0.5f, 1.0f, 0.35f);
				selection_layer->add_child(r);
				selection_rects.push_back(r);
			}

			auto& r   = selection_rects[used++];
			r->pos     = vec2(from.x, from.y - line * 0.5f);
			r->size    = vec2(to_x - from.x, line);
			r->visible = true;

			run = next;
		}
	}

	for (size_t i = used; i < selection_rects.size(); ++i)
		selection_rects[i]->visible = false;
}

void GUI::Elements::edit_text::move_cursor(unsigned int pos, bool extend)
{
	cursor_pos = std::min<unsigned int>(pos, (unsigned int)text.size());
	if (!extend)
		anchor_pos = cursor_pos;
}

unsigned int GUI::Elements::edit_text::word_left(unsigned int pos) const
{
	while (pos > 0 && !is_word_char(text[pos - 1])) --pos;
	while (pos > 0 && is_word_char(text[pos - 1])) --pos;
	return pos;
}

unsigned int GUI::Elements::edit_text::word_right(unsigned int pos) const
{
	const unsigned int n = (unsigned int)text.size();
	while (pos < n && is_word_char(text[pos])) ++pos;
	while (pos < n && !is_word_char(text[pos])) ++pos;
	return pos;
}

void GUI::Elements::edit_text::erase_selection()
{
	if (!has_selection()) return;

	const unsigned int start = selection_start();
	text.erase(start, selection_end() - start);
	cursor_pos = anchor_pos = start;
}

void GUI::Elements::edit_text::insert(std::string_view str)
{
	std::string accepted;
	for (char ch : str)
		if (is_insertable(static_cast<unsigned char>(ch)) && (!filter || filter(ch)))
			accepted.push_back(ch);

	if (accepted.empty()) return;

	erase_selection();
	text.insert(cursor_pos, accepted);
	cursor_pos += (unsigned int)accepted.size();
	anchor_pos = cursor_pos;
}

void GUI::Elements::edit_text::copy_selection()
{
	if (!has_selection() || !user_ui || !user_ui->set_clipboard) return;

	const unsigned int start = selection_start();
	user_ui->set_clipboard(convert(std::string_view(text).substr(start, selection_end() - start)));
}

void GUI::Elements::edit_text::process_key(long key, key_mods mods)
{
	// AltGr arrives as Ctrl+Alt and produces printable WM_CHARs on many layouts,
	// so it must not be taken for a Ctrl shortcut.
	const bool ctrl  = mods.ctrl && !mods.alt;
	const bool shift = mods.shift;

	switch (key)
	{
	case VK_LEFT:
		if (has_selection() && !shift)
			move_cursor(selection_start(), false);
		else
			move_cursor(ctrl ? word_left(cursor_pos) : (cursor_pos ? cursor_pos - 1 : 0), shift);
		return;

	case VK_RIGHT:
		if (has_selection() && !shift)
			move_cursor(selection_end(), false);
		else
			move_cursor(ctrl ? word_right(cursor_pos) : cursor_pos + 1, shift);
		return;

	case VK_HOME:
		move_cursor(0, shift);
		return;

	case VK_END:
		move_cursor((unsigned int)text.size(), shift);
		return;

	case VK_UP:
	case VK_DOWN:
	{
		const float step = key == VK_UP ? -label_text->get_line_height() : label_text->get_line_height();
		move_cursor(label_text->get_index(label_text->get_caret_pos(cursor_pos) + vec2(0, step)), shift);
		return;
	}

	case VK_BACK:
		if (!has_selection() && cursor_pos > 0)
			anchor_pos = ctrl ? word_left(cursor_pos) : cursor_pos - 1;
		erase_selection();
		return;

	case VK_DELETE:
		if (!has_selection() && cursor_pos < text.size())
			anchor_pos = ctrl ? word_right(cursor_pos) : cursor_pos + 1;
		erase_selection();
		return;
	}

	if (!ctrl) return;

	switch (key)
	{
	case 'A':
		anchor_pos = 0;
		cursor_pos = (unsigned int)text.size();
		break;

	case 'C':
		copy_selection();
		break;

	case 'X':
		copy_selection();
		erase_selection();
		break;

	case 'V':
		if (user_ui && user_ui->get_clipboard)
			insert(convert(std::wstring_view(user_ui->get_clipboard())));
		break;
	}
}

void GUI::Elements::edit_text::process_events()
{
	bool        changed = false;
	std::string changed_text;

	{
		std::lock_guard<std::mutex> guard(m);

		for (auto& e : events)
		{
			if (auto k = std::get_if<key_input>(&e))
				process_key(k->key, k->mods);
			else
			{
				const char32_t ch = std::get<char32_t>(e);
				if (is_insertable(ch))
				{
					const char narrow = static_cast<char>(ch);
					insert(std::string_view(&narrow, 1));
				}
			}
		}
		events.clear();

		changed = (label_text->text.get() != text);
		label_text->text           = text;
		placeholder_label->text    = placeholder;
		placeholder_label->visible = text.empty();

		if (changed)
			changed_text = text;
	}

	// Outside the lock: handlers commonly write back through set_text().
	if (changed)
		on_change(changed_text);
}

void GUI::Elements::edit_cursor::draw(Context& c)
{
	if (!test_local_visible())
		return;

	c.renderer->draw_color(c, float4(0, 0, 0, 1), get_render_bounds());
}

GUI::Elements::edit_cursor::edit_cursor()
{
	size = { 4, 20 };
	time = 0;
}
