export module GUI:Ribbon;
import :Base;
import :Label;
import :MenuList;
import :ColoredRect;
import :Button;
import :VectorIcon;

export namespace GUI
{
    namespace Elements
    {
        // Sizes of the ribbon's parts in UI px. The touch set is finger-sized.
        struct ribbon_metrics
        {
            float tab_height     = 28.0f;
            float strip_height   = 96.0f;    // groups, their captions and margins
            float caption_height = 18.0f;
            float large_button   = 72.0f;    // a large button; three small ones stack to the same
            float small_button   = 24.0f;
            float icon_large     = 26.0f;
            float icon_small     = 15.0f;
            float text_size      = 13.0f;
            float caption_size   = 11.0f;
            float pad            = 8.0f;

            static ribbon_metrics for_touch()
            {
                ribbon_metrics m;
                m.tab_height   = 40.0f;
                m.strip_height = 136.0f;
                m.large_button = 108.0f;
                m.small_button = 36.0f;
                m.icon_large   = 36.0f;
                m.icon_small   = 20.0f;
                m.text_size    = 16.0f;
                m.caption_size = 12.0f;
                m.pad          = 12.0f;
                return m;
            }
        };

        // The small badge with a letter that the ribbon shows over a tab or a button while
        // the keyboard is in charge (Alt). Positioned by the element that owns it.
        class ribbon_key_tip : public base
        {
            public:
                using ptr = s_ptr<ribbon_key_tip>;

                ribbon_key_tip();
                void set_text(const std::string& text);

                virtual void draw(Context& c) override;

            private:
                label::ptr text_label;
        };

        // A command in a group: an icon (a glyph or an emoji, drawn as text) and a
        // label. Large (icon over text), small (icon beside text) or icon only. Plain
        // callbacks, no registry: a button that toggles gives is_checked, one that
        // can be unavailable gives is_enabled, both asked once per frame. A drop-down
        // makes it a menu button; the menu opens under it. A split button has both: the
        // main part (the icon, or the left of a small one) calls on_click, the arrow part
        // (under the icon, or the right edge) opens the menu.
        class ribbon_button : public base
        {
            public:
                using ptr  = s_ptr<ribbon_button>;
                using wptr = w_ptr<ribbon_button>;

                enum class size_style { large, small, icon_only };

                std::function<void()> on_click;
                std::function<bool()> is_checked;
                std::function<bool()> is_enabled;
                // Set with set_dropdown (it changes the button's width).
                menu_list::ptr        dropdown;

                ribbon_button(std::string icon, std::string text, size_style style = size_style::large);

                void set_metrics(const ribbon_metrics& metrics);
                void set_style(size_style new_style);
                size_style get_style() const { return style; }
                void set_dropdown(menu_list::ptr menu);
                // The drop-down is as wide as the button by default; a popup that holds
                // something of its own width (a collapsed group) turns that off.
                void set_dropdown_matches_width(bool value) { dropdown_matches_width = value; }
                // Needs a drop-down and an on_click; the two halves then act separately.
                void set_split(bool value);
                bool is_split() const { return split; }

                // The Alt key tip over the button; empty hides it.
                void show_key_tip(const std::string& text);
                std::string get_key_tip() const { return tip_text; }
                // The ring drawn around the button the keyboard is on.
                void set_key_focus(bool value) { key_focused = value; }
                bool has_key_focus() const { return key_focused; }
                void set_text(std::string text);
                std::string get_text() const { return text; }
                std::string get_icon() const { return icon_label->text.get(); }
                void set_icon_text(std::string icon);

                // Width after layout (fixed; set from the label sizes), so a stack can match its buttons.
                float natural_width() const { return width; }
                void  set_width(float w);

                // The main action: on_click, or the menu for a button that has only a menu.
                void activate();
                // Opens the menu under the button, or closes it if it is open.
                void toggle_dropdown();

                virtual void on_pre_render(Context& c) override;
                virtual void draw(Context& c) override;
                virtual bool on_mouse_move(vec2 pos) override;
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
                virtual bool on_long_pressed(const touch& t) override;

            private:
                label::ptr     icon_label, text_label;
                ribbon_metrics metrics;
                size_style     style;
                std::string    text;
                float          width = 0.0f;
                bool           enabled_now = true;
                bool           checked_now = false;

