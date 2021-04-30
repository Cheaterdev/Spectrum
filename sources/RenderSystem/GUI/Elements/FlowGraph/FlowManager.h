namespace GUI
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
                    manager()
                    {
                        visible = false;

                    }

                public:
                    using ptr = s_ptr<manager>;
                    std::map<::FlowGraph::graph::ptr, tab_button::ptr> get_all()
                    {
                        return canvases;
                    }


                

                    void on_edit(link_item::ptr item)
                    {
                        if (edit)
                        {
                            auto l = edit->line.lock();
                          if(l)l->draw_helper = false;

                        }

                        {
                            auto l = item->line.lock();
                            if (l)l->draw_helper = true;
                        }
   
                        edit = item;
                  
                    }

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

                    void add_graph(::FlowGraph::graph::ptr from, ::FlowGraph::graph::ptr g)
                    {
                        tab_control::ptr tabs = canvases[from]->owner.lock();
                        canvas::ptr canva(new canvas(this));
                        canva->init(g.get());
                        g->name = "some graph";
                        tab_button::ptr but(new tab_button());
                        but->get_label()->text = g->name;
                        but->page = canva;
                        canvases[g] = but;
                        tabs->add_button(but);
                    }

                    tab_button::ptr add_graph(::FlowGraph::graph::ptr g)
                    {
                        if (!g) return nullptr;

                        canvas::ptr canva(new canvas(this));
                        canva->init(g.get());
                        g->name = "some graph";
                        tab_button::ptr but(new tab_button());
                        but->get_label()->text = g->name;
                        but->page = canva;
                        canvases[g] = but;
                        return but;
                    }


            };

        }
    }
}