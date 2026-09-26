module GUI:ScrollBar;
import :Renderer;

namespace GUI
{
	// In the thumb's logical units, like its pos and size. render_bounds is in
	// window pixels (logical * result_scale): mixing the two made the thumb
	// lag the mouse by the UI scale (1.5-2x on a high-DPI display).
	float Elements::scroll_bar::track_length()
	{
		const float scale = drag->result_scale > 0 ? drag->result_scale : 1.0f;
		return type == scroll_type::HORIZONTAL ? render_bounds->w / scale - padding->left - padding->right
		                                       : render_bounds->h / scale - padding->top - padding->bottom;
	}

	// The thumb travels track - thumb, both here and in on_move: with a long
	// document the proportional thumb is below its minimal size, and placing
	// it at track * scroll / content (as if it were proportional) put it out
	// of step with the drag mapping and pushed it off the end near the bottom.
	void Elements::scroll_bar::set_sizes(float filled_size, float container_size, float container_pos)
	{
		const bool  horizontal = type == scroll_type::HORIZONTAL;
		const float track      = std::max(0.0f, track_length());
		const float minimal    = horizontal ? drag->minimal_size.x : drag->minimal_size.y;

		float fraction = 1, t = 0;
		if (container_size > Math::eps10)
		{
			fraction = std::min(1.0f, filled_size / container_size);
			const float range = container_size - filled_size;
			t = range > 0 ? std::clamp(-container_pos / range, 0.0f, 1.0f) : 0.0f;
		}

		const float thumb = std::min(track, std::max(minimal, track * fraction));
		const float pos   = (track - thumb) * t;

		// While the thumb is held, the drag owns its position. The dragger moves
		// it incrementally (pos += mouse delta), and an owner that reports back
		// a frame later (edit_text syncs through run_on_ui) would set it to
		// where it was a move ago, dropping part of every delta: the thumb
		// fell behind the mouse.
		const bool held = drag->is_dragging();

		if (horizontal)
		{
			drag->size = { thumb, drag->size->y };
			if (!held) drag->pos = { pos, drag->pos->y };
		}
		else
		{
			drag->size = { drag->size->x, thumb };
			if (!held) drag->pos = { drag->pos->x, pos };
		}

		visible = fraction < 1;
	}

	void Elements::scroll_bar::draw(Context& c)
	{
		// The thumb is a dragger and draws itself (idle/active colors set below).
		c.renderer->draw_color(c, float4(0, 0, 0, 0.5), get_render_bounds());
	}

	Elements::scroll_bar::scroll_bar(scroll_type _type)
	{
		class self_dragger : public dragger
		{
			scroll_bar* owner;
		public:
			self_dragger(scroll_bar* owner)
			{
				this->owner = owner;
			}
			virtual void on_pos_changed(const vec2& r) override
			{
				base::on_pos_changed(r);
				//  on_move(r);
			}

		};
		padding = { 1, 1, 1, 1 };
		type = _type;
		size = { 10, 10 };
		drag.reset(new self_dragger(this));
		drag->size = { 8, 8 };
		drag->minimal_size = { 8, 8 };
		drag->idle_color = float4(1, 1, 1, 0.5f);   // must stand out on the dark track
		// drag->pos = {1, 1};
		drag->on_move = [this](vec2 _pos, vec2 delta)
		{
			if (on_move)
			{
				const bool  horizontal = type == scroll_type::HORIZONTAL;
				const float travel     = track_length() - (horizontal ? drag->size->x : drag->size->y);
				const float p          = horizontal ? _pos.x : _pos.y;

				on_move(travel > 0 ? Math::clamp(p / travel, 0.0f, 1.0f) : 0.0f);
			}
		};
		add_child(drag);

		if (type == scroll_type::HORIZONTAL)
		{
			drag->size = { 50, 8 };
			height_size = size_type::FIXED;
			drag->height_size = size_type::FIXED;
			drag->allow_y = false;
		}

		else
		{
			drag->size = { 8, 50 };
			width_size = size_type::FIXED;
			drag->width_size = size_type::FIXED;
			drag->allow_x = false;
		}

		drag->clamp_to_parent = ParentClamp::ALL;
		drag_skin = Skin::get().DefaultButton;
	}

}
