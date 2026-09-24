module GUI:EditText;

import <windows/windows.h>;

import :Renderer;

import GUI;

namespace
{
	std::string utf16_to_utf8(std::wstring_view s)
	{
		if (s.empty()) return {};
		const int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
		std::string out(n, '\0');
		WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n, nullptr, nullptr);
		return out;
	}

	std::wstring utf8_to_utf16(std::string_view s)
	{
		if (s.empty()) return {};
		const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
		std::wstring out(n, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n);
		return out;
	}

	double now_seconds()
	{
		using namespace std::chrono;
		return duration<double>(steady_clock::now().time_since_epoch()).count();
	}

	const Text::Style edit_style = { 16, Text::Weight::Light };
}

GUI::Elements::edit_text::edit_text() : editor(edit_style)
{
	clickable = true;

	// Single line: control characters (newlines and tabs included, e.g. from a
	// paste) never enter the text, whatever the user filter says.
	editor.filter = [this](char32_t ch) { return ch >= 0x20 && ch != 0x7F && (!filter || filter(ch)); };

	padding = { 5, 5, 5, 5 };

	// Text drags show a preview in the drag holder (generate_container) rather
	// than a live copy of the whole field.
	set_package("text");
	drag_n_drop_copy = false;
}

// Mouse positions arrive in window pixels, the same space as render bounds;
// the editor's origin is the content box.
vec2 GUI::Elements::edit_text::to_editor(vec2 window_pos, float scale)
{
	const vec2 origin = vec2(get_render_bounds().pos) + vec2(padding->left, padding->top) * scale;
	return (window_pos - origin) / scale;
}

void GUI::Elements::edit_text::set_text(const std::string& t)
{
	std::lock_guard<std::mutex> guard(m);
	editor.set_text(t);
	text = t;
}

std::string GUI::Elements::edit_text::get_text()
{
	std::lock_guard<std::mutex> guard(m);
	return text;
}

void GUI::Elements::edit_text::on_key_action(key_action action, long key, key_mods mods)
{
	if (action != key_action::DOWN) return;
	std::lock_guard<std::mutex> guard(m);
	events.emplace_back(key_input{ key, mods });
}

void GUI::Elements::edit_text::on_char(char32_t ch)
{
	// Ctrl+letter arrives as a control character too; shortcuts are handled
	// from the key event instead.
	if (ch < 0x20 || ch == 0x7F) return;
	std::lock_guard<std::mutex> guard(m);
	events.emplace_back(ch);
}

bool GUI::Elements::edit_text::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
	std::lock_guard<std::mutex> guard(m);
	base::on_mouse_action(action, button, pos);

	if (button == mouse_button::RIGHT && action == mouse_action::UP)
		open_context_menu(pos);

	if (button == mouse_button::LEFT)
	{
		using Kind = mouse_input::Kind;
		const bool was_selecting = mouse_selecting;
		mouse_selecting = action == mouse_action::DOWN;

		if (action == mouse_action::DOWN)
		{
			drag_candidate = editor.selection_contains(to_editor(pos, result_scale));
			drag_started   = false;

			mouse_input down{ Kind::Down, pos, now_seconds() };
			down.in_selection = drag_candidate;
			events.emplace_back(down);
		}
		else if (was_selecting)
		{
			mouse_input up{ action == mouse_action::UP ? Kind::Up : Kind::Cancel, pos };
			up.after_drag = drag_started;
			events.emplace_back(up);
			drag_candidate = false;
		}
	}

	focus();
	return true;
}

bool GUI::Elements::edit_text::on_mouse_move(vec2 pos)
{
	std::lock_guard<std::mutex> guard(m);
	// A press held for drag-and-drop doesn't extend the selection.
	if (mouse_selecting && !drag_candidate)
		events.emplace_back(mouse_input{ mouse_input::Kind::Drag, pos });
	return base::on_mouse_move(pos);
}

bool GUI::Elements::edit_text::need_drag_drop()
{
	std::lock_guard<std::mutex> guard(m);
	return drag_candidate;
}

void GUI::Elements::edit_text::on_drag_start()
{
	std::lock_guard<std::mutex> guard(m);
	drag_started = true;
}

// UI thread, at drag start: a faded copy of the dragged text follows the cursor.
void GUI::Elements::edit_text::generate_container(base::ptr holder)
{
	auto preview = std::make_shared<label>();
	preview->text  = editor.get_selected_text();
	preview->color = float4(text_color.x, text_color.y, text_color.z, 0.6f);
	holder->add_child(preview);
}

bool GUI::Elements::edit_text::can_accept(drag_n_drop_package::ptr package)
{
	return package && package->name == "text";
}

