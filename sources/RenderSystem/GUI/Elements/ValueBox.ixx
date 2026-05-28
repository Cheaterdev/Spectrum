export module GUI:ValueBox;
import :Base;
import :Button;

export namespace GUI
{
    namespace Elements
    {
        class value_box : public base
        {
                button::ptr create_less_button();
                button::ptr create_more_button();
                virtual bool on_wheel(mouse_wheel type, float value, float2 pos) override;
                float cur_time = 0;
                float inner_time = 0;
                virtual void think(float dt) override;
                button::ptr minus_butt;
                button::ptr plus_butt;
            public:
                using ptr = s_ptr<value_box>;
                label::ptr info;

                int min_value = 0;
                int max_value = 100;
                int current_value = 0;
                int max_good_value = 100000;

                std::function<void(int, value_box*)> on_change;
                value_box();
        };
    }
}

