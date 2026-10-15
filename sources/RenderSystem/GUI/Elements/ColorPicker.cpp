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

            width_size  = size_type::FIXED;
            height_size = size_type::FIXED;
            size = {300, 0};

            relayout();
        }

        void color_picker::set_channels(color_channels value)
        {
            channels = value;
            relayout();
            update_display();
        }

        void color_picker::set_display(color_display value)
        {
            display = value;
            relayout();
        }

        // Positions are absolute, so everything is recomputed from the two
        // flag sets: sliders stack from the top for each enabled channel, the
        // swatch sits to their left when shown, and the read-out labels go on
        // one row underneath. The picker's height follows its content.
        void color_picker::relayout()
        {
            const bool show_preview = check(display & color_display::Preview);
            const bool show_hex     = check(display & color_display::Hex);
            const bool show_rgb     = check(display & color_display::Rgb);

            float_slider* sliders[4]   = {slider_r.get(), slider_g.get(), slider_b.get(), slider_a.get()};
            const color_channels bits[4] = {color_channels::R, color_channels::G, color_channels::B, color_channels::A};

            const float w          = size->x;
            const float slider_x   = show_preview ? PREVIEW_SIZE + 8.0f : 0.0f;
            const float slider_w   = w - slider_x - 4.0f;
            const float row_stride = SLIDER_HEIGHT + SLIDER_GAP;

            int rows = 0;

            for (int i = 0; i < 4; i++)
            {
                bool enabled = check(channels & bits[i]);
                sliders[i]->visible = enabled;

                if (!enabled)
                    continue;

                sliders[i]->pos  = {slider_x, rows * row_stride};
                sliders[i]->size = {slider_w, SLIDER_HEIGHT};
                rows++;
            }

            preview->visible = show_preview;
            preview->pos     = {0, 0};

            float content_h = rows ? rows * row_stride - SLIDER_GAP : 0.0f;

            if (show_preview)
                content_h = std::max(content_h, PREVIEW_SIZE);

            hex_label->visible = show_hex;
            rgb_label->visible = show_rgb;

            float height = content_h;

            if (show_hex || show_rgb)
            {
                float label_y = content_h > 0 ? content_h + LABEL_GAP : 0.0f;
                height = label_y + LABEL_HEIGHT;

                if (show_hex && show_rgb)
                {
                    float label_w = w / 2.0f - 4.0f;

                    hex_label->pos  = {0, label_y};
                    hex_label->size = {label_w, LABEL_HEIGHT};
                    rgb_label->pos  = {label_w + 8.0f, label_y};
                    rgb_label->size = {w - label_w - 8.0f, LABEL_HEIGHT};
                }
                else
                {
                    auto* only = show_hex ? hex_label.get() : rgb_label.get();
                    only->pos  = {0, label_y};
                    only->size = {w, LABEL_HEIGHT};
                }
            }

            size = {w, height};
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

            int ai = static_cast<int>(current_color.w * 255.0f + 0.5f);
            const bool alpha = check(channels & color_channels::A);

            char hex[16];

            if (alpha)
                snprintf(hex, sizeof(hex), "#%02X%02X%02X%02X", ri, gi, bi, ai);
            else
                snprintf(hex, sizeof(hex), "#%02X%02X%02X", ri, gi, bi);

            hex_label->text = hex;

            // Only the enabled channels, named by their letters: "rgb(26 26 26)",
            // "rgba(26 26 26 255)", "rg(26 26)".
            std::string names;
            std::string values;
            const int component[4] = {ri, gi, bi, ai};
            const char letters[4]  = {'r', 'g', 'b', 'a'};
            const color_channels bits[4] = {color_channels::R, color_channels::G, color_channels::B, color_channels::A};

            for (int i = 0; i < 4; i++)
            {
                if (!check(channels & bits[i]))
                    continue;

                names += letters[i];
                values += (values.empty() ? "" : " ") + std::to_string(component[i]);
            }

            rgb_label->text = names + "(" + values + ")";
        }

    } // namespace Elements
} // namespace GUI
