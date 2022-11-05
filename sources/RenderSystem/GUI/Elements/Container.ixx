export module GUI:Container;
import :ScrollContainer;


export namespace GUI
{

    namespace Elements
    {

        class container : public scroll_container
        {
                std::vector<base::ptr > elements;
            protected:

                void recalculate();
            public:
                using ptr = s_ptr<container>;
                using wptr = w_ptr<container>;

                container();

                void remove_elements();
                virtual void add_child(base::ptr obj) override;

                virtual void on_bounds_changed(const rect& r) override;

        };
    }
}