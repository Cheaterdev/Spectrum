module GUI:Ribbon;
import :Renderer;
import :Toast;

namespace
{
    // A label sized from its text (FIXED, like the toast's): font size first, then the
    // text, so the measure happens once at the right size.
    GUI::Elements::label::ptr make_label(const std::string& text, float font_size, int magnet)
    {
        auto l = std::make_shared<GUI::Elements::label>();
        l->font_size = font_size;
        l->text = text;
        l->magnet_text = magnet;
        l->width_size = GUI::size_type::FIXED;
        l->height_size = GUI::size_type::FIXED;
        return l;
    }

    constexpr int label_flags = FW1_LEFT | FW1_TOP | FW1_NOWORDWRAP;

    // TEMP-POPUPLOG: see Base.cpp.
    void popup_log(const std::string& line)
    {
        static std::mutex log_mutex;
        std::lock_guard<std::mutex> guard(log_mutex);
        std::ofstream out("popup_log.temp", std::ios::app);
        out << line << "\n";
    }
}

namespace GUI
{
    namespace Elements
    {
        // ---- ribbon_button ----

        ribbon_button::ribbon_button(std::string icon_text, std::string label_text, size_style start_style)
            : style(start_style), text(std::move(label_text))
        {
            clickable = true;

            icon_label = make_label(icon_text, metrics.icon_large, label_flags);
            text_label = make_label(text, metrics.text_size, label_flags);
            add_child(icon_label);
            add_child(text_label);

            auto& light = add_effect<LightOverlay>();
            light.reveal = true;
            light.border = true;
            light.ripple = true;
            light.radius = 4.0f;

            relayout();
        }

        void ribbon_button::set_metrics(const ribbon_metrics& new_metrics)
        {
            metrics = new_metrics;
            relayout();
        }

        void ribbon_button::set_style(size_style new_style)
        {
            style = new_style;
            relayout();
        }

        void ribbon_button::set_dropdown(menu_list::ptr menu)
        {
            dropdown = std::move(menu);
            relayout();
        }

        void ribbon_button::set_icon_text(std::string new_icon)
        {
            icon_label->text = new_icon;
            relayout();
        }

        void ribbon_button::set_text(std::string new_text)
        {
            text = std::move(new_text);
            text_label->text = text;
            relayout();
        }

        void ribbon_button::set_width(float w)
        {
            width = w;
            size = vec2(width, size->y);
        }

        void ribbon_button::relayout()
        {
            const bool large     = style == size_style::large;
            const bool icon_only = style == size_style::icon_only;
            const bool has_icon  = !icon_label->text.get().empty();
            const bool has_text  = !icon_only && !text.empty();

            icon_label->font_size = large ? metrics.icon_large : metrics.icon_small;
            text_label->font_size = metrics.text_size;
            icon_label->visible = has_icon;
            text_label->visible = has_text;

            const vec2 icon_size = icon_label->size.get();
            const vec2 text_size = text_label->size.get();
            const float arrow    = dropdown ? 1.0f : 0.0f;

            float height = metrics.small_button;

            if (large)
            {
                // The text sticks to the bottom, so the labels of neighbouring buttons line
                // up whatever their icons measure; the icon is centred in what is left above.
                // The same band under the text for every large button, with or without a
                // drop-down arrow (drawn in that band), so neighbours' labels share a height.
                const float bottom = 9.0f;
                const float text_band = has_text ? text_size.y + bottom : bottom;
                const float top = std::max(2.0f, (metrics.large_button - text_band - icon_size.y) * 0.5f);

                icon_label->docking = dock::TOP;
                icon_label->x_type  = pos_x_type::CENTER;
                icon_label->y_type  = pos_y_type::TOP;
                icon_label->margin  = { 0, top, 0, 0 };

                text_label->docking = dock::BOTTOM;
                text_label->x_type  = pos_x_type::CENTER;
                text_label->y_type  = pos_y_type::TOP;
                text_label->margin  = { 0, 0, 0, bottom };

                width  = std::max(metrics.large_button * 0.78f, std::max(has_icon ? icon_size.x : 0.0f, has_text ? text_size.x : 0.0f) + 2.0f * metrics.pad);
                height = metrics.large_button;
            }
            else if (icon_only)
            {
                width = std::max(metrics.small_button, icon_size.x + 12.0f + arrow * 10.0f);

                icon_label->docking = dock::LEFT;
                icon_label->x_type  = pos_x_type::LEFT;
                icon_label->y_type  = pos_y_type::CENTER;
                icon_label->margin  = { (width - icon_size.x - arrow * 10.0f) * 0.5f, 0, 0, 0 };
            }
            else
            {
                icon_label->docking = dock::LEFT;
                icon_label->x_type  = pos_x_type::LEFT;
                icon_label->y_type  = pos_y_type::CENTER;
                icon_label->margin  = { 6, 0, has_text ? 4.0f : 6.0f, 0 };

                text_label->docking = dock::LEFT;
                text_label->x_type  = pos_x_type::LEFT;
                text_label->y_type  = pos_y_type::CENTER;
                text_label->margin  = { 0, 0, 6, 0 };

                width = 6.0f + (has_icon ? icon_size.x : 0.0f) + (has_icon && has_text ? 4.0f : 0.0f)
                      + (has_text ? text_size.x + 6.0f : 6.0f) + arrow * 10.0f;
            }

            width_size  = size_type::FIXED;
            height_size = size_type::FIXED;
            size        = vec2(width, height);
        }

