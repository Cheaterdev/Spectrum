module FrameGraphDebug;

import windows;
import Core;
import HAL;
import GUI;	 					 
import FrameGraph;


using namespace GUI::Elements;
	 /*

class PassInfo:public GUI::base
{

public:
	using ptr = std::shared_ptr<ResourceDebugger>;


	PassInfo(FrameGraph::Pass& pass)
	{
	
		width_size = GUI::size_type::FIXED;
		height_size = GUI::size_type::FIXED;
		size={128,128};



	}
};
			*/
class ResourceDebugger:public GUI::base
{
public:
	using ptr = std::shared_ptr<ResourceDebugger>;
	FrameGraph::Graph& graph;
public:
	ResourceDebugger(FrameGraph::Graph& _graph):graph(_graph)
	{

		width_size = GUI::size_type::MATCH_PARENT;
		height_size = GUI::size_type::MATCH_PARENT;

		auto resource_selector = std::make_shared<combo_box>();
		resource_selector->docking = GUI::dock::TOP;
		add_child(resource_selector);
	

		passes_list = std::make_shared<list_box>();
		passes_list->docking = GUI::dock::LEFT;
		passes_list->size={256,256};
		add_child(passes_list);


		for (auto& [name, info] : graph.builder.alloc_resources)
		{
			resource_selector->add_item(name)->on_select = [name, this]() {
				Log::get() << name << Log::endl;

				passes_list->clear_items();


				 auto it = graph.builder.alloc_resources.find(name);
				for (auto& s : it->second.states)
				{

					if (!s.write) continue;

					auto pass = s.passes.front();
					passes_list->add_item(convert(pass->name))->on_select = [=](list_element::ptr) {
						//				Log::get()<< pass->name<<Log::endl;
						};
				}



				};
		}







	}


private:

	list_box::ptr passes_list;

};
	GUI::base::ptr FrameGraphDebug::create_debug_layout(FrameGraph::Graph& graph){
	
	return std::make_shared<ResourceDebugger>(graph);
	}		  
		 