void GUI::Elements::edit_text::on_drop_move(drag_n_drop_package::ptr package, vec2 pos)
{
	std::lock_guard<std::mutex> guard(m);
	drop_hover = true;
	drop_pos   = pos;
	cursor     = cursor_style::ALL;
}

void GUI::Elements::edit_text::on_drop_leave(drag_n_drop_package::ptr package)
{
	std::lock_guard<std::mutex> guard(m);
	drop_hover = false;
	cursor     = cursor_style::BEAM;
}

bool GUI::Elements::edit_text::on_drop(drag_n_drop_package::ptr package, vec2 pos)
{
	auto source = std::dynamic_pointer_cast<edit_text>(package->element.lock());
	if (!source) return false;

	// Mouse events carry no modifiers, and this runs on the UI thread rather
	// than the window thread GetKeyState() tracks.
	const bool copy = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

	std::lock_guard<std::mutex> guard(m);
	events.emplace_back(drop_input{ pos, source, copy });
	drop_hover = false;
	cursor     = cursor_style::BEAM;
	return true;
}

// Runs on the UI thread from on_mouse_action, with m held. Items don't edit
// directly: they queue the same key events their shortcuts produce, so edits
// still happen on the tree-walk thread through process_key.
void GUI::Elements::edit_text::open_context_menu(vec2 pos)
{
	user_ui->close_menus();

	auto menu = std::make_shared<menu_list>();
	w_ptr<edit_text> weak = get_ptr<edit_text>();

	auto add = [&](const char* name, long key, bool ctrl)
		{
			menu->add_item(name)->on_click = [weak, key, ctrl](menu_list_element::ptr)
				{
					if (auto self = weak.lock())
					{
						std::lock_guard<std::mutex> guard(self->m);
						key_mods mods;
						mods.ctrl = ctrl;
						self->events.emplace_back(key_input{ key, mods });
					}
				};
		};

	const bool selection = editor.has_selection();

	if (editor.can_undo()) add("Undo", 'Z', true);
	if (editor.can_redo()) add("Redo", 'Y', true);
	if (selection)         add("Cut", 'X', true);
	if (selection)         add("Copy", 'C', true);
	add("Paste", 'V', true);
	if (selection)         add("Delete", VK_DELETE, false);
	add("Select All", 'A', true);

	menu->pos = pos;
	menu->self_open(user_ui);
}

void GUI::Elements::edit_text::copy_selection()
{
	if (!editor.has_selection() || !user_ui || !user_ui->set_clipboard) return;
	user_ui->set_clipboard(utf8_to_utf16(editor.get_selected_text()));
}

void GUI::Elements::edit_text::process_key(long key, key_mods mods)
{
	// AltGr arrives as Ctrl+Alt and produces printable WM_CHARs on many layouts,
	// so it must not be taken for a Ctrl shortcut.
	const bool ctrl  = mods.ctrl && !mods.alt;
	const bool shift = mods.shift;

	using Key = Text::Editor::Key;
	switch (key)
	{
	case VK_LEFT:   editor.key(Key::Left, shift, ctrl); return;
	case VK_RIGHT:  editor.key(Key::Right, shift, ctrl); return;
	case VK_UP:     editor.key(Key::Up, shift, ctrl); return;
	case VK_DOWN:   editor.key(Key::Down, shift, ctrl); return;
	case VK_HOME:   editor.key(Key::Home, shift, ctrl); return;
	case VK_END:    editor.key(Key::End, shift, ctrl); return;
	case VK_BACK:   editor.key(Key::Backspace, shift, ctrl); return;
	case VK_DELETE: editor.key(Key::Delete, shift, ctrl); return;
	}

	if (!ctrl) return;

	switch (key)
	{
	case 'A':
		editor.select_all();
		break;

	case 'C':
		copy_selection();
		break;

	case 'X':
		copy_selection();
		editor.delete_selection();
		break;

	case 'V':
		if (user_ui && user_ui->get_clipboard)
			editor.insert(utf16_to_utf8(user_ui->get_clipboard()));
		break;

	case 'Z':
		if (shift) editor.redo(); else editor.undo();
		break;

	case 'Y':
		editor.redo();
		break;
	}
}

// The content box in window pixels, without the scroll offset: mouse input
// arrives in that space, and draw_color adds the offset itself.
rect GUI::Elements::edit_text::content_rect(Context& c)
{
	rect r = get_render_bounds();
	r.x += padding->left * c.scale;
	r.y += padding->top * c.scale;
	r.w -= (padding->left + padding->right) * c.scale;
	r.h -= (padding->top + padding->bottom) * c.scale;
	return r;
}

void GUI::Elements::edit_text::process_events(Context& c)
{
	for (auto& e : events)
	{
		if (auto k = std::get_if<key_input>(&e))
			process_key(k->key, k->mods);
		else if (auto ms = std::get_if<mouse_input>(&e))
			process_mouse(*ms, to_editor(ms->pos, c.scale));
		else if (auto d = std::get_if<drop_input>(&e))
			process_drop(*d, to_editor(d->pos, c.scale));
		else
			editor.insert(std::get<char32_t>(e));
	}
	events.clear();
}

