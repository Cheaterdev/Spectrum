module GUI:CollapsibleSection;
import :Renderer;

namespace GUI
{
    namespace Elements
    {
        static constexpr float HEADER_HEIGHT = 24.0f;

        class section_header : public base
        {
        public:
            std::function<void()> on_click;

            section_header()
            {
                docking     = dock::TOP;
                height_size = size_type::FIXED;
                size        = {0, HEADER_HEIGHT};
                clickable   = true;
            }

            virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
            {
                if (button == mouse_button::LEFT && action == mouse_action::UP)
                    if (on_click) on_click();
                return true;
            }
        };

        collapsible_section::collapsible_section(const std::string& title)
        {
            docking     = dock::TOP;
            height_size = size_type::MATCH_CHILDREN;
            width_size  = size_type::MATCH_PARENT;

            // header row
            auto hdr = std::make_shared<section_header>();
            hdr->on_click = [this]() { toggle(); };
            header = hdr;
            base::add_child(header);

            // arrow icon
            arrow.reset(new toogle_icon());
            arrow->docking = dock::LEFT;
            arrow->margin  = {4, 0, 4, 0};
            arrow->toogle(true);
            arrow->on_toogle = [this]() { toggle(); };
            header->add_child(arrow);

            // title label
            title_label.reset(new label());
            title_label->text        = title;
            title_label->docking     = dock::FILL;
            title_label->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
            header->add_child(title_label);

            // content container
            content.reset(new base());
            content->docking     = dock::TOP;
            content->height_size = size_type::MATCH_CHILDREN;
            content->width_size  = size_type::MATCH_PARENT;
            content->padding     = {8, 4, 4, 4};
            base::add_child(content);
        }

        void collapsible_section::toggle()
        {
            set_expanded(!content->visible.get());
        }

        void collapsible_section::set_expanded(bool v)
        {
            content->visible = v;
            arrow->toogle(v);
            on_expanded_changed(v);
        }

        bool collapsible_section::is_expanded() const
        {
            return content->visible.get();
        }
    }
}
