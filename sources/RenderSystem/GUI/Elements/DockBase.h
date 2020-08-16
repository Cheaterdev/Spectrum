namespace GUI
{
    namespace Elements
    {
        class dock_base : public base
        {
                base::ptr virt;
                base::ptr virt_base;
                //	image::ptr fill,center,right,top,bottom;
                tab_control::ptr tabs;
                resizer::ptr sizer;
                bool original = true;
            public:
                using ptr = s_ptr<dock_base>;
                using wptr = w_ptr<dock_base>;

                dock_base::ptr left, top, bottom, right;
                dock_base();

                Events::Event<bool> on_empty;

                virtual	bool can_accept(drag_n_drop_package::ptr package) override;


                virtual void on_drop_enter(drag_n_drop_package::ptr) override;

                virtual void on_drop_move(drag_n_drop_package::ptr e, vec2 p) override;

                virtual void on_drop_leave(drag_n_drop_package::ptr) override;

                virtual bool on_drop(drag_n_drop_package::ptr, vec2) override;

                virtual void draw_after(Render::context& c) override;


                dock get_docking(vec2 p);

                tab_control::ptr get_tabs();

                dock_base::ptr get_dock(dock d);

                virtual void remove_child(base::ptr obj) override;


                void update_resizer()
                {
                    if (docking == dock::FILL)
                    {
                        if (sizer)
                            sizer->remove_from_parent();

                        return;
                    }

                    if (!sizer)
                    {
                        sizer.reset(new resizer());
                        add_child(sizer);
                    }

                    sizer->dir = to_direction(invert(docking.get()));
                    //	sizer->size = { 2, 2 };
                    sizer->target = this;
                    sizer->docking = invert(docking.get());
                }

                virtual void on_dock_changed(const dock& r) override;

        };
    }
}