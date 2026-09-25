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

GUI::Elements::edit_text::edit_text() : edit_text(edit_style)
{
}

GUI::Elements::edit_text::edit_text(Text::Style style) : style(style), editor(style)
{
	clickable = true;

	// Control characters never enter the text, whatever the user filter says;
	// a multiline field lets newlines and tabs through (typed or pasted). '\r'
	// from CRLF clipboard text is dropped: Skribidi treats it as a paragraph
	// break of its own.
	editor.filter = [this](char32_t ch)
		{
			const bool allowed = (ch >= 0x20 && ch != 0x7F) || (multiline && (ch == '\n' || ch == '\t'));
			return allowed && (!filter || filter(ch));
		};
	editor.highlighter = [this](std::u32string_view text, std::vector<uint32_t>& colors)
		{
			if (highlighter) highlighter(text, colors);
		};

	padding = { 5, 5, 5, 5 };

	// Text drags show a preview in the drag holder (generate_container) rather
	// than a live copy of the whole field.
	set_package("text");
	drag_n_drop_copy = false;

	// Hidden until sync_scroll_bars (multiline only) finds content that
	// doesn't fit. The bars live no longer than this element, so they can hold
	// a raw this.
	using scroll_type = scroll_bar::scroll_type;
	for (auto [bar, type] : { std::pair{ &vbar, scroll_type::VERTICAL }, std::pair{ &hbar, scroll_type::HORIZONTAL } })
	{
		*bar = std::make_shared<scroll_bar>(type);
		(*bar)->docking = type == scroll_type::VERTICAL ? dock::RIGHT : dock::BOTTOM;
		(*bar)->visible = false;
		(*bar)->on_move = [this, vertical = type == scroll_type::VERTICAL](float t)
			{
				std::lock_guard<std::mutex> guard(m);
				events.emplace_back(scrollbar_input{ vertical, t });
			};
		add_child(*bar);
	}
}

