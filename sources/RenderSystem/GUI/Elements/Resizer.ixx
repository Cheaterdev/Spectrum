export module GUI:Resizer;
import :Base;
import :Dragger;



export namespace GUI
{
    namespace Elements
    {
        enum class direction : int
        {
            LEFT = 1, RIGHT = 2, TOP = 4, BOTTOM = 8,

            LEFT_TOP = LEFT | TOP, RIGHT_TOP = RIGHT | TOP,
            LEFT_BOTTOM = LEFT | BOTTOM, RIGHT_BOTTOM = RIGHT | BOTTOM
        };

        direction to_direction(dock d);

        dock invert(dock d);

        class resizer : public dragger
        {

        public:
            direction dir = direction::BOTTOM;
            using ptr = s_ptr<resizer>;
            using wptr = w_ptr<resizer>;
            resizer();
            virtual void on_mouse_enter(vec2) override;

            virtual void on_mouse_leave(vec2) override;

            virtual bool on_mouse_move(vec2 pos) override;



        };
    }
}