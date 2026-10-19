module GUI:Toast;
import :Renderer;

namespace
{
    struct pending_toast
    {
        std::string                 text;
        GUI::Elements::toast_kind   kind;
        float                       seconds;
    };

    std::mutex                 pending_m;
    std::vector<pending_toast> pending;

    constexpr float fade_time = 0.4f;
    constexpr float slide_in  = 36.0f;   // px, the distance it arrives from
    constexpr float slide_out = 24.0f;   // px, the distance it leaves to
}

namespace GUI
{
    namespace Elements
    {
        // ---- toast_widget ----

        toast_widget::toast_widget(const std::string& text, toast_kind kind, float seconds)
            : remaining(seconds), lifetime(seconds)
        {
            static const float4 accents[] = {
                float4(60, 120, 220, 255) / 255.0f,   // info
                float4(50, 160, 70, 255) / 255.0f,    // success
                float4(220, 150, 0, 255) / 255.0f,    // warning
                float4(210, 50, 50, 255) / 255.0f,    // error
            };
            accent = accents[std::clamp(int(kind), 0, 3)];

            clickable   = true;
            docking     = dock::TOP;
            height_size = size_type::MATCH_CHILDREN;
            margin      = { 0, 6, 0, 0 };
            // Frosted: the dark background below is translucent, so the blur of
            // what is behind the toast shows through it.
            add_effect<BackdropBlur>();
            // No padding: PARENT-docked children are placed inside it, and the
            // background and strip must reach the edges. The text's offset
            // comes from its own margins instead.
            padding     = { 0, 0, 0, 0 };

            // Background and strip follow the toast's size (MATCH_PARENT), which
            // also keeps them out of its fit-to-children height; counted, their
            // height fed back into the toast's and grew it to the window.
            background = std::make_shared<colored_rect>();
            background->docking     = dock::PARENT;
            background->width_size  = size_type::MATCH_PARENT;
            background->height_size = size_type::MATCH_PARENT;
            add_child(background);

            strip = std::make_shared<colored_rect>();
            strip->docking     = dock::PARENT;
            strip->x_type      = pos_x_type::LEFT;
            strip->width_size  = size_type::FIXED;
            strip->height_size = size_type::MATCH_PARENT;
            strip->size        = { 4, 0 };
            add_child(strip);

            message = std::make_shared<label>();
            message->text        = text;
            message->docking     = dock::TOP;
            message->margin      = { 14, 8, 10, 8 };   // clear of the 4px strip
            message->x_type      = pos_x_type::LEFT;
            message->y_type      = pos_y_type::TOP;
            // FIXED: label sizes itself from its text (see tooltip_widget).
            message->width_size  = size_type::FIXED;
            message->height_size = size_type::FIXED;
            message->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
            add_child(message);

            set_opacity(0);
        }

        void toast_widget::set_opacity(float a)
        {
            // As a group the blur, background, strip and text fade together;
            // per-element alpha would let the strip show through the text
            // area and leave the blur at full strength.
            const bool group = user_ui && user_ui->fade_enabled;
            layer_opacity = group ? a : 1.0f;
            const float alpha = group ? 1.0f : a;

            background->color = float4(0.09f, 0.10f, 0.13f, 0.52f * alpha);
            strip->color      = float4(accent.x, accent.y, accent.z, alpha);
            message->color    = float4(1, 1, 1, alpha);
        }

        void toast_widget::set_offset(float dx)
        {
            if (std::abs(dx - offset) < 0.05f)
                return;

            offset = dx;
            // Shifted by margins: the right one gives back what the left one
            // takes, so the width stays the same and only the position moves.
            margin = { dx, 6, -dx, 0 };
            // A margin change only flags the parent; without this the toast moves
            // but its children keep the positions of the last full layout.
            set_update_layout();
        }

        bool toast_widget::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
        {
            base::on_mouse_action(action, button, pos);
            if (button == mouse_button::LEFT && action == mouse_action::UP)
                remaining = std::min(remaining, fade_time);
            return true;
        }

        // ---- toast_manager ----

        void toast_manager::show(std::string text, toast_kind kind, float seconds)
        {
            std::lock_guard<std::mutex> guard(pending_m);
            // Nothing drains the queue until a manager is added; don't let an
            // app without one grow it forever.
            if (pending.size() < 64)
                pending.push_back({ std::move(text), kind, std::max(seconds, 2 * fade_time) });
        }

        toast_manager::toast_manager()
        {
            clickable = false;
            thinkable = true;
            docking   = dock::NONE;
            visible   = false;   // the manager itself draws nothing
        }

        void toast_manager::on_add(base* parent)
        {
            base::on_add(parent);

            if (auto ui = get_user_ui())
            {
                stack = std::make_shared<base>();
                stack->clickable   = false;
                stack->docking     = dock::NONE;
                stack->x_type      = pos_x_type::RIGHT;
                stack->y_type      = pos_y_type::BOTTOM;
                stack->width_size  = size_type::FIXED;
                stack->height_size = size_type::MATCH_CHILDREN;
                stack->size        = { 360, 0 };
                stack->margin      = { 0, 0, 16, 16 };
                // The toasts slide in from beyond its right edge.
                stack->clip_child  = false;
                ui->add_child(stack);
            }
        }

        void toast_manager::think(float dt)
        {
            if (!stack) return;

            std::vector<pending_toast> incoming;
            {
                std::lock_guard<std::mutex> guard(pending_m);
                incoming.swap(pending);
            }

            for (auto& p : incoming)
            {
                auto t = std::make_shared<toast_widget>(p.text, p.kind, p.seconds);
                stack->add_child(t);
                toasts.push_back(t);
            }
            if (!incoming.empty())
                stack->to_front();

            // Over the limit: the oldest start fading out now.
            for (size_t i = 0; i + max_visible < toasts.size(); i++)
                toasts[i]->remaining = std::min(toasts[i]->remaining, fade_time);

            for (auto& t : toasts)
            {
                t->remaining -= dt;
                const float age = t->lifetime - t->remaining;
                const float in  = std::clamp(age / fade_time, 0.0f, 1.0f);
                const float out = std::clamp(t->remaining / fade_time, 0.0f, 1.0f);
                auto smooth = [](float x) { return x * x * (3.0f - 2.0f * x); };

                t->set_opacity(smooth(std::min(in, out)));

                // Slides in from the right, and a little way back out when it leaves.
                t->set_offset(slide_in * (1.0f - smooth(in)) + slide_out * (1.0f - smooth(out)));
            }

            // Removed from the UI thread's queue rather than here: think() runs
            // inside the UI's own iteration over its elements.
            for (auto it = toasts.begin(); it != toasts.end();)
            {
                if ((*it)->remaining > 0) { ++it; continue; }
                run_on_ui([t = *it]() { t->remove_from_parent(); });
                it = toasts.erase(it);
            }
        }
    }
}
