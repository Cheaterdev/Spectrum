#include "pch_render.h"
#include "CheckBox.h"
#include "GUI/Renderer/Renderer.h"

namespace GUI
{
	namespace Elements
	{

		void check_box::draw(Graphics::context& c)
		{
			if (is_checked())
				renderer->draw(c, skin.Checked, get_render_bounds());
			else
				renderer->draw(c, skin.Normal, get_render_bounds());

			//   renderer->draw(this, c);
		}

		check_box::check_box() : checked(std::bind(&check_box::on_check_changed, this, std::placeholders::_1))
		{
			size = { 20, 20 };
			height_size = size_type::FIXED;
			width_size = size_type::FIXED;
			clickable = true;
			clip_child = true;
			padding = { 5, 5, 5, 5 };
			skin = Skin::get().DefaultCheckBox;
		}
		check_box::check_box(Variable<bool> & p):check_box()
		{
			set_checked(p);
			on_check = [&p](bool v) {
				p = v;
			};
		}
        bool check_box::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
        {
            base::on_mouse_action(action, button, pos);
            //label_text->pos = pressed ? vec2(1, 1) : vec2(0, 0);

            if (action == mouse_action::UP)
            {
                //if (on_click)
                //	on_click();
                if (allow_uncheck || (!allow_uncheck && !checked.get()))
                    checked = !checked.get();
            }

            return true;
        }

        void check_box::on_bounds_changed(const rect& r)
        {
            base::on_bounds_changed(r);
            //label_text->size = vec2(label_text->size->x,80);
        }

        void check_box::on_check_changed(const bool& r)
        {
            if (on_check_internal)
                on_check_internal(r);

            if (on_check)
                on_check(r);
        }
    }
}