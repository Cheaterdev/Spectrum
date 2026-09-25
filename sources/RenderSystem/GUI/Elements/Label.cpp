module GUI:Label;
import :Renderer;

import GUI;

namespace GUI
{
	namespace Elements
	{
		label::label() : text(std::bind(&label::on_text_changed, this, std::placeholders::_1)), font_size(std::bind(&label::on_size_changed, this, std::placeholders::_1))
		{
			clickable = false;
			magnet_text = FW1_LEFT | FW1_TOP | FW1_NOWORDWRAP;
			color = float4(1, 1, 1, 1);
			font_size = 16;
		}

		label::~label()
		{
		}

		// Sized on change rather than from on_pre_render: the tree walk skips
		// on_pre_render for an element with empty bounds, so a label sized only
		// there would stay 0x0 and never be drawn.
		void label::on_text_changed(const std::string& str)
		{
			text_size = Text::Engine::get().measure(str, style()) + vec2(2, 2);
			text_size.x = std::ceil(text_size.x);
			text_size.y = std::ceil(text_size.y);
			size = text_size;
		}

		void label::on_size_changed(const float& new_size)
		{
			on_text_changed(text.get());
		}

		void label::on_pre_render(Context& c)
		{
			PROFILE(L"label_build");

			// Rebuilt every frame even when unchanged: requesting the glyphs is
			// what keeps them alive through the atlas' per-frame eviction.
			Text::Style s = style();
			s.size *= c.scale;

			std::vector<Text::Link> current_links;
			{
				std::lock_guard<std::mutex> guard(links_m);
				current_links = links;
			}
			Text::Engine::get().build(text.get(), s, layout, current_links, hovered_link.load());

			if (Text::Engine::get().has_pending_upload())
				user_ui->pre_draw_infos.emplace_back(this);
		}

		// Alignment only applies while the text fits; overflowing text stays
		// anchored at the top-left and is clipped.
		vec2 label::text_origin(vec2 bounds_size, vec2 text_px) const
		{
			vec2 origin = { 0, 0 };

			if ((magnet_text & FW1_CENTER) && text_px.x < bounds_size.x)
				origin.x = (bounds_size.x - text_px.x) / 2;
			else if ((magnet_text & FW1_RIGHT) && text_px.x < bounds_size.x)
				origin.x = bounds_size.x - text_px.x;

			if ((magnet_text & FW1_VCENTER) && text_px.y < bounds_size.y)
				origin.y = (bounds_size.y - text_px.y) / 2;
			else if ((magnet_text & FW1_BOTTOM) && text_px.y < bounds_size.y)
				origin.y = bounds_size.y - text_px.y;

			return origin;
		}

		void label::set_links(std::vector<Text::Link> new_links, std::function<void(size_t)> on_click)
		{
			std::lock_guard<std::mutex> guard(links_m);
			links     = std::move(new_links);
			on_link   = std::move(on_click);
			clickable = !links.empty();
		}

		bool label::on_mouse_move(vec2 pos)
		{
			std::vector<Text::Link> current_links;
			{
				std::lock_guard<std::mutex> guard(links_m);
				current_links = links;
			}
			if (current_links.empty())
				return base::on_mouse_move(pos);

			// Same pixel space build() lays the text out in; mouse positions and
			// render bounds are both window pixels.
			Text::Style s = style();
			s.size *= result_scale;
			const rect bounds  = render_bounds.get();
			const vec2 text_px = Text::Engine::get().measure(text.get(), s);
			const vec2 local   = pos - vec2(bounds.pos) - text_origin(vec2(bounds.size), text_px);

			const int hit = Text::Engine::get().hit_link(text.get(), s, current_links, local);
			hovered_link = hit;
			cursor = hit >= 0 ? cursor_style::HAND : cursor_style::ARROW;
			return base::on_mouse_move(pos);
		}

		void label::on_mouse_leave(vec2 pos)
		{
			hovered_link = -1;
			cursor = cursor_style::ARROW;
			base::on_mouse_leave(pos);
		}

		bool label::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
		{
			const bool handled = base::on_mouse_action(action, button, pos);

			{
				std::lock_guard<std::mutex> guard(links_m);
				if (links.empty())
					return handled;
			}

			const int hit = hovered_link.load();
			if (action == mouse_action::UP && button == mouse_button::LEFT && hit >= 0)
			{
				std::function<void(size_t)> callback;
				{
					std::lock_guard<std::mutex> guard(links_m);
					callback = on_link;
				}
				if (callback)
					callback(size_t(hit));
			}
			return true;
		}

		void label::pre_draw(HAL::CommandList::ptr list)
		{
			Text::Engine::get().upload(list);
		}

		void label::draw(Context& c)
		{
			base::draw(c);
			PROFILE(L"label");

			rect bounds = render_bounds.get();
			bounds.x += c.offset.x;
			bounds.y += c.offset.y;

			const sizer clip = intersect(c.ui_clipping, math::convert(bounds));
			if (clip.left >= clip.right || clip.top >= clip.bottom)
				return;

			const vec2 pos = vec2(bounds.pos) + text_origin(vec2(bounds.size), layout.size);

			// Text is issued directly, not through the batched NinePatch path, so
			// anything queued ahead of it must reach the list first.
			c.renderer->flush(c);
			Text::Engine::get().draw(c.command_list, layout, pos, color, clip, c.window_size);
		}

		unsigned int label::get_index(vec2 at)
		{
			return Text::Engine::get().hit_test(text.get(), style(), at);
		}

		vec2 label::get_caret_pos(unsigned int index)
		{
			return Text::Engine::get().caret(text.get(), style(), index).center;
		}

		float label::get_line_height()
		{
			return Text::Engine::get().caret(text.get(), style(), 0).height;
		}

		void MultiLineLabel::on_text_changed(const std::string& str)
		{

			contents->remove_all();
			const std::string delim = "\n";

    for (const auto word : std::views::split(str, delim))
    {
		auto word_view = std::string_view(word);
		auto line = std::make_shared<label>();
		line->text = std::string(word_view.substr(0,word_view.length()-1));
		line->docking = dock::TOP;
		add_child(line);


    }

		}

		MultiLineLabel::MultiLineLabel()	: text(std::bind(&MultiLineLabel::on_text_changed, this, std::placeholders::_1))
		{


		}
	}
}
