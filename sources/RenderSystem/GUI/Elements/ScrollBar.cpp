#include "pch_render.h"
#include "ScrollBar.h"
import GUI;

namespace GUI
{
	void Elements::scroll_bar::draw(Graphics::context& c)
	{
		renderer->draw_color(c, float4(0, 0, 0, 0.5), get_render_bounds());
		renderer->draw_color(c, float4(1, 1, 1, 0.5), drag->get_render_bounds());
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
		// drag->pos = {1, 1};
		drag->on_move = [this](vec2 _pos, vec2 delta)
		{
			if (on_move)
			{
				if (type == scroll_type::HORIZONTAL)
				{
					float w = render_bounds->w - drag->size->x;

					if (w)
						on_move(Math::clamp(_pos.x / w, 0.0f, 1.0f));
					else
						on_move(0.0f);
				}

				else
				{
					float h = (render_bounds->h - drag->size->y);

					if (h)
						on_move(Math::clamp(_pos.y / h, 0.0f, 1.0f));
					else
						on_move(0.0f);
				}
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

		drag->clip_to_parent = ParentClip::ALL;
		drag_skin = Skin::get().DefaultButton;
	}

}