        void ribbon_button::activate()
        {
            popup_log("ribbon_button::activate '" + text + "' dropdown=" + std::to_string(dropdown != nullptr) + (dropdown ? " open=" + std::to_string(dropdown->is_self_open()) : std::string()));   // TEMP-POPUPLOG

            if (dropdown)
            {
                // The press that got here has already closed an open menu (it is
                // fading out), so an open one means: this click closes it.
                if (dropdown->is_self_open())
                {
                    dropdown->self_close();
                    return;
                }

                const rect b = get_render_bounds();
                dropdown->pos = vec2(b.x, b.y + b.h);
                dropdown->self_open(user_ui);
                return;
            }

            if (on_click)
            {
                on_click();

                // A button inside a collapsed group's popup: the popup is done.
                if (user_ui)
                    user_ui->close_menus();
            }
        }

        void ribbon_button::on_pre_render(Context& c)
        {
            base::on_pre_render(c);

            // Asked here, on the thread that builds the draw list, so the callbacks
            // read the application's state from the UI thread like any other handler.
            enabled_now = !is_enabled || is_enabled();
            checked_now = is_checked && is_checked();
            clickable   = enabled_now;

            const float4 tint = enabled_now ? float4(1, 1, 1, 1) : float4(1, 1, 1, 0.38f);
            icon_label->color = tint;
            text_label->color = tint;
        }

        void ribbon_button::draw(Context& c)
        {
            const rect b = get_render_bounds();

            if (enabled_now)
            {
                if (checked_now)
                    c.renderer->draw_color(c, float4(0.21f, 0.45f, 0.79f, 0.45f), b);

                if (is_pressed())
                    c.renderer->draw_color(c, float4(1, 1, 1, 0.14f), b);
                else if (is_hovered())
                    c.renderer->draw_color(c, float4(1, 1, 1, 0.07f), b);
            }

            if (dropdown)
            {
                // A small triangle: rows that narrow to a point. Under the text on a
                // large button, at the right edge on the others.
                const float s = c.scale;
                const float cx = style == size_style::large ? b.x + b.w * 0.5f : b.x + b.w - 9.0f * s;
                const float cy = style == size_style::large ? b.y + b.h - 8.0f * s : b.y + b.h * 0.5f - 3.0f * s;
                const float4 color = float4(1, 1, 1, enabled_now ? 0.8f : 0.3f);

                for (int i = 0; i < 4; i++)
                {
                    const float w = (7.0f - 2.0f * i) * s;
                    c.renderer->draw_color(c, color, rect(cx - w * 0.5f, cy + i * 1.5f * s, w, 1.5f * s));
                }
            }
        }

