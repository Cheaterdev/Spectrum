namespace GUI
{

    namespace Elements
    {
        template<class TreeNode>
        class tree_element;

        template<class TreeNode>
        class tree;

        template<class TreeNode>
        class line;


        class toogle_icon : public GUI::Elements::image
        {
          //      GUI::Elements::tree_element* owner;
				Render::Texture::ptr tex_closed, tex_opened;
            public:

                using ptr = s_ptr<toogle_icon>;
                std::function<void()> on_toogle;

        	
                toogle_icon();
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                void toogle(bool v);
        };
    	
     

    	class line_base: public base
    	{
    	protected:
            bool dropping = false;

            void draw(Render::context& c, bool selected);
    	};
    	template<class TreeNode>
        class line : public line_base
        {
              

            public:
                GUI::Elements::tree_element<TreeNode>* owner;

           
                void draw(Render::context& c)
                {
                    line_base::draw(c, owner->selected);
                }
                void on_drop_leave(GUI::drag_n_drop_package::ptr)
                {
                    dropping = false;
                }

                void on_drop_enter(GUI::drag_n_drop_package::ptr)
                {
                    dropping = true;
                }


                bool can_accept(GUI::drag_n_drop_package::ptr p)
                {
                    bool is_tree = p->name == "tree_item";

                    if (!is_tree)
                        return false;

                    auto elem = dynamic_cast<line<TreeNode>*>(p->element.lock().get());
                    return !owner->node->is_parent(elem->owner->node);
                }

                bool on_drop(GUI::drag_n_drop_package::ptr p, vec2)
                {
                    auto elem = dynamic_cast<line<TreeNode>*>(p->element.lock().get());
              //      owner->node->add_child(elem->owner->node);
                    assert(false);
                    return true;
                }

                bool need_drag_drop()
                {
                    return owner->main_tree.lock()->draggable;
                }

                bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
                {
                    if (button == mouse_button::LEFT)
                        if (action == mouse_action::DOWN)
                            owner->main_tree.lock()->on_select_internal(owner->get_ptr<tree_element<TreeNode>>());

                    return true;
                }

                line(GUI::Elements::tree_element<TreeNode>* owner)
                {
                    this->owner = owner;
                    docking = GUI::dock::TOP;
                    height_size = GUI::size_type::MATCH_CHILDREN;
                    width_size = GUI::size_type::MATCH_PARENT;
                    x_type = GUI::pos_x_type::LEFT;
                    set_package("tree_item");
                }


        };
    	
        template<class TreeNode>
        class tree_creator
        {
        public:
            using ptr = s_ptr<tree_creator<TreeNode>>;


           virtual void init_element(tree_element<TreeNode>* tree, TreeNode* elem)
            {
                base::ptr l(new line<TreeNode>(tree));

                base::ptr space(new base);

                space->size = { elem->calculate_depth() * 25,0 };
                space->docking = dock::LEFT;
                l->add_child(space);

                toogle_icon::ptr	open_icon(new toogle_icon());

                open_icon->on_toogle = [tree]() {tree->toogle();};
                //  open_icon->texture = Render::Texture::null;
              //    open_icon->size = { 16, 16 }; // size_type::MATCH_PARENT;
                open_icon->docking = dock::LEFT;

                open_icon->visible = elem->get_child_count() > 0;
                l->add_child(open_icon);
                /*    image::ptr icon(new image());
                    //
                    icon->size = { 16, 16 }; // size_type::MATCH_PARENT;
                    icon->docking = dock::LEFT;
                    icon->width_size = size_type::FIXED;
                    icon->height_size = size_type::FIXED;
                    l->add_child(icon);*/
                label::ptr label_text(new label());
                label_text->docking = dock::LEFT;
                label_text->text = convert(elem->get_name());
                label_text->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
                label_text->margin = { 5, 0, 0, 0 };
                l->add_child(label_text);
                tree->add_child(l);

                tree->on_toogle = [open_icon](bool v)
                {

                    open_icon->toogle(v);
                };
            }

        };
        class other_all : public GUI::base
        {
        public:
            using ptr = s_ptr<other_all>;
            std::function<void()> on_empty;

            virtual void remove_child(GUI::base::ptr obj) override
            {
                base::remove_child(obj);

                if (childs.size() == 0)
                    on_empty();
            }


        };

    	template<class TreeNode>
        class tree_element : public base, public tree_item_listener
        {
                friend class GUI::Elements::tree<TreeNode>;
                friend class toogle_icon;
                friend class line<TreeNode>;

                using creator_type = typename tree_creator<TreeNode>::ptr;
                //  label::ptr label_text;
                //   image::ptr icon;
           //     image::ptr open_icon;
                other_all::ptr other;
		        w_ptr<GUI::Elements::tree<TreeNode>> main_tree;
                creator_type creator;

            public:

			
				Events::Event<bool> on_toogle;
                bool selected = false;

                TreeNode* node;
                using ptr = s_ptr<tree_element>;
                using wptr = w_ptr<tree_element>;

                void init(creator_type creator, TreeNode* node)
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


                GUI::Elements::tree_element<TreeNode>::ptr add_tree(creator_type creator, TreeNode* elem)
                {
                    tree_element<TreeNode>::ptr res(new tree_element<TreeNode>(creator, elem));
                    other->add_child(res);
                    res->main_tree = main_tree;
                    return res;
                }

               tree_element(creator_type creator, TreeNode* elem)
                {
                    this->creator = creator;
                    height_size = size_type::MATCH_CHILDREN;
                    width_size = size_type::MATCH_PARENT;
                    size = { 100, 0 };
                    x_type = pos_x_type::LEFT;
                    docking = dock::TOP;
                    creator->init_element(this, elem);
                    other.reset(new other_all());
                    other->docking = dock::TOP;
                    other->height_size = size_type::MATCH_CHILDREN;
                    other->width_size = size_type::MATCH_PARENT;
                    other->x_type = pos_x_type::LEFT;
                    other->padding = { 0, 2, 0, 2 };
                    other->on_empty = [this]() {on_empty();};
                    add_child(other);
                }


                void toogle()
                {
                    other->visible = !other->visible.get();
                    on_toogle(other->visible);
                }

                void tree_on_remove()
                {
                    run_on_ui([this](){  remove_from_parent(); });
                  
                }

                void tree_added_child(TreeNode* child)
                {
                   init(creator, child); 
                }
        };


        template<class TreeNode>
        class tree : public scroll_container
        {
                friend class tree_element<TreeNode>;
                friend class toogle_icon;
                friend class line<TreeNode>;
                w_ptr<tree_element<TreeNode>> selected_item ;
                using creator_type = typename tree_creator<TreeNode>::ptr;

                using elemet_type = typename tree_element<TreeNode>::ptr;
                void on_select_internal(elemet_type elem)
                {
                    auto prev = selected_item.lock();

                    if (prev)
                        prev->selected = false;

                    selected_item = elem;

                    if (elem)
                        elem->selected = true;

                     on_select(elem->node);
                }
                creator_type creator;



                void  draw(Render::context& c)
                {
              ///      renderer->draw_container(get_ptr(), c);

                }


            public:

                Events::Event<TreeNode*> on_select;
                bool draggable = false;
                //tree_contoller_base* controller;
                using ptr = s_ptr<tree>;
                using wptr = w_ptr<tree>;

         
                void init(TreeNode* root)
                {
                    //this->controller = controller;
                    //  base_tree* root = controller->get_root();
                    auto root_elem = add_tree(root);
                    root->register_listener(root_elem.get());
                    root_elem->node = root;

                    for (int i = 0; i < root->get_child_count(); i++)
                        root_elem->init(creator, root->get_child(i));
                }





                elemet_type add_tree(TreeNode* elem)
                {
                    elemet_type res(new tree_element<TreeNode>(creator, elem));
                    contents->add_child(res);
                    res->main_tree = get_ptr<GUI::Elements::tree<TreeNode>>();
                    return res;
                }

               tree(creator_type creator = std::make_shared<tree_creator<TreeNode>>())
        		{
                    contents->size = { 20, 00 };
                    contents->width_size = size_type::MATCH_PARENT;
                    contents->height_size = size_type::MATCH_CHILDREN;
                    x_type = pos_x_type::LEFT;
                    this->creator = creator;
                }

        };
    }
}