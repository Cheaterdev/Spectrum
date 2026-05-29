module GUI:ValueBox;
import :Renderer;


namespace GUI
{
    namespace Elements
    {

        button::ptr value_box::create_less_button()
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
            silver_image->size = silver_image->skin.Normal.texture.get_size();
            return silver_image;
        }

        button::ptr value_box::create_more_button()
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
            silver_image->size = silver_image->skin.Normal.texture.get_size();
            return silver_image;
        }

        bool value_box::on_wheel(mouse_wheel type, float value, float2 pos)
        {
            if (value > 0)
                plus_butt->on_click(minus_butt);

            if (value < 0)
                minus_butt->on_click(minus_butt);

            return true;
        }

        void value_box::think(float dt)
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

        value_box::value_box()
        {
            clickable = true;
            thinkable = true;
            minus_butt = create_less_button();
            plus_butt = create_more_button();
            plus_butt->click_style = button::click_styles::IMMEDIATELY;
            minus_butt->click_style = button::click_styles::IMMEDIATELY;
            //minus_butt->size=minus_butt->size.get()/2;
            //plus_butt->size=plus_butt->size.get()/2;
            height_size = size_type::MATCH_CHILDREN;
            size =
            { 110, 0 };
            add_child(minus_butt);
            add_child(plus_butt);
            image::ptr counter(new image());
            counter->texture = Skin::get().DefaultEditBox.Normal;
            //  counter->texture.padding =
            // { 10, 10, 10, 10 };
            counter->size =
            { 10, 24 };
            counter->docking = dock::FILL;
            counter->height_size = size_type::FIXED;
            add_child(counter);
            info.reset(new label);
            info->text = "";
            info->docking = dock::FILL;
            info->color = {0, 0, 0, 200};
            counter->add_child(info);
            //   add_child(info);
            minus_butt->on_click = [this](button::ptr)
            {
                if (current_value > min_value)current_value--;

                on_change(current_value, this);
            };
            plus_butt->on_click = [this](button::ptr)
            {
                if (current_value < max_value)current_value++;

                on_change(current_value, this);
            };
        }
    }
}