export module GUI:StatusBar;
import :Base;
import :Skin;

export namespace GUI
{
    namespace Elements
    {

        class status_bar: public base
        {
            public:
                using ptr = s_ptr<status_bar>;
                using wptr = w_ptr<status_bar>;
                status_bar()
                {
                    size = { 25, 25 };
                    docking = dock::BOTTOM;
                    skin = Skin::get().DefaultStatusBar;
                }
                Skin::StatusBar skin;
                virtual void draw(Context&) override;

                virtual void add_child(base::ptr obj) override;

        };
    }
}