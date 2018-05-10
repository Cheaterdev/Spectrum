namespace GUI
{

    namespace Elements
    {
        class tree;
        class tree_element;
        class toogle_icon : public GUI::Elements::image
        {
                GUI::Elements::tree_element* owner;
				Render::Texture::ptr tex_closed, tex_opened;

            public:

                toogle_icon(GUI::Elements::tree_element* owner);

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
        };
        class tree_creator
        {
            public:
                using ptr = s_ptr<tree_creator>;
                virtual void init_element(tree_element* l, base_tree* elem);

        };


        class line : public GUI::base
        {
                bool dropping;

            public:
                GUI::Elements::tree_element* owner;

                line(GUI::Elements::tree_element* owner);

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                virtual bool need_drag_drop() override;

                bool on_drop(GUI::drag_n_drop_package::ptr p, vec2);

                bool can_accept(GUI::drag_n_drop_package::ptr p);

                virtual void draw(Render::context& c) override;

                virtual void on_drop_enter(GUI::drag_n_drop_package::ptr) override;

                virtual void on_drop_leave(GUI::drag_n_drop_package::ptr) override;


        };

        class tree_element : public base, public tree_item_listener
        {
                friend class tree;
                friend class toogle_icon;
                friend class line;

                //  label::ptr label_text;
                //   image::ptr icon;
           //     image::ptr open_icon;
                base::ptr other;
		        w_ptr<GUI::Elements::tree> main_tree;
                tree_creator::ptr creator;

            public:

			
				Events::Event<bool> on_toogle;
                bool selected = false;

                base_tree* node;
                using ptr = s_ptr<tree_element>;
                using wptr = w_ptr<tree_element>;

                tree_element(tree_creator::ptr creator, base_tree* elem);

                tree_element::ptr add_tree(tree_creator::ptr creator, base_tree* elem);

                void toogle();
                //	virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                //    virtual bool can_accept(GUI::drag_n_drop_package::ptr p) override;

                //  virtual bool on_drop(GUI::drag_n_drop_package::ptr p, vec2) override;

                virtual void tree_on_remove() override;

                virtual void tree_added_child(base_tree* child) override;

                void init(tree_creator::ptr creator, base_tree* node)
                {
                    auto tree_elem = add_tree(creator, node);
                    tree_elem->node = node;
                    node->register_listener(tree_elem.get());

                    for (int i = 0; i < node->get_child_count(); i++)
                        tree_elem->init(creator, node->get_child(i));
                }

                virtual void on_empty()
                {
                    //      open_icon->texture = Render::Texture::null;
                    //      icon->texture = Render::Texture::null;
                }
        };



        class tree : public scroll_container
        {
                friend class tree_element;
                friend class toogle_icon;
                friend class line;
                w_ptr<tree_element> selected_item ;

                void on_select_internal(tree_element::ptr elem)
                {
                    auto prev = selected_item.lock();

                    if (prev)
                        prev->selected = false;

                    selected_item = elem;

                    if (elem)
                        elem->selected = true;

                    if (on_select)
                        on_select(elem);
                }
                tree_creator::ptr creator;

				void draw(Render::context& c) override;

            public:

                std::function<void(tree_element::ptr)> on_select;
                bool draggable = false;
                //tree_contoller_base* controller;
                using ptr = s_ptr<tree>;
                using wptr = w_ptr<tree>;

                tree(tree_creator::ptr = tree_creator::ptr(new tree_creator()));
                virtual tree_element::ptr add_tree(base_tree* elem);

                void init(base_tree* root)
                {
                    //this->controller = controller;
                    //  base_tree* root = controller->get_root();
                    auto root_elem = add_tree(root);
                    root->register_listener(root_elem.get());
                    root_elem->node = root;

                    for (int i = 0; i < root->get_child_count(); i++)
                        root_elem->init(creator, root->get_child(i));
                }


        };
    }
}