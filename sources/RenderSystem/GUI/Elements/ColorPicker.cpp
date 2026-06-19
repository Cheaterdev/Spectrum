module GUI:ColorPicker;
import :Renderer;

namespace GUI
{
    namespace Elements
    {
        static constexpr float PREVIEW_SIZE  = 64.0f;
        static constexpr float SLIDER_HEIGHT = 14.0f;
        static constexpr float SLIDER_GAP    = 6.0f;
        static constexpr float LABEL_HEIGHT  = 18.0f;
        static constexpr float LABEL_GAP     = 4.0f;

        color_picker::color_picker()
        {
            // Preview square
            preview.reset(new colored_rect());
            preview->color       = {0, 0, 0, 1};
            preview->docking     = dock::NONE;
            preview->x_type      = pos_x_type::LEFT;
            preview->y_type      = pos_y_type::TOP;
            preview->width_size  = size_type::FIXED;
            preview->height_size = size_type::FIXED;
            preview->size        = {PREVIEW_SIZE, PREVIEW_SIZE};
            add_child(preview);

            // R slider
            slider_r.reset(new float_slider());
            slider_r->left_color  = {0, 0, 0, 1};
            slider_r->right_color = {1, 0, 0, 1};
            slider_r->on_change   = [this](float) { on_slider_changed(); };
            add_child(slider_r);

            // G slider
            slider_g.reset(new float_slider());
            slider_g->left_color  = {0, 0, 0, 1};
            slider_g->right_color = {0, 1, 0, 1};
            slider_g->on_change   = [this](float) { on_slider_changed(); };
            add_child(slider_g);

            // B slider
            slider_b.reset(new float_slider());
            slider_b->left_color  = {0, 0, 0, 1};
            slider_b->right_color = {0, 0, 1, 1};
            slider_b->on_change   = [this](float) { on_slider_changed(); };
            add_child(slider_b);

            // A slider
            slider_a.reset(new float_slider());
            slider_a->left_color  = {0, 0, 0, 1};
            slider_a->right_color = {1, 1, 1, 1};
            slider_a->value       = 1.0f;
            slider_a->on_change   = [this](float) { on_slider_changed(); };
            add_child(slider_a);

            // Hex label
            hex_label.reset(new label());
            hex_label->text        = "#000000";
            hex_label->color       = float4(1, 1, 1, 1);
            hex_label->docking     = dock::NONE;
            hex_label->x_type      = pos_x_type::LEFT;
            hex_label->y_type      = pos_y_type::TOP;
            hex_label->width_size  = size_type::FIXED;
            hex_label->height_size = size_type::FIXED;
            add_child(hex_label);

            // RGB label
            rgb_label.reset(new label());
            rgb_label->text        = "rgb(0 0 0)";
            rgb_label->color       = float4(1, 1, 1, 1);
            rgb_label->docking     = dock::NONE;
            rgb_label->x_type      = pos_x_type::LEFT;
            rgb_label->y_type      = pos_y_type::TOP;
            rgb_label->width_size  = size_type::FIXED;
            rgb_label->height_size = size_type::FIXED;
            add_child(rgb_label);

            float default_h = std::max(4 * (SLIDER_HEIGHT + SLIDER_GAP) - SLIDER_GAP, PREVIEW_SIZE)
                            + LABEL_GAP + LABEL_HEIGHT;
            width_size  = size_type::FIXED;
            height_size = size_type::FIXED;
            size = {300, default_h};

            float w          = size->x;
            float slider_x   = PREVIEW_SIZE + 8.0f;
            float slider_w   = w - slider_x - 4.0f;
            float row_stride = SLIDER_HEIGHT + SLIDER_GAP;

            preview->pos = {0, 0};

            float_slider* sliders[4] = {slider_r.get(), slider_g.get(), slider_b.get(), slider_a.get()};
            for (int i = 0; i < 4; i++)
            {
                sliders[i]->pos  = {slider_x, i * row_stride};
                sliders[i]->size = {slider_w, SLIDER_HEIGHT};
            }

            float label_y = PREVIEW_SIZE + LABEL_GAP;
            float label_w = w / 2.0f - 4.0f;

            hex_label->pos  = {0, label_y};
            hex_label->size = {label_w, LABEL_HEIGHT};

            rgb_label->pos  = {label_w + 8.0f, label_y};
            rgb_label->size = {w - label_w - 8.0f, LABEL_HEIGHT};
        }

        float4 color_picker::get_color() const
        {
            return current_color;
        }

        void color_picker::set_color(float4 color)
        {
            setting_color = true;
            current_color = color;
            slider_r->value = color.x;
            slider_g->value = color.y;
            slider_b->value = color.z;
            slider_a->value = color.w;
            setting_color = false;
            update_slider_gradients();
            update_display();
            preview->color = float4(color.x, color.y, color.z, 1);
        }

        void color_picker::on_slider_changed()
        {
            if (setting_color) return;
            current_color = float4(
                slider_r->value,
                slider_g->value,
                slider_b->value,
                slider_a->value);

            update_slider_gradients();
            update_display();
            preview->color = float4(current_color.x, current_color.y, current_color.z, 1);
            on_change(current_color);
        }

        void color_picker::update_slider_gradients()
        {
            float r = current_color.x;
            float g = current_color.y;
            float b = current_color.z;

            slider_r->left_color  = {0, g, b, 1};
            slider_r->right_color = {1, g, b, 1};

            slider_g->left_color  = {r, 0, b, 1};
            slider_g->right_color = {r, 1, b, 1};

            slider_b->left_color  = {r, g, 0, 1};
            slider_b->right_color = {r, g, 1, 1};

            slider_a->left_color  = {r, g, b, 0};
            slider_a->right_color = {r, g, b, 1};
        }

        void color_picker::update_display()
        {
            int ri = static_cast<int>(current_color.x * 255.0f + 0.5f);
            int gi = static_cast<int>(current_color.y * 255.0f + 0.5f);
            int bi = static_cast<int>(current_color.z * 255.0f + 0.5f);

            char hex[16];
            snprintf(hex, sizeof(hex), "#%02X%02X%02X", ri, gi, bi);
            hex_label->text = hex;

            char rgb[32];
            snprintf(rgb, sizeof(rgb), "rgb(%d %d %d)", ri, gi, bi);
            rgb_label->text = rgb;
        }

    } // namespace Elements
} // namespace GUI