        bool ribbon_button::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
        {
            base::on_mouse_action(action, button, pos);

            if (button == mouse_button::LEFT && action == mouse_action::UP && enabled_now)
                activate();

            return true;
        }

        bool ribbon_button::on_long_pressed(const touch&)
        {
            // No hover on a phone: a long press says what the button does.
            const std::string tip = !tooltip.empty() ? tooltip : text;
            if (!tip.empty())
                toast_manager::show(tip, toast_kind::info, 2.0f);
            return true;
        }

        // ---- ribbon_group ----

        ribbon_group::ribbon_group(std::string group_title, const ribbon_metrics& start_metrics)
            : metrics(start_metrics), title(std::move(group_title))
        {
            clickable = false;
            docking   = dock::LEFT;
            width_size = size_type::MATCH_CHILDREN;
            margin    = { 2, 3, 2, 3 };

            caption = make_label(title, metrics.caption_size, label_flags);
            caption->color   = float4(1, 1, 1, 0.55f);
            caption->docking = dock::BOTTOM;
            caption->x_type  = pos_x_type::CENTER;
            caption->margin  = { 0, 0, 0, 2 };
            add_child(caption);

            body = std::make_shared<base>();
            body->clickable   = false;
            body->docking     = dock::TOP;
            body->width_size  = size_type::MATCH_CHILDREN;
            body->height_size = size_type::FIXED;
            body->size        = vec2(0, metrics.large_button);
            add_child(body);
        }

        void ribbon_group::add_entry(entry e)
        {
            if (break_next)
            {
                e.break_before = true;
                break_next = false;
            }

            entries.push_back(std::move(e));
            rebuild();
        }

        ribbon_button::ptr ribbon_group::add_large(std::string icon, std::string text, std::function<void()> on_click, menu_list::ptr dropdown)
        {
            auto b = std::make_shared<ribbon_button>(std::move(icon), std::move(text), ribbon_button::size_style::large);
            b->set_metrics(metrics);
            if (dropdown)
                b->set_dropdown(std::move(dropdown));
            b->on_click = std::move(on_click);

            entry e;
            e.button   = b;
            e.authored = ribbon_button::size_style::large;
            add_entry(std::move(e));
            return b;
        }

        ribbon_button::ptr ribbon_group::add_small(std::string icon, std::string text, std::function<void()> on_click, menu_list::ptr dropdown)
        {
            auto b = std::make_shared<ribbon_button>(std::move(icon), std::move(text), ribbon_button::size_style::small);
            b->set_metrics(metrics);
            if (dropdown)
                b->set_dropdown(std::move(dropdown));
            b->on_click = std::move(on_click);

            entry e;
            e.button   = b;
            e.authored = ribbon_button::size_style::small;
            add_entry(std::move(e));
            return b;
        }

        ribbon_button::ptr ribbon_group::add_icon(std::string icon, std::function<void()> on_click, menu_list::ptr dropdown)
        {
            auto b = std::make_shared<ribbon_button>(std::move(icon), std::string(), ribbon_button::size_style::icon_only);
            b->set_metrics(metrics);
            if (dropdown)
                b->set_dropdown(std::move(dropdown));
            b->on_click = std::move(on_click);

            entry e;
            e.button   = b;
            e.authored = ribbon_button::size_style::icon_only;
            add_entry(std::move(e));
            return b;
        }

        ribbon_button::ptr ribbon_group::add_menu_item(menu_list_element::ptr item, std::string icon, bool large)
        {
            if (!item)
                return nullptr;

            std::function<void()> click;
            menu_list::ptr submenu;

            if (item->has_menu())
                submenu = item->get_menu();
            else
                click = [item] { if (item->on_click) item->on_click(item); };

            return large ? add_large(std::move(icon), item->text, std::move(click), std::move(submenu))
                         : add_small(std::move(icon), item->text, std::move(click), std::move(submenu));
        }

