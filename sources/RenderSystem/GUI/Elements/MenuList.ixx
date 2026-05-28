export module GUI:MenuList;
import :Base;
import :ScrollContainer;
import :Label;
import <RenderSystem.h>;


export namespace GUI
{

    namespace Elements
    {
        class menu_list;
        class menu_list_element : public base
        {
            protected:


                std::shared_ptr<menu_list> menu;
                bool on_strip = false;
            public:
                using ptr = s_ptr<menu_list_element>;
                using wptr = w_ptr<menu_list_element>;
                std::function<void(ptr)> on_click;
                std::function<void(menu_list_element::ptr)> on_hover;
                std::string text;

                bool selected = false;


                virtual void draw(Context& c) override;

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
                std::shared_ptr<menu_list> get_menu();

                virtual void close_menus();

                menu_list_element(bool on_strip);

                virtual void open_menu();
                virtual void close_menu();

                virtual bool is_menu_open();

                virtual void on_mouse_enter(vec2 pos) override;

        };



        class menu_list : public scroll_container
        {

                std::vector<menu_list_element::ptr > elements;
            protected:

                bool vertical;


                virtual bool need_open_on_hover();
            public:
                menu_list(bool vertical = true);
                //	class renderer;
                //friend class renderer;
                using ptr = s_ptr<menu_list>;
                using wptr = w_ptr<menu_list>;
                //	std::function<void(void)> on_click;
                //		bool draw_background;
                bool draw_icon = false;


        	void make_fixed_width();
                template<class T = menu_list_element>
                typename T::ptr add_item(std::string elem)
                {
                    label::ptr e(new label());
                    e->text = elem;
                    e->magnet_text = FW1_TEXT_FLAG::FW1_LEFT | FW1_TEXT_FLAG::FW1_VCENTER | FW1_TEXT_FLAG::FW1_NOWORDWRAP;
                    e->docking = dock::NONE;
                  //   e->y_type = pos_y_type::CENTER;
                  //   e->height_size = size_type::FIXED;

                    typename T::ptr l_e(new T(!vertical));
                    l_e->docking = vertical ? dock::TOP : dock::LEFT;
                    l_e->text = elem;

                    if(vertical)
                    {
                         l_e->width_size=size_type::MATCH_PARENT_CHILDREN; 
                    }   
                    else
                    {
                     l_e->width_size=size_type::MATCH_CHILDREN; 
                    
                    }

                     l_e->height_size=size_type::MATCH_CHILDREN;

                    if (draw_icon)
                    {
                        l_e->padding = { 30, 3, 30, 3 };
                   //     l_e->size = e->size.get() + vec2(60, 6);
                    }

                    else
                    {
                        l_e->padding = { 3, 3, 3, 3 };
                      //  l_e->size = e->size.get() + vec2(6, 6);
                    }

                    l_e->add_child(e);
                    l_e->on_hover = [this](menu_list_element::ptr elem)
                    {
                        if (!need_open_on_hover()) return;

                        if (elem->is_menu_open()) return;

                        base::close_menus();
                        elem->open_menu();
                        /* run_on_ui([this, elem]
                        {
                        close_menus();
                        //
                        });*/
                    };
                    contents->add_child(l_e);
                    elements.push_back(l_e);
                    //    if (vertical)
                    //    size = { std::max(size->x, padding->left + padding->right + l_e->size->x + l_e->margin->left + l_e->margin->right), size->y + l_e->size->y + l_e->margin->top + l_e->margin->bottom };
                    l_e->selected = elements.size() == 1;
                    return l_e;
                }

                virtual void draw(Context& c) override;
                virtual bool is_menu_component();

                virtual bool is_menu_open();

                bool is_self_open();

                void self_open(user_interface* ui);

                void self_close();

                virtual void remove_all();

                virtual void close_menus() override;


                /*	virtual void on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                virtual void on_bounds_changed(const rect &r) override;

                label::ptr get_label()
                {
                return label_text;
                }*/
        };



        class menu_strip : public menu_list
        {
            public:
                menu_strip();
                //	class renderer;
                //friend class renderer;
                using ptr = s_ptr<menu_list>;
                using wptr = w_ptr<menu_list>;

                virtual bool need_open_on_hover() override;

        };
    }
}