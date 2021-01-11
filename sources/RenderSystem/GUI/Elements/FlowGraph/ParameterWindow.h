#include "../Label.h"
#include "../ComboBox.h"
namespace GUI
{
    namespace Elements
    {


        class property_base : public base
        {
            public:
                using wptr = w_ptr<property_base>;
                using ptr = s_ptr<property_base>;


                property_base(std::string text_name)
                {
                    draw_helper = true;
                    docking = dock::TOP;
                    size = { 50, 50 };
                    margin = { 20, 10, 20, 10 };
                    label::ptr name(new label());
                    name->text = text_name;
                    name->docking = dock::LEFT;
                    add_child(name);
                }

                virtual std::string get_value()
                {
                    return "0";
                }

        };

#define PE(x) {x,#x}

        class property_enum : public property_base
        {

                combo_box::ptr combo;

                template<class V, class E>
                void init(V& docking, E p, std::string name)
                {
                    auto item = combo->add_item(name);
                    item->on_select = [&docking, p]() {docking = p; };

                    if (docking == p)
                        item->select();
                }

                template<class V, class E, class ...Args>
                void init(V& docking, E p, std::string name, Args... args)
                {
                    init(docking, p, name);
                    init(docking, args...);
                }
            public:

                template<class V, class ...Args>
                property_enum(V& docking, std::string text_name, Args...args) : property_base(text_name), combo(new combo_box())
                {
                    init(docking, args...);
                    combo->docking = dock::FILL;
                    add_child(combo);
                }

        };
        /*
        class property_graph_type : public property_base
        {
        public:
        	property_graph_type(FlowGraph::parameter::ptr p,std::string text_name) :property_base(text_name)
        	{

        		combo_box::ptr combo(new combo_box());
        		//combo->size = {100,};
        		combo->add_item("float")->on_select = [p](){ p->type = FlowGraph::data_types("float"); };
        		combo->add_item("float2")->on_select = [p](){ p->type = FlowGraph::data_types("float2"); };
        		combo->add_item("float3")->on_select = [p](){ p->type = FlowGraph::data_types("float3"); };
        		combo->add_item("float4")->on_select = [p](){ p->type = FlowGraph::data_types("float4"); };


        		combo->docking = dock::FILL;
        		add_child(combo);
        	}

        };*/

        /*	class ParameterView :public base
        	{

        	public:
        		using wptr = w_ptr<ParameterView>;
        		using ptr = s_ptr<ParameterView>;

        		ParameterView(std::string text_name)
        		{
        			draw_helper = true;
        			docking = dock::TOP;
        			size = { 50, 50 };
        			margin = { 20, 10, 20, 10 };

        			label::ptr name(new label());

        			name->text = text_name;

        			add_child(name);
        		}
        	};
        	*/

        class ParameterWindow : public base
        {
            public:
                using ptr = s_ptr<ParameterWindow>;

                ParameterWindow()
                {
                    draw_helper = true;
                }

                void show(ParameterHolder::ptr holder)
                {
                    run_on_ui([this, holder]()
                    {
                        remove_all();

                        if (!holder) return;

                        holder->init_properties(this);
                    });
                }

                virtual void add_param(property_base::ptr param)
                {
                    add_child(param);
                }
        };
    }
}