module GUI:Tooltip;
import :Renderer;

namespace GUI
{
    namespace Elements
    {
        // ---- tooltip_widget ----

        tooltip_widget::tooltip_widget()
        {
            clickable    = false;
            docking      = dock::NONE;
            x_type       = pos_x_type::LEFT;
            y_type       = pos_y_type::TOP;
            width_size   = size_type::MATCH_CHILDREN;
            height_size  = size_type::MATCH_CHILDREN;
            visible      = false;
            padding      = {6, 4, 6, 4};

            background.reset(new colored_rect());
            background->color       = {0.12f, 0.12f, 0.12f, 0.95f};
            background->docking     = dock::FILL;
            add_child(background);

            text_label.reset(new label());
            text_label->color       = {1, 1, 1, 1};
            text_label->docking     = dock::NONE;
            text_label->x_type      = pos_x_type::LEFT;
            text_label->y_type      = pos_y_type::TOP;
            text_label->width_size  = size_type::MATCH_CHILDREN;
            text_label->height_size = size_type::FIXED;
            text_label->size        = {0, 16};
            text_label->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
            add_child(text_label);
        }

        void tooltip_widget::show(const std::string& text, vec2 screen_pos)
        {
            text_label->text = text;
            pos     = screen_pos;
            visible = true;
        }

        void tooltip_widget::hide()
        {
            visible = false;
        }

        // ---- tooltip_manager ----

        tooltip_manager::tooltip_manager()
        {
            clickable  = false;
            thinkable  = true;
            docking    = dock::NONE;
            visible    = false; // manager itself is invisible
        }

        void tooltip_manager::on_add(base* parent)
        {
            base::on_add(parent);

            // attach the floating overlay directly to user_interface (top level)
            auto ui = get_user_ui();
            if (ui)
            {
                overlay.reset(new tooltip_widget());
                ui->add_child(overlay);
            }
        }

        void tooltip_manager::think(float dt)
        {
            auto ui = get_user_ui();
            if (!ui || !overlay) return;

            auto hovered_ptr = ui->get_hovered().lock();
            base* hovered    = hovered_ptr.get();

            if (hovered != last_hovered)
            {
                last_hovered = hovered;
                timer        = 0.0f;
                if (shown)
                {
                    overlay->hide();
                    shown = false;
                }
            }

            if (!hovered || hovered->tooltip.empty())
            {
                if (shown) { overlay->hide(); shown = false; }
                return;
            }

            if (!shown)
            {
                timer += dt;
                if (timer >= DELAY)
                {
                    vec2 p = ui->get_mouse_pos() + vec2(14, 18);
                    overlay->show(hovered->tooltip, p);
                    overlay->to_front();
                    shown = true;
                }
            }
            else
            {
                // keep following cursor while shown
                overlay->pos = ui->get_mouse_pos() + vec2(14, 18);
            }
        }
    }
}
