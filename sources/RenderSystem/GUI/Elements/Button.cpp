#include "pch_render.h"
#include "Button.h"
import GUI;

namespace GUI
{
    namespace Elements
    {

        void button::draw(Graphics::context& c)
        {
            if (background_style == view_style::DEFAULT)
            {
                //skin->draw(this, c);
                if (is_pressed())
                    renderer->draw(c, skin.Pressed, get_render_bounds());
                else if (is_hovered())
                    renderer->draw(c, skin.Hover, get_render_bounds());
                else
                    renderer->draw(c, skin.Normal, get_render_bounds());
            }

            if (background_style == view_style::FLAT)
            {
                //skin->draw(this, c);
                if (is_pressed())
                    renderer->draw_color(c, float4(1, 1, 1, 0.1), get_render_bounds());
                else if (is_hovered())
                    renderer->draw_color(c, float4(1, 1, 1, 0.3), get_render_bounds());

                //	else
                //	renderer->draw(c, skin.Normal, get_render_bounds());
            }

            label_text->color = pressed ? rgba8(200, 200, 200, 255) : rgba8(255, 255, 255, 255);
        }


        button::button()
        {
            clickable = true;
            clip_child = true;
            label_text.reset(new label());
            label_text->text = "click me!";
            label_text->docking = dock::FILL;
            add_child(label_text);
            padding = {3, 3, 3, 3};
            im.reset(new image());
            im->size = {25, 25};
            im->docking = dock::LEFT;
            im->visible = false;
            add_child(im);
            skin = Skin::get().DefaultButton;
        }

        bool button::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
        {
            base::on_mouse_action(action, button,  pos);

            if (button != mouse_button::LEFT)
                return true;

            label_text->pos = pressed ? vec2(1, 1) : vec2(0, 0);

            if (click_style == click_styles::DEFAULT)
            {
                if (action == mouse_action::UP)
                {
                    on_click(get_ptr<GUI::Elements::button>());
                }
            }

            else if (click_style == click_styles::IMMEDIATELY)
            {
                if (action == mouse_action::DOWN)
                {
                    on_click(get_ptr<GUI::Elements::button>());
                }
            }

            return true;
        }

        void button::on_bounds_changed(const rect& r)
        {
            base::on_bounds_changed(r);
            //label_text->size = vec2(label_text->size->x,80);
        }



    }

}