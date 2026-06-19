export module GUI:CollapsibleSection;
import :Base;
import :Label;
import :Tree;

export namespace GUI
{
    namespace Elements
    {
        class collapsible_section : public base
        {
            base::ptr        header;
            toogle_icon::ptr arrow;
            label::ptr       title_label;

            void toggle();

        public:
            using ptr  = s_ptr<collapsible_section>;
            using wptr = w_ptr<collapsible_section>;

            base::ptr content;

            Events::Event<bool> on_expanded_changed;

            collapsible_section(const std::string& title = "Section");

            void set_expanded(bool v);
            bool is_expanded() const;

            label::ptr get_title_label() { return title_label; }
        };
    }
}
