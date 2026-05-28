export module GUI:FlowGraph.FlowManager;

import :ScrollContainer;
import :TabControl;
import :FlowGraph.Canvas;

import Core;

export namespace GUI
{
    namespace Elements
    {
        namespace FlowGraph
        {
            class manager : public base, public Singleton<manager>
            {
                    std::map<::FlowGraph::graph::ptr, tab_button::ptr> canvases;
                    link_item::ptr edit;
                    friend class Singleton<manager>;
                    manager();

                public:
                    using ptr = s_ptr<manager>;
                    std::map<::FlowGraph::graph::ptr, tab_button::ptr> get_all();


                

                    void on_edit(link_item::ptr item);

                    template <class T = ::FlowGraph::graph>
                    tab_button::ptr create_graph()
                    {
                        canvas::ptr canva(new canvas(this));
                        s_ptr<T> g(new T());
                        canva->init(g.get());
                        g->name = "some graph";
                        tab_button::ptr but(new tab_button());
                        but->get_label()->text = g->name;
                        but->page = canva;
                        canvases[g] = but;
                        return but;
                    }

                    void add_graph(::FlowGraph::graph::ptr from, ::FlowGraph::graph::ptr g);

                    tab_button::ptr add_graph(::FlowGraph::graph::ptr g);


            };

        }
    }
}