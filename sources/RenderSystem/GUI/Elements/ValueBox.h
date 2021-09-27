#pragma once
#include "Button.h"

namespace GUI
{
    namespace Elements
    {
        class value_box : public base
        {
                button::ptr create_less_button()
                {
                    button::ptr silver_image(new button);
                    silver_image->skin = Skin::get().TabButton;
                    /*	silver_image->skin.Normal = Texture("ui/btn_less2.png");
                    	silver_image->skin.Hover = Texture("ui/btn_less1.png");
                    	silver_image->skin.Pressed = Texture("ui/btn_less0.png");*/
                    silver_image->margin =
                    { 3, 3, 3, 3 };
                    silver_image->get_label()->visible = false;
                    silver_image->docking = GUI::dock::LEFT;
                    silver_image->width_size = size_type::FIXED;
                    silver_image->height_size = size_type::FIXED;
                    silver_image->size = silver_image->skin.Normal.texture->get_size().xy;
                    return silver_image;
                }
                button::ptr create_more_button()
                {
                    button::ptr silver_image(new button);
                    silver_image->skin = Skin::get().TabButton;
                    /*	silver_image->skin.Normal = Texture("ui/btn_less2.png");
                    	silver_image->skin.Hover = Texture("ui/btn_less1.png");
                    	silver_image->skin.Pressed = Texture("ui/btn_less0.png");
                    	*/
                    /*silver_image->pressed_texture.tc = float4(1, 0, 0, 1);
                    silver_image->hover_texture.tc = float4(1, 0, 0, 1);
                    silver_image->normal_texture.tc = float4(1, 0, 0, 1);
                    */
                    silver_image->margin =
                    { 3, 3, 3, 3 };
                    silver_image->get_label()->visible = false;
                    silver_image->docking = GUI::dock::RIGHT;
                    silver_image->width_size = size_type::FIXED;
                    silver_image->height_size = size_type::FIXED;
                    silver_image->size = silver_image->skin.Normal.texture->get_size().xy;
                    return silver_image;
                }
                virtual bool on_wheel(mouse_wheel type, float value, float2 pos) override
                {
                    if (value > 0)
                        plus_butt->on_click(minus_butt);

                    if (value < 0)
                        minus_butt->on_click(minus_butt);

                    return true;
                }
                float cur_time = 0;
                float inner_time = 0;
                virtual void think(float dt) override
                {
                    cur_time += dt;

                    if (minus_butt->is_pressed() || plus_butt->is_pressed())
                    {
                        if (cur_time > 0.5)
                        {
                            inner_time += dt;

                            if (inner_time > 0.1)
                            {
                                if (minus_butt->is_pressed())
                                    minus_butt->on_click(minus_butt);

                                if (plus_butt->is_pressed())
                                    plus_butt->on_click(minus_butt);

                                inner_time = 0;
                            }
                        }

                        else
                            inner_time = 0;
                    }

                    else
                        cur_time = 0;

                    if (current_value > max_good_value)
                        info->color = rgba8(1, 0, 0, 200);
                    else
                        info->color = rgba8(0, 0, 0, 200);
                }
                button::ptr minus_butt;
                button::ptr plus_butt;
            public:
                using ptr = s_ptr<value_box>;
                label::ptr info;

                int min_value = 0;
                int max_value = 100;
                int current_value = 0;
                int max_good_value = 100000;

                std::function<void(int, value_box*)> on_change;
                value_box();
        };
    }
}

