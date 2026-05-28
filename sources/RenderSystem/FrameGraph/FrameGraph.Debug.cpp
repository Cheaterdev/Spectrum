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



bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

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

	Events::prop_helper* helper = nullptr;

	image::ptr rendered_image;
	MultiLineLabel::ptr rendered_text;
	third_person_camera camera_3d;
	vec2 start_pos;
	vec2 prev_pos;

	bool dragging = false;
	float2 offset;
	float img_scale = 1;

	bool buffer_inited = false;
	GUI::Elements::tree<member_item,object_tree_creator>::ptr buffer_info;

	GUI::Elements::dock_base::ptr docker;
	
		std::map<std::string, int> pass_to_index;
		std::map<std::string, int> resource_to_index;


	bool passes_inited =false;
public:
	ResourceDebugger(FrameGraph::Graph& _graph) :graph(_graph)
	{

		width_size = GUI::size_type::MATCH_PARENT;
		height_size = GUI::size_type::MATCH_PARENT;

		{
			auto timeline = FrameGraphDebug::create_timeline_layout(_graph);
			timeline->docking = GUI::dock::FILL;
			add_child( timeline);
		}

		//add_child(table);




	/*///	TEMPRORALY NOT USED - USE THIS ONLY TO GENERATE RESOURCE PREVIEW AFTER EACH PASS

	//	rendered_text->magnet_text = FW1_LEFT | FW1_TOP;
		for (auto& [name, infov] : graph.builder.alloc_resources)
		{
			auto info = &infov;
			resource_selector->add_item(name)->on_select = [name, this, info]() {
				Log::get() << name << Log::endl;
				resource_name = name;
				passes_list->clear_items();

				offset = { 0,0 };
				img_scale = 1;


				auto it = graph.builder.alloc_resources.find(name);
				for (auto& s : it->second.states)
				{

					if (!s.write) continue;

					auto pass = s.passes.front();

					auto name = pass->name;	 /// can be dada already
					passes_list->add_item(convert(name))->on_select = [this, name](list_element::ptr) {

						pass_name = name;
						 buffer_inited = false;
						};
				}

				if (helper) helper->unregister();


				helper = info->process_debug_resource.register_handler(this, [this, info](FrameGraph::Pass* pass, FrameGraph::FrameContext* context) {
					if (pass_name == pass->name)
					{
						uint2 debug_size = int2::max({ 64,64 }, rendered_image->get_render_bounds().size);


						if (!rendered_image->texture.texture || uint2(rendered_image->texture.texture->get_desc().as_texture().Dimensions.xy) != debug_size)
						{
							HAL::ResourceDesc desc = HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { debug_size }, 1, 1, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess);
							auto texture = std::make_shared<Texture>(desc, TextureLayout::SHADER_RESOURCE);
							rendered_image->texture.texture = std::make_shared<Texture>(desc);
						}




						auto list = context->get_list();
						auto& compute = list->get_compute();
						auto& copy = list->get_copy();

						if (info->resource->get_desc().is_texture())
						{
							rendered_image->visible = true;
							buffer_info->visible = false;
							{
								Slots::FrameGraph_Debug_Common common;
								common.GetTarget() = rendered_image->texture.texture->texture_2d().rwTexture2D;
								common.GetTargetSize() = rendered_image->texture.texture->get_desc().as_texture().Dimensions.xy;
								compute.set(common);
							}



							if (auto source = dynamic_cast<HAL::Texture2DView*>(info->view.get()))
							{
								compute.set_pipeline<PSOS::FrameGraph_Debug_Texture2D>();
								{
									Slots::FrameGraph_Debug_Texture2D tex2d;
									tex2d.GetSource() = *source;
									tex2d.GetSourceSize() = info->resource->get_desc().as_texture().Dimensions.xy;
									tex2d.GetOffset() = offset;
									tex2d.GetScale() = img_scale;

									compute.set(tex2d);
								}
							}
							else if (auto source = dynamic_cast<HAL::Texture3DView*>(info->view.get()))
							{
								compute.set_pipeline<PSOS::FrameGraph_Debug_Texture3D>();


								mat4x4 view;
								mat4x4 proj;
								camera_3d.set_projection_params(Math::pi / 4, float(debug_size.x) / debug_size.y, 1, 1500);

								camera_3d.frame_move(0.1f);
								camera_3d.update();
								{
									Slots::FrameGraph_Debug_Texture3D tex3d;
									tex3d.GetSource() = *source;
									tex3d.GetSourceSize() = info->resource->get_desc().as_texture().Dimensions;
									tex3d.GetCamera() = camera_3d.camera_cb.current;

									compute.set(tex3d);
								}
							}

							else
							{
								compute.set_pipeline<PSOS::FrameGraph_Debug_NotImplemented>();
							}


							compute.dispatch(uint3(rendered_image->texture.texture->get_desc().as_texture().Dimensions.xy, 1));
							//	MipMapGenerator::get().copy_texture_2d_slow(list->get_graphics(), rendered_image->texture.texture, );
						}
						else
						{

							if (auto source = dynamic_cast<HAL::StructuredBufferViewBase*>(info->view.get()))
							{

								size_t elements_count = source->get_count();

								if (buffer_inited == false)
									copy.read_buffer(source->resource.get(), 0, elements_count * source->get_element_size(),
										[this, source, elements_count](std::span<std::byte> memory)
										{
											buffer_inited = true;
											auto tree = source->describe(memory.data() , memory.size());								

											run_on_ui([this,tree]()
												{
													buffer_info->contents->remove_all();
													buffer_info->init(tree.get());
												});
										});
								rendered_image->visible = false;
								buffer_info->visible = true;
							}
							else
								ASSERT(false);

						}
					}

					});

				};
		}*/



	}


	virtual bool on_mouse_move(vec2 pos) override
	{
		if (pressed && !dragging && (pos - start_pos).length() > 5)
		{
			dragging = true;
			prev_pos = pos;
			set_movable(true);
		}

		if (dragging)
		{
			camera_3d.input((pos - prev_pos) / 1000.0f);
			offset += pos - prev_pos;
			//owner->moving(speed);
			prev_pos = pos;
		}

		return dragging;
	}


	virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
	{
		if (button == mouse_button::RIGHT)
		{
			pressed = action == mouse_action::DOWN;
			start_pos = pos;

			if (!pressed)
			{
				dragging = false;
				set_movable(false);
			}
		}

		return false;
	}


	virtual bool on_wheel(mouse_wheel type, float value, vec2 wheel_pos) override
	{

		if(!rendered_image)	   return false;
		float prev_scale = img_scale;

		camera_3d.input(value / 100);
		//	 img_scale =value>0?2.0f:1.0f; 
		img_scale *= 1 + value / 10.0f;
		img_scale = Math::clamp(img_scale, 0.1f, 10.0f);

		vec2 mp = wheel_pos - rendered_image->get_render_bounds().pos;
		offset = img_scale / prev_scale * (offset - mp) + mp;


		return true;
	};



private:

	list_box::ptr passes_list;

};
GUI::base::ptr FrameGraphDebug::create_debug_layout(FrameGraph::Graph& graph) {

	return std::make_shared<ResourceDebugger>(graph);
}
