#pragma once

#include "GUI/Elements/FlowGraph/ComponentWindow.h"

namespace FlowGraph
{

    class FlowSystem : public Singleton<FlowSystem>
    {
            friend class Singleton<FlowSystem>;
            class desc
            {
                    friend class FlowSystem;

                    std::function<::FlowGraph::window::ptr()> create_func;
                public:
                    std::string name;

                    ::FlowGraph::window::ptr create()
                    {
                        return create_func();
                    }
            };

            std::vector<desc> creators;
            FlowSystem()
            {
                register_node<::FlowGraph::window>("comment");
            }
        public:


            template<class T>
            bool register_node(std::string name)
            {
                desc d;
                d.name = name;
                d.create_func = [name]()->::FlowGraph::window::ptr
                {
                    auto res = ::FlowGraph::window::ptr(new T());
                    res->name = name;
                    return res; };
                creators.push_back(d);
                return true;
            }


            template<class T>
            bool register_node(std::string name,  T f)
            {
                desc d;
                d.name = name;
                d.create_func = f;
                creators.push_back(d);
                return true;
            }



            std::vector<desc>& get_all()
            {
                return creators;
            }
    };
}