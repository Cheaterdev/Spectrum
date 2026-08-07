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

                    // "Edit in new tab" for a subgraph node -- adds g as a
                    // new page in whichever tab_control from is already
                    // hosted in (found via canvases/register_tab below).
                    void add_graph(::FlowGraph::graph::ptr from, ::FlowGraph::graph::ptr g);

                    // Just builds the graph's editor content (a canvas) --
                    // no tab_button, no dock/window of its own. The caller
                    // decides how/where to host it (add_page on whatever
                    // tab_control makes sense -- see on_open_tab below) and
                    // should call register_tab() with the result so "edit in
                    // new tab" above and the settings-panel wrapping in
                    // main.cpp's canvas::on_open can still find it.
                    canvas::ptr create_canvas(::FlowGraph::graph::ptr g);

                    // Records which tab a graph ended up in, once the caller
                    // has added its canvas (from create_canvas) as a page.
                    void register_tab(::FlowGraph::graph::ptr g, tab_button::ptr tab);

                    // Set by the app (main.cpp) to add a graph's canvas as a
                    // page in the center dock, the same place its own "Debug
                    // Graph" menu item opens FrameGraphDebug -- instead of
                    // each call site building its own floating window/dock.
                    // Takes just (name, content); the app decides the
                    // tab_control. Left null (falls back to a floating
                    // window) for contexts without that app-level dock,
                    // e.g. tests.
                    static inline std::function<tab_button::ptr(std::string, base::ptr)> on_open_tab;


            };

        }
    }
}