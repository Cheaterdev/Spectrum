export module GUI:FlowGraph.Canvas;

import :ScrollContainer;
import :CheckBox;
import :Button;
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

                    bool inserted = false;
                    float beam_time = 0;
                    bool beaming = false;
                    link_type type;
                    ::FlowGraph::parameter* p;
                    base::wptr line;
                    bool can_delete = false;
                    link_item();

                    virtual bool need_drag_drop() override;

                    virtual bool can_accept(drag_n_drop_package::ptr) override;

                    virtual bool on_drop(drag_n_drop_package::ptr, vec2) override;

                    void update();

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

                void set_selected(bool value);
                bool test(vec2 from, vec2 to) override;

                bool test(vec2 p) override;

                void update(float dt);
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


                    base::ptr selection;
                    base::ptr toolbar;

                    base::ptr comments;
                    vec2 select_pos;

                public:
                    manager* main_manager;
                    ::FlowGraph::graph* g;

                             component_window::ptr graph_in;
                    component_window::ptr graph_out;
                    virtual void draw(Context& c) override;

                    using ptr = s_ptr<canvas>;
                    using wptr = w_ptr<canvas>;
                    canvas(manager* main_manager);
                    //virtual void set_skin(Renderer_ptr skin) override;
                    virtual void on_add(base* parent) override;
                    virtual void on_remove() override;

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

                    virtual void on_key_action(key_action action, long key) override;


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