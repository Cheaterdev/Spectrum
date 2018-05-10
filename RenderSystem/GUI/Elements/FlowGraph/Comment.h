namespace GUI
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

                    virtual void draw(Render::context& c) override;
                    comment(canvas* canva, ::FlowGraph::window* wnd);

                    virtual void on_pos_changed(const vec2& r) override;

                    virtual void on_size_changed(const vec2& r) override;

                    virtual bool test(vec2 from, vec2 to)
                    {
                        rect t = intersect(rect(from, to - from), rect(pos.get(), size.get()));
                        return t.size.x > 0 && t.size.y > 0;
                    }

                    virtual bool test(vec2 at)
                    {
                        return at.x > pos->x && (at.x < pos->x + size->x) && at.y > pos->y && (at.y < pos->y + size->y);
                    }

                    virtual void on_touch() override;
                    virtual void on_child_touched(base::ptr child)
                    {
                        if (child == title_bar)
                            on_touch();
                    }

                    virtual bool test_full(vec2 from, vec2 to)
                    {
                        return (pos->x >= from.x && pos->x + size->x < to.x) && (pos->y >= from.y && pos->y + size->y < to.y);
                    }

                    virtual void think(float dt) override;

            };


        }
    }
}