// Mouse positions arrive in window pixels, the same space as render bounds;
// the editor's origin is the content box, shifted by the scroll position.
vec2 GUI::Elements::edit_text::to_editor(vec2 window_pos, float scale)
{
	const vec2 origin = vec2(get_render_bounds().pos) + vec2(padding->left, padding->top) * scale;
	return (window_pos - origin) / scale + scroll;
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

void GUI::Elements::edit_text::set_diagnostics(std::vector<Text::Diagnostic> diagnostics)
{
	editor.set_diagnostics(std::move(diagnostics));
}

void GUI::Elements::edit_text::goto_line(uint32_t line, uint32_t column)
{
	std::lock_guard<std::mutex> guard(m);
	editor.set_caret(line, column);
	follow_caret = true;
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

	// The tooltip manager shows whatever this holds while hovered.
	tooltip = editor.diagnostic_at(to_editor(pos, result_scale));
	return base::on_mouse_move(pos);
}

bool GUI::Elements::edit_text::on_wheel(mouse_wheel type, float value, vec2 pos)
{
	// Single-line fields leave the wheel to whatever scrolls around them
	// (e.g. the Properties panel).
	if (!multiline || type != mouse_wheel::VERTICAL) return false;

	std::lock_guard<std::mutex> guard(m);
	events.emplace_back(wheel_input{ value });
	return true;
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
	if (!package) return false;
	if (package->name == "text") return true;

	// Anything the image hook can turn into a texture becomes a block image.
	Image image;
	return multiline && image_from_package && image_from_package(package, image);
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
	Image image;
	if (package->name != "text" && multiline && image_from_package && image_from_package(package, image))
	{
		std::lock_guard<std::mutex> guard(m);
		const uint32_t id = next_image_id++;
		inserted_images[id] = std::move(image);
		events.emplace_back(image_input{ pos, id });
		drop_hover = false;
		cursor     = cursor_style::BEAM;
		return true;
	}

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

	if (selection)
	{
		add("Bold", 'B', true);
		add("Italic", 'I', true);

		auto colors = menu->add_item("Color")->get_menu();
		auto add_color = [&](const char* name, std::optional<float4> color)
			{
				colors->add_item(name)->on_click = [weak, color](menu_list_element::ptr)
					{
						if (auto self = weak.lock())
						{
							std::lock_guard<std::mutex> guard(self->m);
							self->events.emplace_back(color_input{ color });
						}
					};
			};

		add_color("Red",     float4(200, 30, 30, 255) / 255.0f);
		add_color("Green",   float4(30, 150, 50, 255) / 255.0f);
		add_color("Blue",    float4(30, 80, 220, 255) / 255.0f);
		add_color("Orange",  float4(230, 120, 0, 255) / 255.0f);
		add_color("Default", std::nullopt);
	}

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

	case VK_RETURN:
		if (multiline) editor.key(Key::Enter, shift, ctrl);
		return;

	case VK_TAB:
		// Tab arrives only as a key: on_char drops control characters.
		if (multiline && !ctrl) editor.insert(U'\t');
		return;
	}

	if (!ctrl) return;

	switch (key)
	{
	case 'A':
		editor.select_all();
		break;

	case 'B':
		editor.toggle_bold();
		break;

	case 'I':
		editor.toggle_italic();
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

	// Visible scroll bars are docked inside the padding and take their strips.
	if (vbar->visible.get()) r.w -= vbar->get_render_bounds().w;
	if (hbar->visible.get()) r.h -= hbar->get_render_bounds().h;
	return r;
}

// Bar properties belong to the UI thread, and this runs on the tree walk:
// hand the numbers over, only when they changed.
void GUI::Elements::edit_text::sync_scroll_bars(vec2 view, vec2 content)
{
	if (view == synced_view && content == synced_content && scroll == synced_scroll)
		return;

	synced_view    = view;
	synced_content = content;
	synced_scroll  = scroll;

	run_on_ui([v = vbar, h = hbar, view, content, scroll = scroll]()
		{
			v->set_sizes(view.y, content.y, -scroll.y);
			h->set_sizes(view.x, content.x, -scroll.x);
		});
}

void GUI::Elements::edit_text::process_events(Context& c)
{
	for (auto& e : events)
	{
		// Everything but the wheel moves the caret or the text under it, and
		// the view should follow; wheel scrolling must not snap back to it.
		if (auto w = std::get_if<wheel_input>(&e))
		{
			const float line = caret.height > 0 ? caret.height : style.size * 1.3f;
			scroll.y -= w->notches * line * 3;
			continue;
		}
		follow_caret = true;

		if (auto k = std::get_if<key_input>(&e))
			process_key(k->key, k->mods);
		else if (auto ms = std::get_if<mouse_input>(&e))
			process_mouse(*ms, to_editor(ms->pos, c.scale));
		else if (auto d = std::get_if<drop_input>(&e))
			process_drop(*d, to_editor(d->pos, c.scale));
		else if (auto col = std::get_if<color_input>(&e))
		{
			if (col->color) editor.set_color(*col->color);
			else            editor.clear_color();
		}
		else if (auto img = std::get_if<image_input>(&e))
		{
			// Natural size, capped in height and to the view's width.
			const auto found = inserted_images.find(img->id);
			if (found == inserted_images.end() || found->second.size.x <= 0 || found->second.size.y <= 0)
				continue;

			const vec2  size   = found->second.size;
			const rect  box    = content_rect(c);
			const float max_w  = std::max(16.0f, box.w / c.scale - 8);
			float height = std::min(size.y, 240.0f);
			if (size.x * height / size.y > max_w)
				height = max_w * size.y / size.x;

			editor.insert_image(to_editor(img->pos, c.scale), img->id, height);
		}
		else if (auto bar = std::get_if<scrollbar_input>(&e))
		{
			// Same mapping as scroll_container: t spans the scrollable range.
			const rect content = content_rect(c);
			const vec2 view  = vec2(content.w, content.h) / c.scale;
			const vec2 range = vec2::max(vec2(0, 0), editor.content_size() + vec2(4, 0) - view);
			if (bar->vertical) scroll.y = bar->t * range.y;
			else               scroll.x = bar->t * range.x;
			follow_caret = false;   // dragging the thumb must not snap back to the caret
		}
		else
			editor.insert(std::get<char32_t>(e));
	}
	events.clear();
}

// Keeps the caret in view after edits and caret moves, and keeps the view
// inside the content (which may have shrunk). Logical units.
void GUI::Elements::edit_text::update_scroll(vec2 view)
{
	if (follow_caret)
	{
		const float top    = caret.center.y - caret.height / 2;
		const float bottom = caret.center.y + caret.height / 2;
		const float x      = caret.center.x;
		const float margin = 2;

		if (x < scroll.x + margin)               scroll.x = x - margin;
		else if (x > scroll.x + view.x - margin) scroll.x = x - view.x + margin;

		if (top < scroll.y)                 scroll.y = top;
		else if (bottom > scroll.y + view.y) scroll.y = bottom - view.y;

		follow_caret = false;
	}

	// The caret may sit just past the last glyph, hence the extra width.
	const vec2 content = editor.content_size() + vec2(4, 0);
	scroll.x = std::clamp(scroll.x, 0.0f, std::max(0.0f, content.x - view.x));
	scroll.y = std::clamp(scroll.y, 0.0f, std::max(0.0f, content.y - view.y));
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
			Text::Style s = style;
			s.size *= c.scale;
			Text::Engine::get().build(placeholder, s, layout);
		}
		else
			editor.build(c.scale, layout);

		selection     = editor.selection_rects();
		caret         = editor.caret();
		caret_visible = is_focused();
		image_boxes   = editor.images();

		const rect content = content_rect(c);
		const vec2 view    = vec2(content.w, content.h) / c.scale;
		update_scroll(view);
		if (multiline)
			sync_scroll_bars(view, editor.content_size() + vec2(4, 0));

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
	rect on_screen = content;
	on_screen.x += c.offset.x;
	on_screen.y += c.offset.y;

	// Scrolled content must not spill over the padding; draw_color clips to
	// ui_clipping, so narrow it to the content box for these draws.
	const sizer orig_clip = c.ui_clipping;
	c.ui_clipping = intersect(orig_clip, math::convert(on_screen));

	// Editor space to window pixels (draw_color adds c.offset itself).
	const vec2 origin = content.pos - scroll * c.scale;
	for (auto& s : selection)
	{
		rect r;
		r.pos  = origin + vec2(s.x, s.y) * c.scale;
		r.size = vec2(s.z - s.x, s.w - s.y) * c.scale;
		c.renderer->draw_color(c, selection_color, r);
	}

	// Block images, under the text like the selection. Width from the
	// image's aspect: the editor only reserved the height.
	for (auto& box : image_boxes)
	{
		const auto found = inserted_images.find(box.id);
		if (found == inserted_images.end() || found->second.size.y <= 0)
			continue;

		const vec2 size = found->second.size;
		rect r;
		r.pos  = origin + box.pos * c.scale;
		r.size = vec2(size.x * box.height / size.y, box.height) * c.scale;

		GUI::Texture texture;
		texture = found->second.view;
		c.renderer->draw(c, texture, r);
	}

	c.ui_clipping = orig_clip;
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

	const vec2 origin = content.pos - scroll * c.scale;

	// Text is issued directly, not through the batched NinePatch path, so
	// anything queued ahead of it must reach the list first.
	c.renderer->flush(c);
	Text::Engine::get().draw(c.command_list, layout, origin + c.offset,
		showing_placeholder ? placeholder_color : text_color, clip, c.window_size);

	// While dragging a selection the caret shows where it would drop.
	const Text::Caret& shown = drop_hover ? drop_caret : caret;
	if (caret_visible || drop_hover)
	{
		const sizer orig_clip = c.ui_clipping;
		c.ui_clipping = clip;

		rect r;
		r.pos  = origin + (shown.center - vec2(1, shown.height / 2)) * c.scale;
		r.size = vec2(2, shown.height) * c.scale;
		c.renderer->draw_color(c, caret_color, r);

		c.ui_clipping = orig_clip;
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
