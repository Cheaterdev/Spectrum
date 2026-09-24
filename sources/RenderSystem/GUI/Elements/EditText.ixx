export module GUI:EditText;
import :Base;
import :Label;
import :ColoredRect;

export namespace GUI
{
    namespace Elements
    {
        class edit_cursor : public base
        {

                float time;
                virtual void draw(Context& c) override;

            public:



                using ptr = s_ptr<edit_cursor>;
                using wptr = w_ptr<edit_cursor>;

                edit_cursor();
        };

        class edit_text : public base
        {
            private:
                struct key_input
                {
                    long     key;
                    key_mods mods;
                };
                using input_event = std::variant<key_input, char32_t>;

                std::vector<input_event> events;
                std::mutex m;
                bool mouse_selecting = false;

                void process_events();
                void process_key(long key, key_mods mods);
                void insert(std::string_view str);
                void erase_selection();
                void move_cursor(unsigned int pos, bool extend);
                unsigned int word_left(unsigned int pos) const;
                unsigned int word_right(unsigned int pos) const;
                void copy_selection();
                vec2 to_text_local(vec2 window_pos);

            protected:
                label::ptr label_text;
                label::ptr placeholder_label;
                edit_cursor::ptr label_cursor;
                base::ptr selection_layer;
                std::vector<colored_rect::ptr> selection_rects;

                unsigned int cursor_pos;
                // Selection is [min(anchor, cursor), max(anchor, cursor)); empty when equal.
                unsigned int anchor_pos;
                std::string text;

                bool has_selection() const { return anchor_pos != cursor_pos; }
                unsigned int selection_start() const { return std::min(anchor_pos, cursor_pos); }
                unsigned int selection_end() const { return std::max(anchor_pos, cursor_pos); }

                void update_caret();
                void update_selection();
            public:


                using ptr = s_ptr<edit_text>;
                using wptr = w_ptr<edit_text>;

                Events::Event<std::string> on_change;

                std::string placeholder;

                std::function<bool(char)> filter;

                edit_text();

                // Programmatic set (e.g. seeding a bound value at construction).
                // Does not fire on_change -- callers seed the initial text before
                // wiring on_change, same ordering check_box_text/float_slider use.
                void set_text(const std::string& t);
                const std::string& get_text() const { return text; }

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
                virtual bool on_mouse_move(vec2 pos) override;

                virtual void on_key_action(key_action action, long key, key_mods mods) override;
                virtual void on_char(char32_t ch) override;

                void draw(Context& c);

                virtual void on_mouse_enter(vec2 pos) override;

                virtual void on_mouse_leave(vec2 pos) override;
        };
    }
}