        void ribbon_group::end_stack()
        {
            break_next = true;
        }

        void ribbon_group::add_widget(base::ptr widget)
        {
            entry e;
            e.widget = std::move(widget);
            add_entry(std::move(e));
        }

        // The buttons of one stack share the widest one's width, so the column reads as a block.
        void ribbon_group::fit_stack(base::ptr column)
        {
            float w = 0.0f;
            for (int i = 0; i < column->get_child_count(); i++)
                w = std::max(w, static_cast<ribbon_button*>(column->get_child(i))->natural_width());

            for (int i = 0; i < column->get_child_count(); i++)
                static_cast<ribbon_button*>(column->get_child(i))->set_width(w);

            column->size = vec2(w, metrics.large_button);
        }

        // Lays the entries out as columns for the current level: large buttons alone
        // (only at level 0), small and icon-only ones stacked three high, widgets alone.
        void ribbon_group::rebuild()
        {
            // What was in the body stays alive for a couple of frames: the previous frame
            // may still be recording draws that point at it.
            graveyard[1].clear();
            graveyard[1].swap(graveyard[0]);
            for (int i = 0; i < body->get_child_count(); i++)
                graveyard[0].push_back(body->get_child(i)->get_ptr());

            body->remove_all();

            // The collapsed form shows the full layout, inside its popup.
            const int shown = level >= max_level ? 0 : level;

            base::ptr column;
            int count = 0;
            columns_width = 0.0f;
            std::vector<base::ptr> stacks;

            auto open_column = [&]
            {
                column = std::make_shared<base>();
                column->clickable   = false;
                column->docking     = dock::LEFT;
                column->width_size  = size_type::FIXED;
                column->height_size = size_type::FIXED;
                column->size        = vec2(10, metrics.large_button);
                body->add_child(column);
                stacks.push_back(column);
                count = 0;
            };

            for (auto& e : entries)
            {
                if (e.widget)
                {
                    column.reset();
                    e.widget->docking = dock::LEFT;
                    body->add_child(e.widget);
                    columns_width += e.widget->size->x;
                    continue;
                }

                ribbon_button::size_style style = e.authored;
                if (shown == 1 && style == ribbon_button::size_style::large)
                    style = ribbon_button::size_style::small;
                if (shown == 2)
                    style = ribbon_button::size_style::icon_only;

                e.button->set_style(style);

                if (style == ribbon_button::size_style::large)
                {
                    column.reset();
                    e.button->docking = dock::LEFT;
                    body->add_child(e.button);
                    columns_width += e.button->natural_width();
                    continue;
                }

                if (!column || count >= 3 || (shown == 0 && e.break_before))
                    open_column();

                e.button->docking = dock::TOP;
                column->add_child(e.button);
                count++;
            }

            for (auto& s : stacks)
            {
                fit_stack(s);
                columns_width += s->size->x;
            }
        }

        float ribbon_group::measure_width()
        {
            if (level >= max_level && collapsed_button)
                return collapsed_button->natural_width() + 4.0f;

            return std::max(columns_width, caption->size.get().x) + 4.0f;
        }

        void ribbon_group::set_level(int new_level)
        {
            new_level = std::clamp(new_level, 0, max_level);
            if (new_level == level)
                return;

            const bool was_collapsed = level >= max_level;
            level = new_level;
            const bool collapsed = level >= max_level;

            if (collapsed != was_collapsed)
                host_body_in_popup(collapsed);

            rebuild();
        }

        // The collapsed form: the body moves into the popup of a single button that
        // carries the group's title, and back again when there is room.
        void ribbon_group::host_body_in_popup(bool in_popup)
        {
            if (in_popup)
            {
                if (!popup)
                {
                    popup = std::make_shared<menu_list>();

                    std::string glyph = icon;
                    if (glyph.empty())
                        for (auto& e : entries)
                            if (e.button && !e.button->get_icon().empty())
                            {
                                glyph = e.button->get_icon();
                                break;
                            }

                    collapsed_button = std::make_shared<ribbon_button>(glyph, title, ribbon_button::size_style::large);
                    collapsed_button->set_metrics(metrics);
                    collapsed_button->set_dropdown(popup);
                    collapsed_button->docking = dock::LEFT;
                    add_child(collapsed_button);
                }

                remove_child(body);
                popup->contents->add_child(body);
                collapsed_button->visible = true;
                caption->visible = false;
            }
            else
            {
                if (popup->is_self_open())
                    popup->self_close();

                popup->contents->remove_child(body);
                add_child(body);
                collapsed_button->visible = false;
                caption->visible = true;
            }
        }

