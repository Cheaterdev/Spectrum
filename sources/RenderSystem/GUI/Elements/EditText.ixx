export module GUI:EditText;
import :Base;
import TextEngine;

export namespace GUI
{
    namespace Elements
    {
        // Single-line text field over Text::Editor (skb_editor).
        //
        // Input arrives on the UI thread and is queued; it is applied in
        // on_pre_render, on the tree-walk thread, because the editor shares
        // Text::Engine's layout and atlas state with every other text element.
        //
        // Deliberately has no child elements: on_pre_render can't create them
        // (add_child is UI-thread only), and a child covering the field would
        // take the mouse moves a drag-selection needs. Selection, text,
        // placeholder and caret are all drawn directly.
        class edit_text : public base
        {
            private:
                struct key_input
                {
                    long     key;
                    key_mods mods;
                };
                struct mouse_input
                {
                    enum class Kind { Down, Drag, Up, Cancel };
                    Kind   kind;
                    vec2   pos;                 // window pixels
                    double time = 0;
                    bool   in_selection = false; // Down: landed inside the selection
                    bool   after_drag   = false; // Up: a drag-and-drop happened
                };
                struct drop_input
                {
                    vec2             pos;       // window pixels
                    w_ptr<edit_text> source;
                    bool             copy;
                };
                using input_event = std::variant<key_input, char32_t, mouse_input, drop_input>;

                std::vector<input_event> events;
                std::mutex m;
                bool mouse_selecting = false;

                // UI-thread side of dragging selected text: the engine's
                // drag-and-drop asks need_drag_drop() on the UI thread, so whether
                // a press landed in the selection is decided there, at press time.
                bool drag_candidate = false;
                bool drag_started   = false;

                // A press inside the selection is held back: released in place it
                // is replayed as a click, dragged it becomes a drag-and-drop.
                bool   press_held = false;
                vec2   press_pos;          // editor space
                double press_time = 0;

                // Hovered by a text drag: show where it would land.
                bool        drop_hover = false;
                vec2        drop_pos;      // window pixels
                Text::Caret drop_caret;

                Text::Editor editor;
                std::string  text;   // mirror of the editor's text, for get_text()

                // Built by on_pre_render, drawn by draw/draw_after; guarded by m.
                // Editor-space logical units, relative to the content box.
                Text::Layout        layout;
                bool                showing_placeholder = false;
                std::vector<float4> selection;
                Text::Caret         caret;
                bool                caret_visible = false;

                void process_events(Context& c);
                void process_mouse(const mouse_input& e, vec2 pos);
                void process_drop(const drop_input& e, vec2 pos);
                vec2 to_editor(vec2 window_pos, float scale);
                void process_key(long key, key_mods mods);
                void copy_selection();
                void open_context_menu(vec2 pos);
                rect content_rect(Context& c);

            public:
                using ptr = s_ptr<edit_text>;
                using wptr = w_ptr<edit_text>;

                Events::Event<std::string> on_change;

                std::string placeholder;

                // Codepoints rejected here never enter the text, typed or pasted.
                std::function<bool(char32_t)> filter;

                float4 text_color        = float4(40, 40, 40, 255) / 255.0f;
                float4 placeholder_color = float4(120, 120, 120, 180) / 255.0f;
                float4 selection_color   = float4(0.25f, 0.5f, 1.0f, 0.35f);
                float4 caret_color       = float4(0, 0, 0, 1);

                edit_text();

                // Programmatic set (e.g. seeding a bound value at construction).
                // Does not fire on_change -- callers seed the initial text before
                // wiring on_change, same ordering check_box_text/float_slider use.
                void set_text(const std::string& t);
                std::string get_text();

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
                virtual bool on_mouse_move(vec2 pos) override;

                virtual void on_key_action(key_action action, long key, key_mods mods) override;
                virtual void on_char(char32_t ch) override;

                virtual void on_pre_render(Context& c) override;
                virtual void pre_draw(HAL::CommandList::ptr list) override;
                virtual void draw(Context& c) override;
                virtual void draw_after(Context& c) override;

                virtual void on_mouse_enter(vec2 pos) override;

                virtual void on_mouse_leave(vec2 pos) override;

                // Drag-and-drop of selected text, within and between fields.
                virtual bool need_drag_drop() override;
                virtual void on_drag_start() override;
                virtual void generate_container(base::ptr holder) override;
                virtual bool can_accept(drag_n_drop_package::ptr package) override;
                virtual void on_drop_move(drag_n_drop_package::ptr package, vec2 pos) override;
                virtual void on_drop_leave(drag_n_drop_package::ptr package) override;
                virtual bool on_drop(drag_n_drop_package::ptr package, vec2 pos) override;
        };
    }
}
