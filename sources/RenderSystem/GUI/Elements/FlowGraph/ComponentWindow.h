#pragma once
#include "FlowGraph/FlowGraph.h"
#include "GUI/Elements/Resizer.h"
#include "GUI/Elements/Button.h"

namespace GUI
{
    namespace Elements
    {
        namespace FlowGraph
        {
			class component_window;
			class VisualGraph
			{
            public:
                float4 color = float4(31, 63, 85, 255) / 255.0f;;
				friend class component_window;
			protected:
				virtual GUI::base::ptr create_editor_window() { return nullptr; }
			};
			class selectable
            {
                public:
                    bool selected = false;

                    virtual bool test(vec2 at)
                    {
                        return false;
                    }
                    virtual bool test(vec2 from, vec2 to)
                    {
                        return false;
                    }
                    virtual bool test_full(vec2 from, vec2 to)
                    {
                        return false;
                    }
            };
            class link_item;
            class canvas;

            enum class window_type : int
            {
                NODE, GRAPH_IN, GRAPH_OUT
            };

            class component_window : public base, public selectable
            {
                    label::ptr label_text;
                    dragger::ptr title_bar;
                    resizer::ptr resiz;

                    base::ptr contents_left;
                    base::ptr contents_right;
                    base::ptr contents_preview;
                    //    base::ptr preview;

                    button::ptr close_button;
                    ::FlowGraph::Node* node = nullptr;

                    VisualGraph* visual_graph = nullptr;

                    friend class canvas;
                    window_type type;
                    canvas* canva;
                    Skin::Window skin;
                public:


                    using ptr = s_ptr<component_window>;
                    using wptr = w_ptr<component_window>;

                    virtual void draw(Render::context& c) override;
                    component_window(canvas* canva, ::FlowGraph::Node* node, window_type type);

                    s_ptr<link_item> add_input(::FlowGraph::parameter* p);
                    s_ptr<link_item> add_output(::FlowGraph::parameter* p);

                    //	link_item::ptr remove_input(::FlowGraph::parameter*p);
                    //	link_item::ptr remove_output(::FlowGraph::parameter*p);

                    virtual void on_touch() override;
                    virtual void on_child_touched(base::ptr child)
                    {
                        if (child == title_bar)
                            on_touch();
                    }

                    virtual void on_pos_changed(const vec2& r) override;
                    virtual bool test(vec2 from, vec2 to)
                    {
                        rect t = intersect(rect(from, to - from), rect(pos.get(), size.get()));
                        return t.size.x > 0 && t.size.y > 0;
                    }


                    virtual bool test(vec2 at)
                    {
                        return at.x > pos->x && (at.x < pos->x + size->x) && at.y > pos->y && (at.y < pos->y + size->y);
                    }

                    virtual bool test_full(vec2 from, vec2 to)
                    {
                        return (pos->x >= from.x && pos->x + size->x < to.x) && (pos->y >= from.y && pos->y + size->y < to.y);
                    }

                    virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                    virtual void think(float dt) override;

            };



        }
    }
}