        void ribbon_group::set_icon(std::string glyph)
        {
            icon = std::move(glyph);

            if (collapsed_button)
                collapsed_button->set_icon_text(icon);
        }

        void ribbon_group::set_metrics(const ribbon_metrics& new_metrics)
        {
            metrics = new_metrics;
            caption->font_size = metrics.caption_size;
            body->size = vec2(body->size->x, metrics.large_button);

            for (auto& e : entries)
                if (e.button)
                    e.button->set_metrics(metrics);

            if (collapsed_button)
                collapsed_button->set_metrics(metrics);

            rebuild();
        }

        // ---- ribbon_page ----

        ribbon_page::ribbon_page(const ribbon_metrics& start_metrics)
            : metrics(start_metrics)
        {
            clickable = false;
            docking   = dock::FILL;
            padding   = { 4, 0, 4, 0 };
        }

        ribbon_group::ptr ribbon_page::add_group(std::string title)
        {
            if (!groups.empty())
            {
                auto separator = std::make_shared<colored_rect>();
                separator->color       = float4(1, 1, 1, 0.12f);
                separator->docking     = dock::LEFT;
                separator->width_size  = size_type::FIXED;
                separator->height_size = size_type::FIXED;
                separator->size        = vec2(1, metrics.large_button);
                separator->y_type      = pos_y_type::CENTER;
                separator->margin      = { 3, 0, 3, 0 };
                add_child(separator);
                separators.push_back(separator);
            }

            auto group = std::make_shared<ribbon_group>(std::move(title), metrics);
            add_child(group);
            groups.push_back(group);
            return group;
        }

        void ribbon_page::set_metrics(const ribbon_metrics& new_metrics)
        {
            metrics = new_metrics;

            for (auto& g : groups)
                g->set_metrics(metrics);

            for (auto& s : separators)
                s->size = vec2(1, metrics.large_button);
        }

        // The groups' widths plus the separators between them (1 px and 3 px either side).
        float ribbon_page::measure_total()
        {
            float total = 0.0f;
            for (auto& g : groups)
                total += g->measure_width();

            if (!groups.empty())
                total += float(groups.size() - 1) * 7.0f;

            return total;
        }

        void ribbon_page::fit(float available, bool reset)
        {
            if (reset)
                for (auto& g : groups)
                    g->set_level(0);

            // Round by round: first every group that has to goes one level down, right to
            // left, stopping as soon as it fits; then another level, and so on.
            for (int target = 1; target <= ribbon_group::max_level && measure_total() > available; target++)
                for (int i = int(groups.size()) - 1; i >= 0 && measure_total() > available; i--)
                    if (groups[i]->get_level() < target)
                        groups[i]->set_level(target);
        }

        int ribbon_page::content_signature() const
        {
            int signature = int(groups.size());
            for (auto& g : groups)
                signature = signature * 31 + g->entry_count();
            return signature;
        }

        // ---- ribbon_tab_header ----

        ribbon_tab_header::ribbon_tab_header(const std::string& title, const ribbon_metrics& metrics)
        {
            clickable = true;
            docking   = dock::LEFT;

            text_label = make_label(title, metrics.text_size, label_flags);
            add_child(text_label);

            set_metrics(metrics);
        }