                bool           split = false;
                bool           arrow_hovered = false;   // the pointer is over the arrow part
                bool           arrow_pressed = false;   // the press began on the arrow part
                bool           key_focused = false;
                bool           dropdown_matches_width = true;
                ribbon_key_tip::ptr tip;
                std::string    tip_text;

                void relayout();
                // The two parts of a split button, in render px.
                rect main_zone();
                rect arrow_zone();
        };

        // A captioned set of buttons laid out as columns: large buttons alone, small
        // ones stacked three high. The group can shrink through compact forms when the
        // ribbon runs out of width (set_level; the page picks the level):
        //   0  as authored
        //   1  every button small, stacked three high
        //   2  icons only
        //   3  one button with the group's title that opens the group as a popup
        class ribbon_group : public base
        {
            public:
                using ptr  = s_ptr<ribbon_group>;
                using wptr = w_ptr<ribbon_group>;

                ribbon_group(std::string title, const ribbon_metrics& metrics);

                // With a drop-down the button opens that menu instead of calling on_click,
                // unless `split`: then on_click is the main action and the menu the arrow part.
                ribbon_button::ptr add_large(std::string icon, std::string text, std::function<void()> on_click = {}, menu_list::ptr dropdown = {}, bool split = false);
                // Fills the open stack; a fourth one starts the next column.
                ribbon_button::ptr add_small(std::string icon, std::string text, std::function<void()> on_click = {}, menu_list::ptr dropdown = {}, bool split = false);
                ribbon_button::ptr add_icon(std::string icon, std::function<void()> on_click = {}, menu_list::ptr dropdown = {});
                // A button for an existing menu item: it calls the item's on_click, or, when
                // the item has a submenu, opens that as its drop-down. Lets the old menus
                // keep defining what the commands do while the ribbon shows them.
                ribbon_button::ptr add_menu_item(menu_list_element::ptr item, std::string icon, bool large = true);
                // Closes the open stack: the next small button starts a new column.
                void end_stack();
                // Any element as a column of its own (a combo box, a slider...); give it a fixed size.
                void add_widget(base::ptr widget);

                void set_metrics(const ribbon_metrics& new_metrics);

                static constexpr int max_level = 3;
                void set_level(int new_level);
                int  get_level() const { return level; }

                // When the ribbon is too narrow, the groups with the lowest priority shrink
                // first (equal priorities: right to left). Default 0.
                void set_priority(int value) { priority = value; }
                int  get_priority() const { return priority; }

                // What a key tip can point at: the group's buttons, or the one popup button
                // while the group is collapsed.
                std::vector<ribbon_button::ptr> key_targets() const;

                // Width the group takes at its current level, in UI px, margins included.
                float measure_width();

                // How many buttons and widgets were added (a cheap change signature).
                int entry_count() const { return int(entries.size()); }

                // The glyph on the collapsed button; the first button's icon by default.
                void set_icon(std::string icon);

                // The popup of the collapsed form, null until the group has been collapsed.
                menu_list::ptr get_popup() const { return popup; }

            private:
                struct entry
                {
                    ribbon_button::ptr           button;
                    base::ptr                    widget;
                    ribbon_button::size_style    authored = ribbon_button::size_style::large;
                    bool                         break_before = false;   // end_stack() came before it
                };

                ribbon_metrics metrics;
                std::string    title;
                std::string    icon;                 // set_icon; empty = the first button's
                label::ptr     caption;
                base::ptr      body;
                std::vector<entry> entries;
                bool           break_next = false;
                int            level = 0;
                int            priority = 0;
                float          columns_width = 0.0f;

                // The collapsed form: a button that opens `popup`, which then holds `body`.
                ribbon_button::ptr collapsed_button;
                menu_list::ptr     popup;

                // Replaced columns and stacks stay alive a couple of frames: the previous
                // frame may still be recording draws that point at them.
                std::vector<base::ptr> graveyard[2];

                void rebuild();
                void fit_stack(base::ptr column);
                void host_body_in_popup(bool in_popup);
                void add_entry(entry e);
        };

        class ribbon_page;

        // A transparent layer on top of a strip that lets a finger drag it sideways when it
        // overflows. A finger that stays within a few pixels is still a tap for the buttons
        // below: mouse input and touch downs pass through, only a drag claims the finger.
        class ribbon_touch_scroll : public base
        {
            public:
                ribbon_touch_scroll(ribbon_page* owner);

