export module GUI:ScrollBar;
import :Base;
import :Dragger;
import :Skin;
import Core;
export namespace GUI
{
    namespace Elements
    {

        class scroll_bar : public base
        {
                dragger::ptr drag;

            public:

                using ptr = s_ptr<scroll_bar>;
                using wptr = w_ptr<scroll_bar>;
                enum class scroll_type : int
                {
                    HORIZONTAL,
                    VERTICAL
                } type;
                std::function<void(float)> on_move;

                scroll_bar(scroll_type type);
                Skin::Button drag_skin;
                virtual void draw(Context& c) override;
                void set_sizes(float filled_size, float container_size, float container_pos);
        };

    }
}