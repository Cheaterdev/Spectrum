export module GUI:FlowGraph.Canvas;

import :ScrollContainer;
import :CheckBox;
import :FlowGraph.Comment;

export namespace GUI
{
    namespace Elements
    {
        namespace FlowGraph
        {

            class manager;
            enum class link_type : int
            {
                LINK_IN, LINK_OUT
            };
            class link_item: public check_box
            {
                public:
                    using ptr = s_ptr<link_item>;
                    using wptr = w_ptr<link_item>;

                    /*
                                        virtual void draw(Graphics::context& c) override;
                    					*/
                    bool inserted = false;
                    float beam_time = 0;
                    bool beaming = false;
                    link_type type;
                    ::FlowGraph::parameter* p;
                    base::wptr line;
                    bool can_delete = false;
                    link_item()
                    {
                        drag_listener = true;
                        set_package("link_item");
                        skin = Skin::get().DefaultOptionBox;
                        //	texture= HAL::Texture::get_resource(HAL::texure_header("textures/gui/option_normal.png"));
                    }

                    virtual bool need_drag_drop() override;

                    virtual bool can_accept(drag_n_drop_package::ptr) override;

                    virtual bool on_drop(drag_n_drop_package::ptr, vec2) override;

                    void update()
                    {
                        if (type == link_type::LINK_IN)
                            inserted = !p->input_connections.empty();

                        if (type == link_type::LINK_OUT)
                            inserted = !p->output_connections.empty();

                        set_checked(inserted);
                    }

                    virtual void on_dragdrop_start(drag_n_drop_package::ptr) override;

                    virtual void on_dragdrop_end() override;

                    virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

            };

            struct link_spline : public selectable
            {

                using ptr = s_ptr<link_spline>;
                using wptr = w_ptr<link_spline>;


                link_item::ptr from;
                link_item::ptr to;

                vec4 color;

                vec2 p1, p2, p3, p4;

                float phase = 0;

                void set_selected(bool value)
                {
                    if (selected != value)
                    {
                        if (value)
                            phase = 0;

                        selected = value;
                    }
                }
                bool test(vec2 from, vec2 to) override
                {
                    float len = abs(p4.x - p1.x);

                    for (int i = 0; i < len; i++)
                    {
                        float t = float(i) / len;
                        float2 pos = pow(1.0f - t, 3.0f) * p1 + 3.0f * pow(1.0f - t, 2.0f) * t * p2 + 3.0f * (1.0f - t) * pow(t, 2.0f) * p3 + pow(t, 3.0f) * p4;

                        if (pos.x > from.x && (pos.x < to.x) && pos.y > from.y && (pos.y < to.y))
                            return true;
                    }

                    return false;
                }

                bool test(vec2 p) override
                {
                    float len = abs(p4.x - p1.x);

                    for (int i = 0; i < len; i++)
                    {
                        float t = float(i) / len;
                        float2 pos = pow(1.0f - t, 3.0f) * p1 + 3.0f * pow(1.0f - t, 2.0f) * t * p2 + 3.0f * (1.0f - t) * pow(t, 2.0f) * p3 + pow(t, 3.0f) * p4;
                        float l = (pos - p).length();

                        if (l < 10)
                            return true;
                    }

                    return false;
                }
                void update(float dt)
                {
                    p1 = vec2(from->get_render_bounds().pos) + vec2(from->get_render_bounds().size) / 2;
                    p4 = vec2(to->get_render_bounds().pos) + vec2(to->get_render_bounds().size) / 2;
                    float d = abs(p1.x - p4.x);
                    float delta = std::max(15.f, d) / 3;
                    p2 = p1 + vec2(delta, 0);
                    p3 = p4 - vec2(delta, 0);

                    if (selected)
                    {
                        float l = 0.5f + 0.5f * sin(Math::m_2_pi * (phase += dt));
                        color = vec4(50, 150, 200, 255) / 255.0f * l + (1 - l) * vec4(16, 46, 100, 255) / 255.0f;
                    }

                    else
                        color = vec4(26, 96, 146, 255) / 255.0f;
                }
            };

            class canvas : public scroll_container, public ::FlowGraph::graph_listener
            {
                    std::map<::FlowGraph::Node*, component_window::ptr > nodes;
                    std::map<::FlowGraph::parameter*, link_item::ptr > links;
                    std::map<::FlowGraph::parameter*, component_window::ptr> link_node;
                    std::map<::FlowGraph::window*, comment::ptr > windows;


                    std::vector<link_spline::ptr> linking;
                    float scale_speed = 0;
                    vec2 wheel_pos;

                    component_window::ptr graph_in;
                    component_window::ptr graph_out;
                    base::ptr selection;

                    base::ptr comments;
                    vec2 select_pos;

                public:
                    manager* main_manager;
                    ::FlowGraph::graph* g;


                    virtual void draw(Graphics::context& c) override;

                    using ptr = s_ptr<canvas>;
                    using wptr = w_ptr<canvas>;
                    canvas(manager* main_manager);
                    //virtual void set_skin(Renderer_ptr skin) override;
                    virtual void on_add(base* parent) override;
                    virtual void on_remove()override
                    {
                        scroll_container::on_remove();
                    }

                    void init(::FlowGraph::graph* g);

                    virtual void on_register(::FlowGraph::window*) override;

                    virtual void on_remove(::FlowGraph::window*) override;

                    virtual void on_add_input(::FlowGraph::parameter*) override;

                    virtual void on_remove_input(::FlowGraph::parameter*) override;

                    virtual void on_add_output(::FlowGraph::parameter*) override;

                    virtual void on_remove_output(::FlowGraph::parameter*) override;

                    virtual void on_link(::FlowGraph::parameter*,  ::FlowGraph::parameter*) override;
                    virtual void on_unlink(::FlowGraph::parameter*, ::FlowGraph::parameter*) override;

                    virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                    virtual void on_key_action(long key) override;


                    virtual	bool on_wheel(mouse_wheel type, float value, vec2 pos) override;

                    virtual void think(float dt) override;

                    virtual bool on_mouse_move(vec2 pos) override;

                    void on_move(component_window* w, vec2 dt);

                    void on_start_drag(comment* w);
                    void on_move(comment* w, vec2 dt);
                    void on_select(component_window* w);

                    void on_select();

                    virtual bool can_accept(drag_n_drop_package::ptr) override;

                    virtual bool on_drop(drag_n_drop_package::ptr, vec2) override;
            };

        }

    }

}