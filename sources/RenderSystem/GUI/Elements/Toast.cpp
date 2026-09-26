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
            background->color = float4(0.13f, 0.13f, 0.14f, 0.94f * a);
            strip->color      = float4(accent.x, accent.y, accent.z, a);
            message->color    = float4(1, 1, 1, a);
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
                t->set_opacity(std::clamp(std::min(age, t->remaining) / fade_time, 0.0f, 1.0f));
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
