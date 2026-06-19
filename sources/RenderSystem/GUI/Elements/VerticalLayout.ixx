export module GUI:VerticalLayout;
import :Base;

export namespace GUI
{
    namespace Elements
    {
        namespace layouts
        {
            class vertical : public base
            {
            public:
                using ptr  = s_ptr<vertical>;
                using wptr = w_ptr<vertical>;

                vertical();

                void on_bounds_changed(const rect& r) override;
                virtual sizer update_layout(sizer r, float scale) override;

                virtual void add_child(base::ptr obj) override;
            };
        }
    }
}