        void ribbon_tab_header::set_metrics(const ribbon_metrics& metrics)
        {
            text_label->font_size = metrics.text_size;
            const vec2 text_size = text_label->size.get();

            text_label->docking = dock::LEFT;
            text_label->x_type  = pos_x_type::LEFT;
            text_label->y_type  = pos_y_type::CENTER;
            text_label->margin  = { metrics.pad + 4.0f, 0, 0, 0 };

            width_size  = size_type::FIXED;
            height_size = size_type::FIXED;
            size        = vec2(text_size.x + 2.0f * metrics.pad + 8.0f, metrics.tab_height);
        }

        void ribbon_tab_header::on_pre_render(Context& c)
        {
            base::on_pre_render(c);
            clock += c.delta_time;
            text_label->color = selected ? float4(1, 1, 1, 1) : float4(1, 1, 1, 0.72f);
        }

        void ribbon_tab_header::draw(Context& c)
        {
            const rect b = get_render_bounds();

            if (is_hovered() && !selected)
                c.renderer->draw_color(c, float4(1, 1, 1, 0.08f), b);

            if (accent.w > 0.0f)
                c.renderer->draw_color(c, accent, rect(b.x, b.y, b.w, 2.0f * c.scale));
        }

        bool ribbon_tab_header::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
        {
            base::on_mouse_action(action, button, pos);

            if (button == mouse_button::LEFT && action == mouse_action::UP)
            {
                // Frame time, not the wall clock: the same on a slow frame, and testable.
                if (clock - last_click < 0.35f && on_double_click)
                {
                    last_click = -10.0f;
                    on_double_click();
                }
                else
                {
                    last_click = clock;
                    if (on_select)
                        on_select();
                }
            }

            return true;
        }

        // ---- ribbon ----

        ribbon::ribbon()
        {
            // Clickable so a press on an empty part of the ribbon counts as a click on
            // something: that is what closes open menus and popups.
            clickable   = true;
            thinkable   = true;
            docking     = dock::TOP;
            height_size = size_type::MATCH_CHILDREN;
            x_type      = pos_x_type::LEFT;

            // Follows the ribbon's size (MATCH_PARENT), which also keeps it out of the
            // fit-to-children height, as the toast's background does.
            auto background = std::make_shared<colored_rect>();
            background->color       = float4(0.10f, 0.11f, 0.15f, 1.0f);
            background->docking     = dock::PARENT;
            background->width_size  = size_type::MATCH_PARENT;
            background->height_size = size_type::MATCH_PARENT;
            add_child(background);

            bar = std::make_shared<base>();
            bar->clickable   = false;
            bar->docking     = dock::TOP;
            bar->height_size = size_type::FIXED;
            bar->size        = vec2(0, metrics.tab_height);
            add_child(bar);

            body = std::make_shared<base>();
            body->clickable   = false;
            body->docking     = dock::TOP;
            body->height_size = size_type::FIXED;
            body->size        = vec2(0, metrics.strip_height);
            body->clip_child  = true;
            add_child(body);
            body_height = metrics.strip_height;

            collapse_button = std::make_shared<button>();
            collapse_button->background_style = button::view_style::FLAT;
            collapse_button->docking     = dock::RIGHT;
            collapse_button->width_size  = size_type::FIXED;
            collapse_button->height_size = size_type::FIXED;
            collapse_button->size        = vec2(metrics.tab_height, metrics.tab_height);
            collapse_button->padding     = { 8, 8, 8, 8 };
            collapse_button->tooltip     = "Collapse the ribbon";
            collapse_button->on_click    = [this](button::ptr) { set_minimized(!minimized); };
            bar->add_child(collapse_button);
            sync_collapse_icon();
        }

        ribbon_page::ptr ribbon::add_tab(std::string title, bool contextual, float4 accent)
        {
            tab_info t;
            t.header = std::make_shared<ribbon_tab_header>(title, metrics);
            if (contextual)
                t.header->accent = accent;

            const int index = int(tabs.size());
            // The ribbon owns its headers, so it outlives the callback.
            t.header->on_select = [this, index]()
            {
                select(index);

                // Minimised: the strip peeks out as a popup, like a menu.
                if (minimized)
                    open_strip();
            };
            // Same as the arrow button, in both directions.
            t.header->on_double_click = [this]() { set_minimized(!minimized); };
            bar->add_child(t.header);

            t.page = std::make_shared<ribbon_page>(metrics);
            t.page->visible = false;
            body->add_child(t.page);

            auto result = t.page;
            tabs.push_back(std::move(t));

            if (current < 0)
                select(index);

            return result;
        }

