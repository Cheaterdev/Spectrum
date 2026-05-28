export module GUI:FlowGraph.Comment;

import :ScrollContainer;
import :CheckBox;
import :Resizable;
import :FlowGraph.ComponentWindow;

export namespace GUI
{
    namespace Elements
    {
        namespace FlowGraph
        {


            class comment : public resizable, public selectable
            {
                    label::ptr label_text;
                    dragger::ptr title_bar;
                    resizer::ptr resiz;

                    button::ptr close_button;
                    ::FlowGraph::window* wnd;

                    friend class canvas;

                    canvas* canva;
                    std::vector<base::ptr> inner_components;

                public:


                    using ptr = s_ptr<comment>;
                    using wptr = w_ptr<comment>;

                    virtual void draw(Context& c) override;
                    comment(canvas* canva, ::FlowGraph::window* wnd);

                    virtual void on_pos_changed(const vec2& r) override;

                    virtual void on_size_changed(const vec2& r) override;

                    virtual bool test(vec2 from, vec2 to) override;

                    virtual bool test(vec2 at) override;

                    virtual void on_touch() override;
                    virtual void on_child_touched(base::ptr child) override;

                    virtual bool test_full(vec2 from, vec2 to) override;

                    virtual void think(float dt) override;

            };


        }
    }
}