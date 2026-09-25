export module GUI:EditText;
import :Base;
import :ScrollBar;
import TextEngine;

export namespace GUI
{
    namespace Elements
    {
        // Text field over Text::Editor (skb_editor), single-line or multiline.
        //
        // Input arrives on the UI thread and is queued; it is applied in
        // on_pre_render, on the tree-walk thread, because the editor shares
        // Text::Engine's layout and atlas state with every other text element.
        //
        // The only children are the scroll bars at the edges, created in the
        // constructor: on_pre_render can't create any (add_child is UI-thread
        // only), and a child covering the text would take the mouse moves a
        // drag-selection needs. Selection, text, images, placeholder and caret
        // are all drawn directly.
        class edit_text : public base
        {
            public:
                // A block image's texture; owner keeps whatever holds the
                // texture alive (e.g. the texture asset).
                struct Image
                {
                    HAL::Texture2DView    view;
                    std::shared_ptr<void> owner;
                    vec2                  size;   // pixels, for the aspect ratio
                };

                // Turns a drag-and-drop package into an image, or returns false.
                // Set by a layer that knows the package's source (texture assets
                // live above GUI). UI thread.
                static inline std::function<bool(const drag_n_drop_package::ptr&, Image&)> image_from_package;

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
                struct wheel_input
                {
                    float notches;   // positive scrolls up
                };
                struct color_input
                {
                    std::optional<float4> color;   // nullopt clears the span color
                };
                struct scrollbar_input
                {
                    bool  vertical;
                    float t;         // 0..1 along the scrollable range
                };
                struct image_input
                {
                    vec2     pos;    // window pixels
                    uint32_t id;     // key into inserted_images
                };
                using input_event = std::variant<key_input, char32_t, mouse_input, drop_input, wheel_input, color_input, scrollbar_input, image_input>;

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

                Text::Style  style;
                Text::Editor editor;
                std::string  text;   // mirror of the editor's text, for get_text()

                // Scroll position of the content box over the editor's content,
                // logical units, >= 0. Kept here rather than in skb_editor: its
                // view size comes from creation params, and changing those resets
                // the text.
                vec2 scroll;
                bool follow_caret = false;   // an edit or caret move since the last frame

                // Multiline only. Created in the constructor (UI thread), docked
                // over the edges of the content box; they take mouse input only
                // over their own strips, so drag-selection in the text is unaffected.
                scroll_bar::ptr vbar, hbar;
                vec2 synced_view = vec2(-1, -1), synced_content, synced_scroll;   // last sent to the bars
                void sync_scroll_bars(vec2 view, vec2 content);

                // Built by on_pre_render, drawn by draw/draw_after; guarded by m.
                // Editor-space logical units, relative to the content box.
                Text::Layout        layout;
                bool                showing_placeholder = false;
                std::vector<float4> selection;
                Text::Caret         caret;
                bool                caret_visible = false;

                // Block images: the document holds only ids (Text::Editor image
                // paragraphs), the textures live here. Guarded by m.
                std::unordered_map<uint32_t, Image> inserted_images;
                uint32_t next_image_id = 1;
                std::vector<Text::ImagePlacement> image_boxes;   // from the last build

                void process_events(Context& c);
                void process_mouse(const mouse_input& e, vec2 pos);
                void process_drop(const drop_input& e, vec2 pos);
                vec2 to_editor(vec2 window_pos, float scale);
                void process_key(long key, key_mods mods);
                void copy_selection();
                void open_context_menu(vec2 pos);
                rect content_rect(Context& c);
                void update_scroll(vec2 view_size);

            public:
                using ptr = s_ptr<edit_text>;
                using wptr = w_ptr<edit_text>;

                Events::Event<std::string> on_change;

                std::string placeholder;

                // Codepoints rejected here never enter the text, typed or pasted.
                std::function<bool(char32_t)> filter;

                // Enter starts a new line and Tab inserts a tab; newlines and tabs
                // survive paste. Single-line fields drop them.
                bool multiline = false;

                // Per-codepoint colors from the text (e.g. GUI::Syntax::highlight_hlsl).
                Text::Highlighter highlighter;

                float4 text_color        = float4(40, 40, 40, 255) / 255.0f;
                float4 placeholder_color = float4(120, 120, 120, 180) / 255.0f;
                float4 selection_color   = float4(0.25f, 0.5f, 1.0f, 0.35f);
                float4 caret_color       = float4(0, 0, 0, 1);

                edit_text();
                explicit edit_text(Text::Style style);

                // Programmatic set (e.g. seeding a bound value at construction).
                // Does not fire on_change -- callers seed the initial text before
                // wiring on_change, same ordering check_box_text/float_slider use.
                void set_text(const std::string& t);
                std::string get_text();

                // Compiler messages shown as wavy underlines and line-end icons;
                // hovering one shows its message as the tooltip. Cleared by any
                // edit.
                void set_diagnostics(std::vector<Text::Diagnostic> diagnostics);

                // Caret to a 0-based line/column, scrolled into view on the next frame.
                void goto_line(uint32_t line, uint32_t column = 0);

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
                virtual bool on_mouse_move(vec2 pos) override;
                virtual bool on_wheel(mouse_wheel type, float value, vec2 pos) override;

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
