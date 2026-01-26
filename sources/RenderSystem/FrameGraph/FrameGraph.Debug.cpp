module FrameGraphDebug;

import windows;
import Core;
import HAL;
import GUI;	 					 
import FrameGraph;

import Graphics;

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


class Texture2DDebugger :public GUI::base
{

public:


};


class ResourceDebugger :public GUI::base
{
public:
	using ptr = std::shared_ptr<ResourceDebugger>;
	FrameGraph::Graph& graph;
	std::wstring pass_name;
	std::string resource_name;

	Events::prop_helper * helper =nullptr;

	image::ptr rendered_image;
public:
	ResourceDebugger(FrameGraph::Graph& _graph) :graph(_graph)
	{

		width_size = GUI::size_type::MATCH_PARENT;
		height_size = GUI::size_type::MATCH_PARENT;

		auto resource_selector = std::make_shared<combo_box>();
		resource_selector->docking = GUI::dock::TOP;
		add_child(resource_selector);


		passes_list = std::make_shared<list_box>();
		passes_list->docking = GUI::dock::LEFT;
		passes_list->size = { 256,256 };
		add_child(passes_list);


		rendered_image = std::make_shared<image>();
		rendered_image->docking = GUI::dock::FILL;
		//	rendered_image->size={256,256};
		add_child(rendered_image);


		for (auto& [name, infov] : graph.builder.alloc_resources)
		{
			auto info = &infov;
			resource_selector->add_item(name)->on_select = [name, this, info]() {
				Log::get() << name << Log::endl;
				resource_name = name;
				passes_list->clear_items();


				auto it = graph.builder.alloc_resources.find(name);
				for (auto& s : it->second.states)
				{

					if (!s.write) continue;

					auto pass = s.passes.front();

					auto name = pass->name;	 /// can be dada already
					passes_list->add_item(convert(name))->on_select = [this, name](list_element::ptr) {

						pass_name = name;
						};
				}

				if(helper) helper->unregister();

				  
				helper = info->process_debug_resource.register_handler(this, [this, info](FrameGraph::Pass* pass, FrameGraph::FrameContext* context) {
					if (pass_name == pass->name)
					{
						auto list = context->get_list();
						auto &compute = list->get_compute();
						if (info->resource->get_desc().is_texture())
						{
							 {			
								Slots::FrameGraph_Debug_Common common;
								 common.GetTarget() = rendered_image->texture.texture->texture_2d().rwTexture2D;
								 common.GetTargetSize() = rendered_image->texture.texture->get_desc().as_texture().Dimensions.xy;
								compute.set(common);
							}


							compute.set_pipeline<PSOS::FrameGraph_Debug_Texture2D>();
							{			
								Slots::FrameGraph_Debug_Texture2D tex2d;
								 tex2d.GetSource() = *static_cast<HAL::Texture2DView*>(info->view.get());
								compute.set(tex2d);
							}

						   compute.dispatch(uint3(rendered_image->texture.texture->get_desc().as_texture().Dimensions.xy,1));
						//	MipMapGenerator::get().copy_texture_2d_slow(list->get_graphics(), rendered_image->texture.texture, );
						}
					}
						  
					});
				  
				};
		}

				  

	}

	void on_bounds_changed(const rect& r) override
	{
		base::on_bounds_changed(r);

		HAL::ResourceDesc desc = HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { r.size }, 1, 1, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess);
		auto texture = std::make_shared<Texture>(desc, TextureLayout::SHADER_RESOURCE);
		rendered_image->texture.texture = std::make_shared<Texture>(desc);
	}
private:

	list_box::ptr passes_list;

};
GUI::base::ptr FrameGraphDebug::create_debug_layout(FrameGraph::Graph& graph) {

	return std::make_shared<ResourceDebugger>(graph);
}