        ribbon_page::ptr ribbon::page(int index) const
        {
            return index >= 0 && index < int(tabs.size()) ? tabs[index].page : nullptr;
        }

        void ribbon::select(int index)
        {
            if (index < 0 || index >= int(tabs.size()) || !tabs[index].visible || index == current)
                return;

            const bool animate = current >= 0 && !minimized && user_ui && user_ui->fade_enabled && user_ui->window_animations;

            if (current >= 0)
            {
                tabs[current].page->visible = false;
                tabs[current].header->selected = false;
            }

            current = index;
            tabs[index].header->selected = true;
            tabs[index].page->visible = true;

            if (animate)
            {
                // The new strip fades in while it slides a few pixels in from the right (think).
                switch_t = 0.0f;
                tabs[index].page->layer_opacity = 0.0f;
                tabs[index].page->fade_to(1.0f, 0.14f);
            }
            else
            {
                switch_t = 1.0f;
                tabs[index].page->layer_opacity = 1.0f;
                tabs[index].page->margin = { 0, 0, 0, 0 };
            }
        }

        void ribbon::set_tab_visible(int index, bool value)
        {
            if (index < 0 || index >= int(tabs.size()) || tabs[index].visible == value)
                return;

            tabs[index].visible = value;
            tabs[index].header->visible = value;

            if (!value && current == index)
            {
                tabs[index].page->visible = false;
                tabs[index].header->selected = false;
                current = -1;

                for (int i = 0; i < int(tabs.size()); i++)
                    if (tabs[i].visible)
                    {
                        select(i);
                        break;
                    }
            }
        }

        bool ribbon::is_tab_visible(int index) const
        {
            return index >= 0 && index < int(tabs.size()) && tabs[index].visible;
        }

        void ribbon::set_minimized(bool value)
        {
            minimized = value;
            sync_collapse_icon();

            // Back in the ribbon at once; hosting in the popup waits until the strip has
            // eased shut (think).
            if (!value)
                host_strip_in_popup(false);
        }

        // While minimised the strip sits in a popup (a menu: it closes on a click
        // anywhere else) under the tab bar, so it can overlap what is below instead of
        // pushing it down, and dismissing it needs no code of its own.
        void ribbon::host_strip_in_popup(bool in_popup)
        {
            if (in_popup == body_in_popup)
                return;

            body_in_popup = in_popup;

            if (in_popup)
            {
                if (!strip_popup)
                    strip_popup = std::make_shared<menu_list>();

                remove_child(body);
                body->width_size = size_type::FIXED;
                body->size = vec2(get_render_bounds().w / std::max(result_scale, 0.001f), metrics.strip_height);
                strip_popup->contents->add_child(body);
            }
            else
            {
                if (strip_popup->is_self_open())
                    strip_popup->self_close();

                strip_popup->contents->remove_child(body);
                body->width_size = size_type::NONE;
                body->size = vec2(0, body_height);
                add_child(body);
            }
        }

        void ribbon::open_strip()
        {
            if (!minimized || !body_in_popup || !strip_popup)
                return;

            const rect b = bar->get_render_bounds();
            body->size = vec2(b.w / std::max(result_scale, 0.001f), metrics.strip_height);
            strip_popup->pos = vec2(b.x, b.y + b.h);
            strip_popup->self_open(user_ui);
        }

        void ribbon::sync_collapse_icon()
        {
            collapse_button->set_icon(minimized ? "chevron_down" : "chevron_up");
            collapse_button->tooltip = minimized ? "Expand the ribbon" : "Collapse the ribbon";
        }

        void ribbon::set_touch_mode(bool value)
        {
            touch_mode = value;
            metrics = value ? ribbon_metrics::for_touch() : ribbon_metrics();
            apply_metrics();
        }

