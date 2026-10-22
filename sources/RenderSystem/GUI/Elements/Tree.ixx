export module GUI:Tree;

import :Base;
import :ScrollContainer;
import :Image;
import :Label;
import :VectorIcon;

export namespace GUI
{

    namespace Elements
    {
        template<class TreeNode, class creator>
        class tree_element;

        template<class TreeNode, class creator>
        class tree;

        template<class TreeNode, class creator>
        class line;


        // Expand/collapse arrow: chevron_right when collapsed, chevron_down when
        // expanded, turning between the two (the chevron_0..chevron_90 icons) when
        // it changes inside a UI.
        class toogle_icon : public vector_icon
        {
            public:

                using ptr = s_ptr<toogle_icon>;
                std::function<void()> on_toogle;


                toogle_icon();
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                // Inverted, as the callers that predate set_expanded use it:
                // true = collapsed (right arrow), false = expanded (down arrow).
                void toogle(bool v);

                // Turns the arrow to the expanded (down) or collapsed (right) pose;
                // without a UI, or with animations off, it jumps there.
                void set_expanded(bool expanded);

                virtual void think(float dt) override;

                // 0 = pointing right (collapsed) ... 1 = pointing down (expanded).
                float turn_amount() const { return turn; }

            private:
                float turn = 0.0f;          // 0 = right, 1 = down
                float turn_target = 0.0f;
                int   shown_step = 0;       // icon currently set, in 10-degree steps
                void  show();
        };
    	
     

        // Draws a SlidingHighlight bar (y and h relative to content_y) over x..x+w,
        // clipped to box. Out of line: the Renderer is not visible from this file.
        void draw_selection_bar(base::Context& c, const rect& box, float content_y, const SlidingHighlight& bar, float x, float w, float strength = 1.0f);

    	class line_base: public base
    	{
    	protected:
            bool dropping = false;

            // Only the drop target: the selection highlight is the tree's sliding bar.
            void draw(Context& c);
    	};
    	inline float tree_row_padding = 0;
    	template<class TreeNode, class Creator>
        class line : public line_base
        {
              

            public:
                GUI::Elements::tree_element<TreeNode, Creator>* owner;

           
                void draw(Context& c)
                {
                    line_base::draw(c);
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

                    auto elem = dynamic_cast<line<TreeNode, Creator>*>(p->element.lock().get());
                    return !owner->node->is_parent(elem->owner->node);
                }

                bool on_drop(GUI::drag_n_drop_package::ptr p, vec2)
                {
                    auto elem = dynamic_cast<line<TreeNode, Creator>*>(p->element.lock().get());
              //      owner->node->add_child(elem->owner->node);
                    ASSERT(false);
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
                            owner->main_tree.lock()->on_select_internal(owner->template get_ptr<tree_element<TreeNode,Creator>>());

                    return true;
                }