                virtual bool on_mouse_action(mouse_action, mouse_button, vec2) override { return false; }
                virtual bool on_mouse_move(vec2) override { return false; }
                virtual bool on_wheel(mouse_wheel, float, vec2) override { return false; }
                virtual bool on_touch_action(mouse_action action, touch& t) override;
                virtual void on_touch_move(std::vector<touch*>& fingers) override;

            private:
                ribbon_page*        owner;
                std::map<int, vec2> poses;       // where each finger was last seen, render px
                bool                dragging = false;
        };

        // The strip under one tab: groups left to right, thin separators between.
        class ribbon_page : public base
        {
            public:
                using ptr  = s_ptr<ribbon_page>;
                using wptr = w_ptr<ribbon_page>;

                ribbon_page(const ribbon_metrics& metrics);

                ribbon_group::ptr add_group(std::string title);
                void set_metrics(const ribbon_metrics& new_metrics);

                int group_count() const { return int(groups.size()); }
                ribbon_group::ptr group(int index) const { return index >= 0 && index < int(groups.size()) ? groups[index] : nullptr; }

                // Chooses the compact level of every group so the page fits `available`
                // UI px: groups shrink one level at a time, lowest priority first (right to
                // left among equals), as many rounds as it takes. When even the smallest
                // forms don't fit, the strip scrolls (wheel, or a click on the faded edge).
                // `reset` starts from the roomiest form (the page grew or changed); without
                // it the groups only shrink further from where they are.
                void fit(float available, bool reset);
                float measure_total();
                int  content_signature() const;

                // How far the strip can scroll, in UI px, and where it is now.
                float get_max_scroll() const { return max_scroll; }
                float get_scroll() const { return scroll; }
                void  scroll_to(float value);

                // Every button a key tip can point at on this page.
                std::vector<ribbon_button::ptr> key_targets() const;

                virtual bool on_wheel(mouse_wheel type, float value, vec2 pos) override;
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
                virtual void draw_after(Context& c) override;

            private:
                ribbon_metrics metrics;
                std::vector<ribbon_group::ptr> groups;
                std::vector<base::ptr>         separators;
                s_ptr<ribbon_touch_scroll>     touch_layer;   // kept on top of the groups
                float scroll = 0.0f;
                float max_scroll = 0.0f;
        };

        // A tab title in the ribbon's bar.
        class ribbon_tab_header : public base
        {
            public:
                using ptr = s_ptr<ribbon_tab_header>;

                std::function<void()> on_select;
                // Two clicks within 0.35 s (frame time) replace the second select.
                std::function<void()> on_double_click;
                bool   selected = false;
                float4 accent   = float4(0, 0, 0, 0);   // contextual tabs carry a colour line on top

                ribbon_tab_header(const std::string& title, const ribbon_metrics& metrics);
                void set_metrics(const ribbon_metrics& new_metrics);

                // The Alt key tip over the title; empty hides it.
                void show_key_tip(const std::string& text);
                std::string get_key_tip() const { return tip_text; }

                virtual void on_pre_render(Context& c) override;
                virtual void draw(Context& c) override;
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

            private:
                label::ptr text_label;
                float      clock = 0.0f;           // accumulated frame time
                float      last_click = -10.0f;    // clock at the last click
                ribbon_key_tip::ptr tip;
                std::string tip_text;
        };

        // One of the title bar's window buttons (minimise, maximise / restore, close): a
        // vector icon on a hover colour. It tags itself for the window frame's hit test
        // (window_region), and a click runs the UI's window_command_handler.
        class ribbon_caption_button : public base
        {
            public:
                using ptr = s_ptr<ribbon_caption_button>;

                ribbon_caption_button(window_command command, float width, float height);
                void set_button_size(float width, float height);

                virtual void on_pre_render(Context& c) override;
                virtual void draw(Context& c) override;
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

            private:
                window_command   command;
                vector_icon::ptr icon;
                std::string      shown_icon;
        };

        // Tab bar plus the strip of the selected tab. Switching tabs fades and slides
        // the new strip in; the strip can be minimised to just the bar.
        class ribbon : public base
        {
            public:
                using ptr  = s_ptr<ribbon>;
                using wptr = w_ptr<ribbon>;

                ribbon();