void GUI::Elements::edit_text::process_mouse(const mouse_input& e, vec2 pos)
{
	using Kind = mouse_input::Kind;

	switch (e.kind)
	{
	case Kind::Down:
		press_held = e.in_selection;
		press_pos  = pos;
		press_time = e.time;
		if (!press_held)
			editor.mouse_click(pos, false, e.time);
		break;

	case Kind::Drag:
		editor.mouse_drag(pos);
		break;

	case Kind::Up:
		// Released without a drag-and-drop: replayed with its original time,
		// so it still counts toward a double-click with the next press.
		if (press_held && !e.after_drag)
			editor.mouse_click(press_pos, false, press_time);
		press_held = false;
		break;

	case Kind::Cancel:
		press_held = false;
		break;
	}
}

// Tree-walk thread, like every other editor mutation. The source field is
// touched from here too, which is safe because both run on this one thread.
void GUI::Elements::edit_text::process_drop(const drop_input& e, vec2 pos)
{
	auto source = e.source.lock();
	if (!source) return;

	if (source.get() == this)
	{
		editor.move_selection(pos, e.copy);
		return;
	}

	const std::string dropped = source->editor.get_selected_text();
	if (dropped.empty()) return;

	editor.drop_text(pos, dropped);
	if (!e.copy)
		source->editor.delete_selection();
}

void GUI::Elements::edit_text::on_pre_render(Context& c)
{
	PROFILE(L"edit_text_build");

	bool        changed = false;
	std::string changed_text;
	{
		std::lock_guard<std::mutex> guard(m);
		process_events(c);

		if (editor.take_changed())
		{
			text         = editor.get_text();
			changed      = true;
			changed_text = text;
		}

		showing_placeholder = text.empty() && !placeholder.empty();
		if (showing_placeholder)
		{
			Text::Style s = edit_style;
			s.size *= c.scale;
			Text::Engine::get().build(placeholder, s, layout);
		}
		else
			editor.build(c.scale, layout);

		selection     = editor.selection_rects();
		caret         = editor.caret();
		caret_visible = is_focused();

		if (drop_hover)
			drop_caret = editor.caret_at(to_editor(drop_pos, c.scale));
	}

	if (Text::Engine::get().has_pending_upload())
		user_ui->pre_draw_infos.emplace_back(this);

	// Outside the lock: handlers commonly write back through set_text().
	if (changed)
		on_change(changed_text);
}

void GUI::Elements::edit_text::pre_draw(HAL::CommandList::ptr list)
{
	Text::Engine::get().upload(list);
}

void GUI::Elements::edit_text::draw(Context& c)
{
	c.renderer->draw(c, Skin::get().DefaultEditBox.Normal, get_render_bounds());

	std::lock_guard<std::mutex> guard(m);
	const rect content = content_rect(c);
	for (auto& s : selection)
	{
		rect r;
		r.pos  = content.pos + vec2(s.x, s.y) * c.scale;
		r.size = vec2(s.z - s.x, s.w - s.y) * c.scale;
		c.renderer->draw_color(c, selection_color, r);
	}
}

// After draw: the text goes over the selection highlight, the caret over the text.
void GUI::Elements::edit_text::draw_after(Context& c)
{
	std::lock_guard<std::mutex> guard(m);

	const rect content = content_rect(c);
	rect on_screen = content;
	on_screen.x += c.offset.x;
	on_screen.y += c.offset.y;

	const sizer clip = intersect(c.ui_clipping, math::convert(on_screen));
	if (clip.left >= clip.right || clip.top >= clip.bottom)
		return;

	// Text is issued directly, not through the batched NinePatch path, so
	// anything queued ahead of it must reach the list first.
	c.renderer->flush(c);
	Text::Engine::get().draw(c.command_list, layout, content.pos + c.offset,
		showing_placeholder ? placeholder_color : text_color, clip, c.window_size);

	// While dragging a selection the caret shows where it would drop.
	const Text::Caret& shown = drop_hover ? drop_caret : caret;
	if (caret_visible || drop_hover)
	{
		rect r;
		r.pos  = content.pos + (shown.center - vec2(1, shown.height / 2)) * c.scale;
		r.size = vec2(2, shown.height) * c.scale;
		c.renderer->draw_color(c, caret_color, r);
	}
}

void GUI::Elements::edit_text::on_mouse_enter(vec2 pos)
{
	cursor = cursor_style::BEAM;
}

void GUI::Elements::edit_text::on_mouse_leave(vec2 pos)
{
	cursor = cursor_style::ARROW;
}
