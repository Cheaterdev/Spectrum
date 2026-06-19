export module GUI:OptionBox;
import :Base;
import :CheckBoxText;


export namespace GUI
{
    namespace Elements
    {
        class option_group : public base
        {
            public:

                using ptr = s_ptr<option_group>;
                using wptr = w_ptr<option_group>;

            protected:
                std::vector<toggle_switch::ptr> all;
                void on_check(toggle_switch::ptr obj);
            public:

                option_group();
                ~option_group();
                toggle_switch::ptr add_option(toggle_switch::ptr obj);

                check_box_text::ptr create_option(bool value = false);

        };
    }
}