                // A contextual tab has an accent colour and is meant to be shown only
                // while it applies (set_tab_visible).
                ribbon_page::ptr add_tab(std::string title, bool contextual = false, float4 accent = float4(0.95f, 0.65f, 0.15f, 1.0f));

                void select(int index);
                int  selected() const { return current; }
                int  tab_count() const { return int(tabs.size()); }
                ribbon_page::ptr page(int index) const;
                // Whether that tab's title is drawn as the selected one (not while minimised
                // with the strip closed).
                bool tab_looks_selected(int index) const { return index >= 0 && index < int(tabs.size()) && tabs[index].header->selected; }

                void set_tab_visible(int index, bool visible);
                bool is_tab_visible(int index) const;

                // Minimised: only the tab bar shows. Clicking a tab then opens its strip as a
                // popup under the bar (a menu: it closes on a click anywhere else); the arrow
                // button brings the strip back for good.
                void set_minimized(bool value);
                bool is_minimized() const { return minimized; }
                // The popup holding the strip while minimised, null before the first time.
                menu_list::ptr get_strip_popup() const { return strip_popup; }

                void set_touch_mode(bool value);
                bool is_touch_mode() const { return touch_mode; }

                // For a window that draws its own title bar (the platform's custom frame):
                // the tab bar becomes the caption (drag it, double click maximises) and gets
                // minimise / maximise / close buttons at its right end, driven by the UI's
                // window_command_handler. The title sits left of the collapse button.
                void set_window_controls(bool enable);
                void set_window_title(std::string title);
                // A vector icon (by name) at the left end of the tab bar, before the tabs.
                void set_window_icon(std::string icon_name);

                // Keyboard access. Alt, pressed and released on its own, shows a letter over
                // every tab; a letter opens that tab and shows letters over its buttons; a
                // letter there runs the button. Left / Right change tab, Down or Enter go into
                // the strip, Escape goes back a step and out, and so do a mouse press or any
                // other key. Typed characters are held back meanwhile (keys_captured).
                bool is_key_mode() const { return key_level > 0; }
                // 0 = off, 1 = tab letters, 2 = button letters.
                int  get_key_level() const { return key_level; }
                // The reaction to a key (the UI's key hook); true = consumed.
                bool on_key(key_action action, long key, key_mods mods);

                virtual void think(float dt) override;
                virtual void on_pre_render(Context& c) override;
                virtual void draw_after(Context& c) override;

            protected:
                virtual void on_add(base* parent) override;

            private:
                int  key_level = 0;
                int  key_focus = -1;         // the button with the focus ring in the strip (level 2)
                bool alt_down = false;
                bool alt_used = false;       // another key came while Alt was down: not a tap

                void enter_key_mode();
                void leave_key_mode();
                void show_tab_tips();
                void show_button_tips();
                void hide_all_tips();

                struct tab_info
                {
                    ribbon_tab_header::ptr header;
                    ribbon_page::ptr       page;
                    std::string            title;
                    bool                   visible = true;
                };

                ribbon_metrics     metrics;
                bool               touch_mode = false;
                base::ptr          bar, body;
                button::ptr        collapse_button;
                std::vector<tab_info> tabs;
                int                current = -1;
                bool               minimized = false;

                // Strip height, eased between 0 (minimised) and the metrics' height.
                float              body_height = 0.0f;

                // Fade and slide of the strip that has just been selected.
                float              switch_t = 1.0f;

                // The underline under the selected tab (render px), eased.
                float              underline_x = 0.0f, underline_w = 0.0f;
                bool               underline_valid = false;

                // The width and content the pages were last fitted for.
                float              fitted_width = -1.0f;
                int                fitted_signature = -1;

                bool                                   window_controls = false;
                std::vector<ribbon_caption_button::ptr> caption_buttons;   // close, maximise, minimise
                label::ptr                             title_label;
                base::ptr                              title_holder;   // the full height of the bar, so the text can centre in it
                void fit_title();
                vector_icon::ptr                       window_icon;
                void rebuild_bar_right();
                void rebuild_bar_left();

                // While minimised the strip lives in this popup instead of the ribbon.
                menu_list::ptr     strip_popup;
                bool               body_in_popup = false;

                void apply_metrics();
                void sync_collapse_icon();
                void fit_pages();
                void host_strip_in_popup(bool in_popup);
                void open_strip();
        };
    }
}