        void ribbon::apply_metrics()
        {
            bar->size = vec2(bar->size->x, metrics.tab_height);
            collapse_button->size = vec2(metrics.tab_height, metrics.tab_height);

            for (auto& t : tabs)
            {
                t.header->set_metrics(metrics);
                t.page->set_metrics(metrics);
            }
            // The strip height follows in think().

            if (body_in_popup)
                body->size = vec2(body->size->x, metrics.strip_height);
        }

        // Refits the groups when the ribbon's width or what is in it has changed. A
        // growing ribbon starts again from the roomiest forms; a shrinking one only
        // takes groups further down.
        void ribbon::fit_pages()
        {
            const float available = get_render_bounds().w / std::max(result_scale, 0.001f) - 8.0f;   // the page's side padding

            // Not laid out yet.
            if (available <= 8.0f)
                return;

            int signature = touch_mode ? 1 : 0;
            for (auto& t : tabs)
                signature = signature * 31 + t.page->content_signature();

            const bool content_changed = signature != fitted_signature;
            if (!content_changed && std::abs(available - fitted_width) < 0.5f)
                return;

            const bool reset = content_changed || available > fitted_width;
            fitted_width = available;
            fitted_signature = signature;

            for (auto& t : tabs)
                t.page->fit(available, reset);
        }

        void ribbon::think(float dt)
        {
            fit_pages();

            const bool animate = user_ui && user_ui->window_animations;

            const float target = minimized ? 0.0f : metrics.strip_height;
            if (body_height != target)
            {
                if (!animate)
                    body_height = target;
                else
                {
                    const float step = dt / 0.16f * std::max(metrics.strip_height, 1.0f);
                    body_height = body_height < target ? std::min(body_height + step, target) : std::max(body_height - step, target);
                }
                body->size = vec2(body->size->x, body_height);
            }

            // Shut: from here on the strip lives in the popup.
            if (minimized && body_height <= 0.0f)
                host_strip_in_popup(true);

            if (switch_t < 1.0f && current >= 0)
            {
                switch_t = std::min(1.0f, switch_t + dt / 0.14f);
                const float eased = 1.0f - (1.0f - switch_t) * (1.0f - switch_t);
                const float dx = 18.0f * (1.0f - eased);
                tabs[current].page->margin = { dx, 0, -dx, 0 };
            }
        }

        void ribbon::on_pre_render(Context& c)
        {
            base::on_pre_render(c);

            // Minimised with the strip closed, no tab is current as far as the eye goes: its
            // title must not look selected, so that it highlights on hover like the others.
            if (current >= 0)
                tabs[current].header->selected = !minimized || (strip_popup && strip_popup->get_raw_parent());

            if (current < 0)
            {
                underline_valid = false;
                return;
            }

            const rect hb = tabs[current].header->get_render_bounds();
            if (!underline_valid)
            {
                underline_x = hb.x;
                underline_w = hb.w;
                underline_valid = true;
                return;
            }

            // Exponential ease, frame-rate independent: about 90% of a jump in 0.1 s.
            const float k = user_ui && user_ui->window_animations ? 1.0f - std::exp(-c.delta_time * 23.0f) : 1.0f;
            underline_x += (hb.x - underline_x) * k;
            underline_w += (hb.w - underline_w) * k;
        }

        void ribbon::draw_after(Context& c)
        {
            const rect b = bar->get_render_bounds();

            c.renderer->draw_color(c, float4(1, 1, 1, 0.08f), rect(b.x, b.y + b.h - c.scale, b.w, c.scale));

            // The underline goes with the "selected" look, which on_pre_render clears while
            // minimised with the strip closed.
            if (current < 0 || !underline_valid || !tabs[current].header->selected)
                return;

            const float h = 2.0f * c.scale;
            c.renderer->draw_color(c, float4(0.21f, 0.45f, 0.79f, 1.0f), rect(underline_x, b.y + b.h - h, underline_w, h));
        }
    }
}