                line(GUI::Elements::tree_element<TreeNode,Creator>* owner)
                {
                    this->owner = owner;
                    owner->row = this;
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
            using Creator = tree_creator<TreeNode>;
        public:
            using ptr = s_ptr<tree_creator<TreeNode>>;


           virtual void init_element(tree_element<TreeNode, Creator>* tree, TreeNode* elem)
            {
                base::ptr l(new line<TreeNode,Creator>(tree));

                base::ptr space(new base);

                space->size = { elem->calculate_depth() * 25,0 };
                space->docking = dock::LEFT;
                l->add_child(space);

                toogle_icon::ptr	open_icon(new toogle_icon());

                open_icon->on_toogle = [tree]() {tree->toogle();};
                //  open_icon->texture = HAL::Texture::null;
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
                label_text->magnet_text = FW1_TEXT_FLAG::FW1_LEFT | FW1_TEXT_FLAG::FW1_VCENTER | FW1_TEXT_FLAG::FW1_NOWORDWRAP;
                label_text->margin = { 5, tree_row_padding, 0, tree_row_padding };
                l->add_child(label_text);
                tree->add_child(l);

                // A new element starts expanded.
                open_icon->set_expanded(true);

                tree->on_toogle = [open_icon](bool expanded)
                {
                    open_icon->set_expanded(expanded);
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

    	template<class TreeNode, class Creator =   tree_creator<TreeNode> >
        class tree_element : public base
        {
                friend class GUI::Elements::tree<TreeNode, Creator>;
                friend class toogle_icon;
                friend class line<TreeNode,Creator>;

                using creator_type = typename Creator::ptr;
                //  label::ptr label_text;
                //   image::ptr icon;
           //     image::ptr open_icon;
                other_all::ptr other;
		        w_ptr<GUI::Elements::tree<TreeNode, Creator>> main_tree;
                creator_type creator;

            public:


				Events::Event<bool> on_toogle;
                bool selected = false;
                // The element's own row (the line the creator built), for the tree's selection bar.
                base* row = nullptr;

                TreeNode* node;
                using ptr = s_ptr<tree_element>;
                using wptr = w_ptr<tree_element>;

                void init(creator_type creator, TreeNode* node)
                {
                    auto tree_elem = add_tree(creator, node);
                    tree_elem->node = node;
                //    node->register_listener(tree_elem.get());

                    for (int i = 0; i < node->get_child_count(); i++)
                        tree_elem->init(creator, node->get_child(i));
                }

                virtual void on_empty()
                {
                    //      open_icon->texture = HAL::Texture::null;
                    //      icon->texture = HAL::Texture::null;
                }


                GUI::Elements::tree_element<TreeNode,Creator>::ptr add_tree(creator_type creator, TreeNode* elem)
                {
                    tree_element<TreeNode,Creator>::ptr res(new tree_element<TreeNode,Creator>(creator, elem));
                    other->add_child(res);
                    res->main_tree = main_tree;
                    return res;
                }

               tree_element(creator_type creator, TreeNode* elem)
                {
                    this->creator = creator;
                    thinkable = true;   // drives the expand / collapse animation
                    height_size = size_type::MATCH_CHILDREN;
                    width_size = size_type::MATCH_PARENT;
                    size = { 100, 0 };
                    x_type = pos_x_type::LEFT;
                    docking = dock::TOP;
          
                    other.reset(new other_all());
                    other->docking = dock::TOP;
                    other->height_size = size_type::MATCH_CHILDREN;
                    other->width_size = size_type::MATCH_PARENT;
                    other->x_type = pos_x_type::LEFT;
                    other->padding = { 0, 2, 0, 2 };
                    other->on_empty = [this]() {on_empty();};
                  

                    creator->init_element(this, elem);  
                    add_child(other);
                    elem->event_on_add_child.register_handler(this, [this](base_tree* child)
                    {
                            init(this->creator, static_cast<TreeNode*>(child));
                    });

                    elem->event_on_remove.register_handler(this, [this]()
                        {
//                            run_on_ui([this]() {  remove_from_parent(); });
                        });

                }


                // Expand / collapse. With animations on, the children slide open or
                // shut: their height eases between 0 and the measured height while
                // they fade as a group. A toggle mid-animation reverses it.
                void toogle()
                {
                    expanded = !expanded;
                    on_toogle(expanded);

                    if (!user_ui || !user_ui->window_animations)
                    {
                        other->visible = expanded;
                        return;
                    }

                    anim_target = expanded ? 1.0f : 0.0f;

                    if (animating)
                        return;   // think() turns around

                    if (expanded)
                    {
                        // Shut and invisible from the first frame; the children are laid
                        // out meanwhile and measured from their own bounds (see
                        // natural_height). Measuring through other's MATCH_CHILDREN
                        // height instead pushed every row below down by the full height
                        // for the frames that takes, then snapped them back.
                        other->height_size = size_type::FIXED;
                        other->size = vec2(other->size->x, 0.0f);
                        other->visible = true;
                        other->layer_opacity = 0.0f;
                        anim_s = 0.0f;
                        measure_frames = 2;
                    }
                    else
                    {
                        const float height = other->get_render_bounds().h / std::max(other->result_scale, 0.001f);
                        if (height <= 0.5f)
                        {
                            other->visible = false;
                            return;
                        }
                        anim_height = height;
                        other->height_size = size_type::FIXED;
                        other->size = vec2(other->size->x, height);
                        anim_s = 1.0f;
                        measure_frames = 0;
                    }

                    animating = true;
                }

                virtual void think(float dt) override
                {
                    if (!animating)
                        return;

                    if (measure_frames > 0)
                    {
                        if (--measure_frames > 0)
                            return;

                        // The children are laid out at their natural height now.
                        if (anim_target <= 0.0f)   // collapsed again while measuring
                        {
                            other->height_size = size_type::MATCH_CHILDREN;
                            other->visible = false;
                            other->layer_opacity = 1.0f;
                            animating = false;
                            return;
                        }

                        const float height = natural_height();
                        if (height <= 0.5f)
                        {
                            other->height_size = size_type::MATCH_CHILDREN;
                            other->layer_opacity = 1.0f;
                            animating = false;
                            return;
                        }
                        anim_height = height;
                    }

                    constexpr float duration = 0.18f;
                    const float step = dt / duration;
                    anim_s = anim_s < anim_target ? std::min(anim_s + step, anim_target) : std::max(anim_s - step, anim_target);

                    const float eased = anim_s * anim_s * (3.0f - 2.0f * anim_s);
                    other->size = vec2(other->size->x, anim_height * eased);
                    other->layer_opacity = eased;

                    if (anim_s == anim_target)
                    {
                        other->height_size = size_type::MATCH_CHILDREN;
                        other->layer_opacity = 1.0f;
                        other->visible = anim_target > 0.0f;
                        animating = false;
                    }
                }

            private:
                // Height of the children laid out at their natural size, UI px, read from
                // the children's own bounds (0 when none has been laid out yet).
                float natural_height()
                {
                    const rect box = other->get_render_bounds();
                    float bottom = box.y;

                    for (int i = 0; i < other->get_child_count(); i++)
                    {
                        auto* child = other->get_child(i);

                        if (!child->visible.get())
                            continue;

                        const rect cb = child->get_render_bounds();
                        bottom = std::max(bottom, cb.y + cb.h);
                    }

                    if (bottom <= box.y)
                        return 0.0f;

                    return (bottom - box.y) / std::max(other->result_scale, 0.001f) + other->padding->bottom;
                }

                bool  expanded = true;
                bool  animating = false;
                float anim_s = 1.0f;          // 0 = collapsed, 1 = expanded
                float anim_target = 1.0f;
                float anim_height = 0.0f;     // natural height of the children, UI px
                int   measure_frames = 0;
            public:

        };


        template<class TreeNode, class Creator =   tree_creator<TreeNode>>
        class tree : public scroll_container
        {
                friend class tree_element<TreeNode>;
                friend class toogle_icon;
                friend class line<TreeNode,Creator>;
                w_ptr<tree_element<TreeNode,Creator>> selected_item ;
                using creator_ptr_type = typename Creator::ptr;

                using elemet_type = typename tree_element<TreeNode,Creator>::ptr;
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
                creator_ptr_type creator;



                // The selection highlight: one bar that slides to the selected row
                // (x and width follow the row; y and height are eased, relative to
                // the scrolled content).
                SlidingHighlight bar;
                float bar_x = 0.0f, bar_w = 0.0f;
                base* bar_row = nullptr;   // the row the bar is on, to tell a move from a new selection
                // The row under the cursor: a fainter bar, same idea.
                SlidingHighlight hover_bar;
                float hover_x = 0.0f, hover_w = 0.0f;
                base* hover_row = nullptr;

                virtual void on_pre_render(Context& c) override
                {
                    scroll_container::on_pre_render(c);

                    auto item = selected_item.lock();
                    base* row = item ? item->row : nullptr;

                    float y = 0.0f, h = 0.0f;
                    if (row)
                    {
                        const rect rb = row->get_render_bounds();
                        y = rb.y - contents->get_render_bounds().y;
                        h = rb.h;
                        bar_x = rb.x;
                        bar_w = rb.w;

                        // Same row, pushed by an expanding branch: stay on it.
                        if (row == bar_row)
                            bar.follow(y, h);
                    }

                    bar_row = row;
                    const bool animate = user_ui && user_ui->window_animations;
                    bar.update(row != nullptr, y, h, c.delta_time, !animate);

                    // The row under the cursor: the hit test finds a label or an
                    // icon, so go up to the nearest row of this tree. A nested
                    // tree's row is found first, as it should be.
                    base* hovered = nullptr;
                    if (user_ui)
                        if (auto hit = user_ui->get_hovered().lock())
                        {
                            base* candidate = nullptr;
                            bool inside = false;
                            for (base* p = hit.get(); p; p = p->get_raw_parent())
                            {
                                if (!candidate)
                                    if (auto* l = dynamic_cast<line_base*>(p))
                                        candidate = l;

                                if (p == this)
                                {
                                    inside = true;
                                    break;
                                }
                            }

                            if (inside)
                                hovered = candidate;
                        }

                    float hover_y = 0.0f, hover_h = 0.0f;
                    if (hovered)
                    {
                        const rect rb = hovered->get_render_bounds();
                        hover_y = rb.y - contents->get_render_bounds().y;
                        hover_h = rb.h;
                        hover_x = rb.x;
                        hover_w = rb.w;

                        if (hovered == hover_row)
                            hover_bar.follow(hover_y, hover_h);
                    }

                    hover_row = hovered;
                    hover_bar.update(hovered != nullptr, hover_y, hover_h, c.delta_time, !animate);
                }

                void  draw(Context& c)
                {
              ///      context.renderer->draw_container(get_ptr(), c);
                    // Under the rows, inside the tree.
                    const float content_y = contents->get_render_bounds().y;
                    draw_selection_bar(c, get_render_bounds(), content_y, hover_bar, hover_x, hover_w, 0.22f);
                    draw_selection_bar(c, get_render_bounds(), content_y, bar, bar_x, bar_w);
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
             //       root->register_listener(root_elem.get());
                    root_elem->node = root;

                    for (int i = 0; i < root->get_child_count(); i++)
                        root_elem->init(creator, root->get_child(i));
                }





                elemet_type add_tree(TreeNode* elem)
                {
                    elemet_type res(new tree_element<TreeNode,Creator>(creator, elem));
                    contents->add_child(res);
                    res->main_tree = get_ptr<GUI::Elements::tree<TreeNode, Creator>>();
                    return res;
                }

               tree(creator_ptr_type creator = std::make_shared<Creator>())
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