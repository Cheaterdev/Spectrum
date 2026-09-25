export module GUI:Label;
import :Base;

import :ScrollContainer;

// FW1_TEXT_FLAG is label's alignment API (magnet_text), so callers get it from here.
export import TextSystem;
import TextEngine;


export namespace GUI
{
    namespace Elements
    {
        class label : public base
        {
                Text::Layout layout;
                vec2 text_size;

                Text::Style style() const { return { font_size.get(), Text::Weight::Light }; }

                virtual void on_text_changed(const std::string& str);
                virtual void on_size_changed(const float& str);

                virtual void on_pre_render(Context& context) override;
                virtual void pre_draw(HAL::CommandList::ptr list) override;

                // Set on the UI thread, read by the tree walk.
                std::mutex                  links_m;
                std::vector<Text::Link>     links;
                std::function<void(size_t)> on_link;
                std::atomic<int>            hovered_link = -1;

                // Top-left of the text inside bounds (pixels), from magnet_text.
                vec2 text_origin(vec2 bounds_size, vec2 text_px) const;
            public:
                int magnet_text;
                using ptr = s_ptr<label>;
                using wptr = w_ptr<label>;

                property<std::string> text;
                property<float> font_size;
                float4 color;
                label();
                virtual	~label();
                virtual void draw(Context& c) override;

                // Caret queries in text-local logical units: origin at the top-left
                // of the laid-out text, independent of alignment and UI scale.
                // Offsets are codepoints.
                unsigned int get_index(vec2 at);
                vec2 get_caret_pos(unsigned int index);
                float get_line_height();

                // Clickable spans (byte ranges of text, sorted): underlined,
                // highlighted on hover, on_click(link index) on a left click.
                void set_links(std::vector<Text::Link> links, std::function<void(size_t)> on_click);

                virtual bool on_mouse_move(vec2 pos) override;
                virtual void on_mouse_leave(vec2 pos) override;
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
        };

        class MultiLineLabel: public scroll_container
        {
                                virtual void on_text_changed(const std::string& str);
            std::list<label::ptr> labels;
        public:
              using ptr = s_ptr<MultiLineLabel>;
                using wptr = w_ptr<MultiLineLabel>;


            property<std::string> text;
        	property<float> font_size;

            MultiLineLabel();
        };
    }
}
