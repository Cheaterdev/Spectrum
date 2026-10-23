module FrameGraphDebug;
import RenderSystem;

import windows;
import Core;
import HAL;
import GUI;	 					 
import FrameGraph;

import Graphics;


using namespace GUI::Elements;

// Mirrors framegraph/stats.hlsli: the picked texel (8 words), min / max / sum per channel (16 words), then
// 4 channels x 256 histogram bins. resource_preview reads all of it back every frame (4 KB); the bins start at
// `readback_words` (the header: pick, min / max / sum).
namespace preview_stats
{
	constexpr uint max_grid       = 512;
	constexpr uint minmax_at      = 8;
	constexpr uint readback_words = 24;
	constexpr uint words          = readback_words + 4 * 256;
	constexpr float sum_fraction  = 4096.0f;
}
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
	const wchar_t* pass_name = nullptr;
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

					auto* pass = graph.builder.get_pass(s.passes.front());
					if (!pass) continue;

					auto name = pass->name;	 /// can be dada already
					passes_list->add_item(convert(name.ptr))->on_select = [this, name](list_element::ptr) {

						pass_name = name.ptr;
						 buffer_inited = false;
						};
				}

				if (helper) helper->unregister();


				helper = info->process_debug_resource.register_handler(this, [this, info](FrameGraph::Pass* pass, FrameGraph::FrameContext* context) {
					if (pass_name == pass->name.ptr)
					{
						uint2 debug_size = int2::max({ 64,64 }, rendered_image->get_render_bounds().size);


						if (!rendered_image->texture.texture || uint2(rendered_image->texture.texture->get_desc().as_texture().Dimensions.xy) != debug_size)
						{
							HAL::ResourceDesc desc = HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { debug_size }, 1, 1, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess);
							rendered_image->texture.texture = std::make_shared<Texture>(RenderSystem::get().device(), desc);

							// resource@pass, so a validation message names what it is
							// and where it came from. These are debugger-owned
							// textures, and unnamed they show up as "Unnamed
							// ID3D12Resource Object" -- indistinguishable from each
							// other and from anything else the debug layer reports.
							rendered_image->texture.texture->resource->set_name(
								std::string("FGDebug::preview::") + info->name() + "@" + convert(pass->name.ptr));
						}




						auto list = context->get_list();
						auto& compute = list->get_compute();
						auto& copy = list->get_copy();

						if (info->resource->get_desc().is_texture())
						{
							rendered_image->visible = true;
							buffer_info->visible = false;
							{
								Slots::Dev::FrameGraph_Debug_Common common;
								common.GetTarget() = rendered_image->texture.texture->texture_2d().rwTexture2D;
								common.GetTargetSize() = rendered_image->texture.texture->get_desc().as_texture().Dimensions.xy;
								compute.set(common);
							}



							if (auto source = dynamic_cast<HAL::Texture2DView*>(info->view.get()))
							{
								compute.set_pipeline<PSOS::Dev::FrameGraph_Debug_Texture2D>();
								{
									Slots::Dev::FrameGraph_Debug_Texture2D tex2d;
									tex2d.GetSource() = *source;
									tex2d.GetSourceSize() = info->resource->get_desc().as_texture().Dimensions.xy;
									tex2d.GetOffset() = offset;
									tex2d.GetScale() = img_scale;

									compute.set(tex2d);
								}
							}
							else if (auto source = dynamic_cast<HAL::Texture3DView*>(info->view.get()))
							{
								compute.set_pipeline<PSOS::Dev::FrameGraph_Debug_Texture3D>();


								mat4x4 view;
								mat4x4 proj;
								camera_3d.set_projection_params(Math::pi / 4, float(debug_size.x) / debug_size.y, 1, 1500);

								camera_3d.frame_move(0.1f);
								camera_3d.update();
								{
									Slots::Dev::FrameGraph_Debug_Texture3D tex3d;
									tex3d.GetSource() = *source;
									tex3d.GetSourceSize() = info->resource->get_desc().as_texture().Dimensions;
									tex3d.GetCamera() = camera_3d.camera_cb.current;

									compute.set(tex3d);
								}
							}

							else
							{
								compute.set_pipeline<PSOS::Dev::FrameGraph_Debug_NotImplemented>();
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


// ===========================================================================
//  resource_preview — universal resource preview widget (source-driven).
// ===========================================================================

resource_preview::resource_preview()
{
	docking     = GUI::dock::FILL;
	width_size  = GUI::size_type::MATCH_PARENT;
	height_size = GUI::size_type::MATCH_PARENT;
	clickable   = true;

	// Docker: preview fills the main area, info panel docks at the bottom.
	// clickable=false lets mouse events fall through to this widget.
	auto docker       = std::make_shared<dock_base>();
	docker->docking   = GUI::dock::FILL;
	docker->clickable = false;
	add_child(docker);

	// Status bar — mip / array selectors.
	{
		auto bar         = std::make_shared<GUI::base>();
		bar->docking     = GUI::dock::TOP;
		bar->height_size = GUI::size_type::FIXED;
		bar->size        = { 0.0f, 56.0f };   // two rows; set_tools drops to one when the second row has nothing to show
		m_bar            = bar;

		auto add_label = [&](const char* text, float x, float w, float y = 4.0f) -> label::ptr
		{
			auto lbl         = std::make_shared<label>();
			lbl->text        = text;
			lbl->font_size   = 13.0f;
			lbl->docking     = GUI::dock::NONE;
			lbl->width_size  = GUI::size_type::FIXED;
			lbl->height_size = GUI::size_type::FIXED;
			lbl->size        = { w, 18.0f };
			lbl->pos         = { x, y };
			bar->add_child(lbl);
			return lbl;
		};

		auto add_button = [&](const char* text, float x, float w) -> button::ptr
		{
			auto btn         = std::make_shared<button>();
			btn->get_label()->text = text;
			btn->docking     = GUI::dock::NONE;
			btn->width_size  = GUI::size_type::FIXED;
			btn->height_size = GUI::size_type::FIXED;
			btn->size        = { w, 20.0f };
			btn->pos         = { x, 3.0f };
			bar->add_child(btn);
			return btn;
		};

		// A tick box with its letter beside it.
		auto add_check = [&](const char* text, float x, bool checked, std::function<void(bool)> on_check,
		                     float y = 4.0f, float label_w = 12.0f) -> std::pair<check_box::ptr, label::ptr>
		{
			auto chk         = std::make_shared<check_box>();
			chk->docking     = GUI::dock::NONE;
			chk->width_size  = GUI::size_type::FIXED;
			chk->height_size = GUI::size_type::FIXED;
			chk->size        = { 18.0f, 18.0f };
			chk->pos         = { x, y };
			bar->add_child(chk);
			chk->set_checked(checked);
			chk->on_check = on_check;
			return { chk, add_label(text, x + 21.0f, label_w, y) };
		};

		// [Reset] [x]R [x]G [x]B [x]A [ ]H   [<] [Mip n] [>]   [<] [Array n] [>]
		// The selectors stay hidden (all of them) until set_source sees more than one entry.
		m_reset_btn = add_button("Reset", 4.0f, 48.0f);
		m_reset_btn->on_click = [this](button::ptr) { reset_view(); };

		static const char* channel_names[4] = { "R", "G", "B", "A" };
		for (int c = 0; c < 4; ++c)
			std::tie(m_channel_check[c], m_channel_label[c]) = add_check(channel_names[c], 58.0f + c * 36.0f, true, [this, c](bool on)
			{
				m_channel_mask = on ? (m_channel_mask | (1 << c)) : (m_channel_mask & ~(1 << c));
				m_histogram->channel_mask = (uint)m_channel_mask;
			});

		m_prev_mip  = add_button("<", 246.0f, 20.0f);
		m_prev_mip->on_click = [this](button::ptr) { select_mip(m_sel_mip - 1); };

		m_mip_combo              = std::make_shared<combo_box>();
		m_mip_combo->docking     = GUI::dock::NONE;
		m_mip_combo->width_size  = GUI::size_type::FIXED;
		m_mip_combo->height_size = GUI::size_type::FIXED;
		m_mip_combo->size        = { 84.0f, 20.0f };
		m_mip_combo->pos         = { 268.0f, 3.0f };
		bar->add_child(m_mip_combo);

		m_next_mip = add_button(">", 354.0f, 20.0f);
		m_next_mip->on_click = [this](button::ptr) { select_mip(m_sel_mip + 1); };

		m_prev_array  = add_button("<", 380.0f, 20.0f);
		m_prev_array->on_click = [this](button::ptr) { select_array(m_sel_array - 1); };

		m_array_combo              = std::make_shared<combo_box>();
		m_array_combo->docking     = GUI::dock::NONE;
		m_array_combo->width_size  = GUI::size_type::FIXED;
		m_array_combo->height_size = GUI::size_type::FIXED;
		m_array_combo->size        = { 84.0f, 20.0f };
		m_array_combo->pos         = { 402.0f, 3.0f };
		bar->add_child(m_array_combo);

		m_next_array = add_button(">", 488.0f, 20.0f);
		m_next_array->on_click = [this](button::ptr) { select_array(m_sel_array + 1); };

		set_selectors_visible(false, false);

		// Second row: [EV -0.0 ======o======] [ ]Lin [ ]Chk [ ]Nrm
		constexpr float row2 = 30.0f;
		m_ev_label = add_label("EV 0.0", 4.0f, 58.0f, row2);

		m_ev_slider              = std::make_shared<float_slider>();
		m_ev_slider->min         = -8.0f;
		m_ev_slider->max         = 8.0f;
		m_ev_slider->value       = 0.0f;
		m_ev_slider->docking     = GUI::dock::NONE;
		m_ev_slider->width_size  = GUI::size_type::FIXED;
		m_ev_slider->height_size = GUI::size_type::FIXED;
		m_ev_slider->size        = { 150.0f, 14.0f };
		m_ev_slider->pos         = { 66.0f, row2 + 3.0f };
		m_ev_slider->on_change   = [this](float ev)
		{
			ev = std::round(ev * 4.0f) / 4.0f;   // quarter-stop steps
			m_exposure = ev;
			m_ev_label->text = std::format("EV {:+.2f}", ev);
		};
		bar->add_child(m_ev_slider);

		auto make_tool = [&](tick_tool& tool, const char* text, float x, std::function<void(bool)> on_check)
		{
			std::tie(tool.box, tool.text) = add_check(text, x, false, on_check, row2, 30.0f);
		};
		make_tool(m_linear_tool,  "Lin", 230.0f, [this](bool on) { m_linear  = on ? 1 : 0; });
		make_tool(m_checker_tool, "Chk", 288.0f, [this](bool on) { m_checker = on ? 1 : 0; });
		make_tool(m_normal_tool,  "Nrm", 346.0f, [this](bool on) { m_normal  = on ? 1 : 0; });

		docker->add_child(bar);
	}

	m_img              = std::make_shared<image>();
	m_img->docking     = GUI::dock::FILL;
	m_img->width_size  = GUI::size_type::NONE;
	m_img->height_size = GUI::size_type::NONE;
	m_img->clickable   = false;
	docker->add_child(m_img);

	auto buffer_tree     = std::make_shared<GUI::Elements::tree<member_item, object_tree_creator>>();
	buffer_tree->docking = GUI::dock::FILL;
	buffer_tree->visible = false;
	docker->add_child(buffer_tree);
	m_buffer_tree        = buffer_tree;

	// Bottom dock — resource information.
	auto bottom_dock  = docker->get_dock(GUI::dock::BOTTOM);
	bottom_dock->size = { 0.0f, 210.0f };

	// Rows of a tab's panel, top to bottom.
	auto add_rows = [](std::shared_ptr<GUI::base> panel)
	{
		return [panel, y = 4.0f](const std::string& text) mutable -> label::ptr
		{
			auto lbl         = std::make_shared<label>();
			lbl->text        = text;
			lbl->font_size   = 13.0f;
			lbl->docking     = GUI::dock::NONE;
			lbl->width_size  = GUI::size_type::MATCH_PARENT;
			lbl->height_size = GUI::size_type::FIXED;
			lbl->size        = { 0.0f, 20.0f };
			lbl->pos         = { 6.0f, y };
			panel->add_child(lbl);
			y += 20.0f;
			return lbl;
		};
	};

	auto info_panel     = std::make_shared<GUI::base>();
	info_panel->docking = GUI::dock::FILL;
	auto add_row        = add_rows(info_panel);

	m_name_lbl   = add_row("Resource: -");
	m_type_lbl   = add_row("Type:     ...");
	m_dim_lbl    = add_row("Size:     ...");
	m_format_lbl = add_row("Format:   ...");
	m_mips_lbl   = add_row("Mips:     ...");
	m_memory_lbl = add_row("Memory:   ...");
	m_flags_lbl  = add_row("Flags:    ...");

	bottom_dock->get_tabs()->add_page("Info", info_panel);

	// Min / max / average of the selected mip and slice per channel, and their histogram, from a GPU pass that
	// runs every frame for 2D / 2D-array textures.
	auto stats_panel     = std::make_shared<GUI::base>();
	stats_panel->docking = GUI::dock::FILL;
	auto add_stat        = add_rows(stats_panel);

	m_min_lbl = add_stat("Min:  -");
	m_max_lbl = add_stat("Max:  -");
	m_avg_lbl = add_stat("Avg:  -");

	m_histogram              = std::make_shared<histogram_view>();
	m_histogram->docking     = GUI::dock::NONE;
	m_histogram->width_size  = GUI::size_type::FIXED;
	m_histogram->height_size = GUI::size_type::FIXED;
	m_histogram->size        = { 512.0f, 100.0f };
	m_histogram->pos         = { 6.0f, 68.0f };
	m_histogram->clickable   = false;
	stats_panel->add_child(m_histogram);

	bottom_dock->get_tabs()->add_page("Stats", stats_panel);

	// Added last so it draws over the image. Not clickable: the mouse events must reach this widget underneath it.
	m_hud              = std::make_shared<preview_hud>();
	m_hud->docking     = GUI::dock::NONE;
	m_hud->width_size  = GUI::size_type::FIXED;
	m_hud->height_size = GUI::size_type::FIXED;
	m_hud->size        = { 330.0f, 70.0f };
	m_hud->clickable   = false;
	m_hud->visible     = false;
	add_child(m_hud);

	auto add_hud_row = [&](float y) -> label::ptr
	{
		auto lbl         = std::make_shared<label>();
		lbl->text        = "-";
		lbl->font_size   = 13.0f;
		lbl->docking     = GUI::dock::NONE;
		lbl->width_size  = GUI::size_type::FIXED;
		lbl->height_size = GUI::size_type::FIXED;
		lbl->size        = { 264.0f, 20.0f };
		lbl->pos         = { 62.0f, y };
		lbl->clickable   = false;
		m_hud->add_child(lbl);
		return lbl;
	};
	m_pixel_lbl = add_hud_row(4.0f);
	m_value_lbl = add_hud_row(25.0f);
	m_hex_lbl   = add_hud_row(46.0f);
}

void preview_hud::draw(Context& c)
{
	const rect b = get_render_bounds();
	c.renderer->draw_color(c, float4(0.06f, 0.07f, 0.09f, 0.94f), b);

	// 1px light frame, then the swatch: left half opaque, right half blended with grey by alpha.
	const float x = b.pos.x + 7.0f;
	const float y = b.pos.y + 8.0f;
	c.renderer->draw_color(c, float4(0.8f, 0.8f, 0.8f, 1.0f), rect{ x, y, 46.0f, 46.0f });
	c.renderer->draw_color(c, float4(color.x, color.y, color.z, 1.0f), rect{ x + 1.0f, y + 1.0f, 22.0f, 44.0f });

	const float grey = 0.35f;
	c.renderer->draw_color(c, float4(grey + (color.x - grey) * alpha, grey + (color.y - grey) * alpha, grey + (color.z - grey) * alpha, 1.0f),
		rect{ x + 23.0f, y + 1.0f, 22.0f, 44.0f });
}

void resource_preview::set_source(std::shared_ptr<HAL::ResourceView> source, const std::string& title)
{
	m_source        = source;
	m_title         = title;
	m_fit_done      = false;
	m_buffer_inited = false;
	m_sel_mip       = 0;
	m_sel_array     = 0;
	m_user_view     = false;
	m_fit_size      = { 0, 0 };
	run_on_ui([this, title]()
	{
		if (m_name_lbl) m_name_lbl->text = "Resource: " + (title.empty() ? std::string("-") : title);
		set_selectors_visible(false, false);
	});
}

void resource_preview::set_selectors_visible(bool mips, bool arrays)
{
	for (auto* w : { (GUI::base*)m_prev_mip.get(), (GUI::base*)m_mip_combo.get(), (GUI::base*)m_next_mip.get() })
		w->visible = mips;
	for (auto* w : { (GUI::base*)m_prev_array.get(), (GUI::base*)m_array_combo.get(), (GUI::base*)m_next_array.get() })
		w->visible = arrays;
}

void resource_preview::set_tools(bool channels, bool flat_view)
{
	for (int c = 0; c < 4; ++c)
	{
		m_channel_check[c]->visible = channels;
		m_channel_label[c]->visible = channels;
	}

	m_pixel_tools         = flat_view;
	m_reset_btn->visible  = flat_view;

	if (!flat_view)
	{
		// No stats pass runs for this resource: don't leave the last texture's numbers in the tab.
		m_min_lbl->text = "Min:  not available for this resource";
		m_max_lbl->text = "Max:";
		m_avg_lbl->text = "Avg:";
		std::fill(m_histogram->bins.begin(), m_histogram->bins.end(), 0u);
		m_histogram->total = 0;
	}

	// Second row: exposure and the linear view for anything the channel mask reaches; the checkerboard and the
	// normal-map view only where alpha and a flat texel exist.
	m_ev_label->visible  = channels;
	m_ev_slider->visible = channels;
	m_linear_tool.show(channels);
	m_checker_tool.show(flat_view);
	m_normal_tool.show(flat_view);
	m_bar->size = { 0.0f, channels ? 56.0f : 28.0f };

	if (!flat_view)
	{
		m_cursor_x     = -1;
		m_cursor_y     = -1;
		m_hud->visible = false;
	}
}

void resource_preview::reset_view()
{
	// The view: refit on the next render.
	m_user_view = false;
	m_refit     = 1;

	// The display settings. Setting the tick boxes / slider fires their handlers, which write the values the
	// render thread reads, so the controls and the values can't disagree.
	m_ev_slider->value = 0.0f;
	m_exposure         = 0.0f;
	m_ev_label->text   = "EV 0.0";

	for (auto* tool : { &m_linear_tool, &m_checker_tool, &m_normal_tool })
		tool->box->set_checked(false);
	m_linear = m_checker = m_normal = 0;

	for (int c = 0; c < 4; ++c)
		m_channel_check[c]->set_checked(true);
	m_channel_mask = 0xF;
	m_histogram->channel_mask = 0xF;
}

void histogram_view::draw(Context& c)
{
	const rect b = get_render_bounds();
	c.renderer->draw_color(c, float4(0.05f, 0.06f, 0.08f, 0.95f), b);

	if (total == 0)
		return;

	// Log scale against the sample count, like a luminance histogram: a texture that is mostly one value
	// doesn't flatten everything else. One column per bin, the bars of the enabled channels overlapping.
	const float denom = std::log(1.0f + float(total));
	const float col_w = b.w / 256.0f;
	static const float4 colors[4] = { { 1.0f, 0.25f, 0.25f, 0.55f }, { 0.25f, 1.0f, 0.25f, 0.55f },
	                                  { 0.35f, 0.55f, 1.0f, 0.55f }, { 0.75f, 0.75f, 0.75f, 0.40f } };
	for (int ch = 0; ch < 4; ++ch)
	{
		if (!(channel_mask & (1u << ch))) continue;

		for (int bin = 0; bin < 256; ++bin)
		{
			const float h = std::log(1.0f + float(bins[ch * 256 + bin])) / denom * (b.h - 2.0f);
			if (h < 0.5f) continue;
			c.renderer->draw_color(c, colors[ch], rect{ b.pos.x + bin * col_w, b.pos.y + b.h - 1.0f - h, std::max(col_w, 1.0f), h });
		}
	}
}

void resource_preview::select_mip(int mip)
{
	m_sel_mip = Math::clamp(mip, 0, std::max(m_mip_count - 1, 0));
	m_mip_combo->get_label()->text = "Mip " + std::to_string(m_sel_mip);
}

void resource_preview::select_array(int index)
{
	m_sel_array = Math::clamp(index, 0, std::max(m_array_count - 1, 0));
	m_array_combo->get_label()->text = "Array " + std::to_string(m_sel_array);
}

void resource_preview::refresh_source(std::shared_ptr<HAL::ResourceView> source)
{
	if (source == m_source) return; // same placement as last frame — nothing to do

	if (!source || !m_source)
	{
		set_source(source, m_title);
		return;
	}

	if (m_source->get_desc() == source->get_desc())
		m_source = source; // aliasing moved the allocation — keep pan/zoom/UI state
	else
		set_source(source, m_title); // resize/recreate — full reset (fit, labels, combos)
}

void resource_preview::render(FrameGraph::FrameContext* context)
{
	if (!m_source) return;

	auto res_desc = m_source->get_desc();

	if (res_desc.is_texture())
	{
		// Read view size cached by update_layout on the UI thread.
		uint2 SZ = { (UINT)m_view_w, (UINT)m_view_h };

		// Create / resize render target — render-thread-owned via m_current_tex.
		bool need_new = !m_current_tex ||
			uint2(m_current_tex->get_desc().as_texture().Dimensions.xy) != SZ;
		if (need_new)
		{
			HAL::ResourceDesc desc = HAL::ResourceDesc::Tex2D(
				HAL::Format::R8G8B8A8_UNORM, { SZ }, 1, 1,
				HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess);
			m_current_tex = std::make_shared<Texture>(RenderSystem::get().device(), desc);

			// Hand the new texture to the image widget on the UI thread.
			auto tex = m_current_tex;
			run_on_ui([this, tex]()
			{
				m_img->texture.texture = tex->texture_2d();
				m_img->visible         = true;
				m_buffer_tree->visible = false;
			});
		}

		if (!m_current_tex) return;

		// Runs after set_source (initial or full reset on a desc change), not
		// only when the preview target got recreated — otherwise labels/fit
		// stay stale when the SOURCE changes but the widget size doesn't.
		if (!m_fit_done)
		{
			{
				m_fit_done = true;
				auto   d   = res_desc.as_texture().Dimensions;
				float2 src = { float(d.x), float(d.y) };
				float2 dst = float2(SZ);
				float  fit = std::min(dst.x / src.x, dst.y / src.y);
				{
					Thread::Lockable::guard lk(m_state_mutex);
					m_scale = fit;
					m_pan   = (dst - src * fit) * 0.5f;
				}

				const bool is_cube = !!dynamic_cast<HAL::CubeView*>(m_source.get());
				const bool is_3d   = !is_cube && (d.z > 1);
				std::string t = is_cube ? std::string(resource_icons::cube) + " TextureCube"
				              : is_3d   ? std::string(resource_icons::texture_3d) + " Texture3D"
				                        : std::string(resource_icons::texture_2d) + " Texture2D";
				std::string s = std::to_string(d.x) + "x" + std::to_string(d.y)
				              + (is_cube ? "x6"
				              : is_3d   ? "x" + std::to_string(d.z) : "");
				run_on_ui([this, t, s, channels = !is_3d, flat_view = !is_cube && !is_3d]()
				{
					m_type_lbl->text = "Type:     " + t;
					m_dim_lbl->text  = "Size:     " + s;
					set_tools(channels, flat_view);
				});

				// Info tab: the TextureDesc as stored, the format, and what the texture costs in memory.
				{
					const HAL::TextureDesc& td = res_desc.as_texture();
					HAL::Format format = td.Format;

					auto bytes_text = [](uint64 n) -> std::string
					{
						if (n >= (1ull << 30)) return std::format("{:.2f} GB", double(n) / double(1ull << 30));
						if (n >= (1ull << 20)) return std::format("{:.2f} MB", double(n) / double(1ull << 20));
						if (n >= (1ull << 10)) return std::format("{:.1f} KB", double(n) / double(1ull << 10));
						return std::format("{} B", n);
					};

					// The data itself: every mip of every slice at the format's size, depth slices included.
					uint64 data_bytes = 0;
					for (uint mip = 0; mip < td.MipLevels; ++mip)
					{
						const uint3 mip_size = td.get_size(mip);
						data_bytes += format.surface_info(uint2(mip_size.x, mip_size.y)).numBytes * std::max(mip_size.z, 1u);
					}
					data_bytes *= td.ArraySize;

					// What the device actually reserves for it (alignment and padding included).
					std::string memory = bytes_text(data_bytes) + " data";
					if (!res_desc.is_virtual())
						memory += ", " + bytes_text(RenderSystem::get().device().get_alloc_info(res_desc).size) + " allocated";

					std::string format_text = format.to_string();
					if (format.is_srgb())        format_text += ", sRGB";
					if (format.is_compressed())  format_text += ", block-compressed";
					else                         format_text += std::format(", {} B/texel", format.size());

					std::string desc_text = std::format("{} x {} x {}, array {}, {} mips", td.Dimensions.x, td.Dimensions.y, td.Dimensions.z, td.ArraySize, td.MipLevels);

					std::string flags_text;
					auto add_flag = [&](HAL::ResFlags f, const char* name)
					{
						if (!check(res_desc.Flags & f)) return;
						if (!flags_text.empty()) flags_text += " | ";
						flags_text += name;
					};
					add_flag(HAL::ResFlags::ShaderResource,  "SRV");
					add_flag(HAL::ResFlags::UnorderedAccess, "UAV");
					add_flag(HAL::ResFlags::RenderTarget,    "RT");
					add_flag(HAL::ResFlags::DepthStencil,    "DS");
					add_flag(HAL::ResFlags::Virtual,         "Virtual");
					add_flag(HAL::ResFlags::Raytracing,      "RT-AS");
					add_flag(HAL::ResFlags::Swapchain,       "Swapchain");
					add_flag(HAL::ResFlags::Immutable,       "Immutable");
					if (flags_text.empty()) flags_text = "-";

					run_on_ui([this, format_text, desc_text, memory, flags_text]()
					{
						m_format_lbl->text = "Format:   " + format_text;
						m_mips_lbl->text   = "Desc:     " + desc_text;
						m_memory_lbl->text = "Memory:   " + memory;
						m_flags_lbl->text  = "Flags:    " + flags_text;
					});
				}

				UINT mip_count   = res_desc.as_texture().MipLevels;
				UINT array_count = res_desc.as_texture().ArraySize;
				run_on_ui([this, mip_count, array_count]()
				{
					m_mip_count   = (int)mip_count;
					m_array_count = (int)array_count;

					m_mip_combo->remove_items();
					for (UINT i = 0; i < mip_count; ++i)
					{
						auto item = m_mip_combo->add_item("Mip " + std::to_string(i));
						item->on_select = [this, i]() { select_mip((int)i); };
					}
					m_array_combo->remove_items();
					for (UINT i = 0; i < array_count; ++i)
					{
						auto item = m_array_combo->add_item("Array " + std::to_string(i));
						item->on_select = [this, i]() { select_array((int)i); };
					}
					select_mip(0);
					select_array(0);
					set_selectors_visible(mip_count > 1, array_count > 1);
				});
			}
		}

		// The first fit can run before the window has its final size (the image then ends up cut off), so
		// until the user pans or zooms, fit again whenever the view size changes.
		if (m_refit || (!m_user_view && m_fit_size != SZ))
		{
			m_refit = 0;
			auto   d   = res_desc.as_texture().Dimensions;
			float2 src = { float(d.x), float(d.y) };
			float2 dst = float2(SZ);
			float  fit = std::min(dst.x / src.x, dst.y / src.y);

			Thread::Lockable::guard lk(m_state_mutex);
			m_scale    = fit;
			m_pan      = (dst - src * fit) * 0.5f;
			m_fit_size = SZ;
		}

		auto& compute = context->get_list()->get_compute();

		if (!m_stats)
			m_stats = std::make_shared<HAL::StructuredBufferView<uint>>(RenderSystem::get().device(), preview_stats::words,
				HAL::counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);

		const int cursor_x = m_cursor_x;
		const int cursor_y = m_cursor_y;

		// Rebound before every dispatch that touches the stats buffer: each bind records its UAV usage,
		// which is what makes the HAL put barriers between clear -> histogram -> draw (see TonemapPass).
		auto bind_common = [&](uint extra_flags = 0)
		{
			Slots::Dev::FrameGraph_Debug_Common common;
			common.GetTarget()             = m_current_tex->texture_2d().rwTexture2D;
			common.GetTargetSize()         = SZ;
			common.GetSelectedMip()        = (UINT)m_sel_mip;
			common.GetSelectedArrayIndex() = (UINT)m_sel_array;
			common.GetChannelMask()        = (UINT)m_channel_mask;
			common.GetCursor()             = int2(cursor_x, cursor_y);
			common.GetFlags()              = (m_linear ? 4u : 0u) | (m_checker ? 8u : 0u) | (m_normal ? 16u : 0u) | extra_flags;
			common.GetExposure()           = m_exposure;
			common.GetStats()              = m_stats->rwStructuredBuffer;
			compute.set(common);
		};
		bind_common();
		bool read_pixel = false;

		if (auto* src = dynamic_cast<HAL::Texture2DView*>(m_source.get()))
		{
			float2 snap_pan;
			float  snap_scale;
			{
				Thread::Lockable::guard lk(m_state_mutex);
				snap_pan   = m_pan;
				snap_scale = m_scale;
			}
			UINT array_size = res_desc.as_texture().ArraySize;

			// A resource with no ResFlags::ShaderResource (UAV-only or
			// depth-stencil-only, e.g. GBuffer_Quality) has no SRV descriptor
			// written at all -- Texture2DView::init() only calls .create() on
			// texture2D/texture2DArray inside `if (ShaderResource)`, while the
			// descriptor SLOTS are always allocated. Binding an unwritten slot
			// samples whatever unrelated resource last occupied it and shows
			// its contents here, with no D3D12 validation error. Skip instead.
			const bool srv_ok = (array_size > 1) ? src->texture2DArray.is_written()
			                                     : src->texture2D.is_written();
			if (!srv_ok)
				return;

			const uint2 source_size = res_desc.as_texture().Dimensions.xy;
			auto bind_source = [&]()
			{
				if (array_size > 1)
				{
					Slots::Dev::FrameGraph_Debug_Texture2DArray tex2darr;
					tex2darr.GetSource()     = *src;
					tex2darr.GetSourceSize() = source_size;
					tex2darr.GetOffset()     = snap_pan;
					tex2darr.GetScale()      = snap_scale;
					compute.set(tex2darr);
				}
				else
				{
					Slots::Dev::FrameGraph_Debug_Texture2D tex2d;
					tex2d.GetSource()     = *src;
					tex2d.GetSourceSize() = source_size;
					tex2d.GetOffset()     = snap_pan;
					tex2d.GetScale()      = snap_scale;
					compute.set(tex2d);
				}
			};

			// Always computed for 2D / 2D-array textures: the Stats tab shows the histogram and min / max / average.
			{
				PROFILE(L"preview_histogram");
				// Same grid as the histogram shaders: at most preview_stats::max_grid samples per axis of the selected mip.
				const uint mip = (uint)m_sel_mip;
				const uint2 grid = { std::min(std::max(source_size.x >> mip, 1u), preview_stats::max_grid),
				                     std::min(std::max(source_size.y >> mip, 1u), preview_stats::max_grid) };

				// The histogram shader zeroes the stats words itself when asked (flag bit 1): a first dispatch with
				// 128 x 9 threads covers all preview_stats::words, then the accumulating one runs. No clear_uav needed.
				if (array_size > 1)
					compute.set_pipeline<PSOS::Dev::FrameGraph_Debug_Histogram2DArray>();
				else
					compute.set_pipeline<PSOS::Dev::FrameGraph_Debug_Histogram2D>();
				bind_common(2u);
				bind_source();
				compute.dispatch(ivec2(128, 9), ivec2(8, 8));

				bind_common();
				bind_source();
				compute.dispatch(ivec2(grid), ivec2(8, 8));
				bind_common();
			}

			if (array_size > 1)
				compute.set_pipeline<PSOS::Dev::FrameGraph_Debug_Texture2DArray>();
			else
				compute.set_pipeline<PSOS::Dev::FrameGraph_Debug_Texture2D>();
			bind_source();
			read_pixel = true;
		}
		else if (auto* src = dynamic_cast<HAL::Texture3DView*>(m_source.get()))
		{
			// Same class of bug as the Texture2D srv_ok check above: a UAV-only
			// volume (no ResFlags::ShaderResource, e.g. voxel GI volumes) never
			// has texture3D.create() called, so the descriptor slot is allocated
			// but unwritten -- sampling it shows whatever unrelated resource
			// last occupied that heap slot, and since heap slots cycle frame to
			// frame, a different one each time (the "flickering garbage" symptom).
			if (!src->texture3D.is_written())
				return;

			auto snap_cam_cb = [&]() {
				Thread::Lockable::guard lk(m_state_mutex);
				m_cam_3d.set_projection_params(Math::pi / 4,
					float(SZ.x) / float(SZ.y), 1.0f, 1500.0f);
				m_cam_3d.frame_move(0.1f);
				m_cam_3d.update();
				return m_cam_3d.camera_cb.current;
			}();
			compute.set_pipeline<PSOS::Dev::FrameGraph_Debug_Texture3D>();
			Slots::Dev::FrameGraph_Debug_Texture3D tex3d;
			tex3d.GetSource()     = *src;
			tex3d.GetSourceSize() = res_desc.as_texture().Dimensions;
			tex3d.GetCamera()     = snap_cam_cb;
			compute.set(tex3d);
		}
		else if (auto* src = dynamic_cast<HAL::CubeView*>(m_source.get()))
		{
			// Same as above: textureCube is only created under ShaderResource.
			if (!src->textureCube.is_written())
				return;

			compute.set_pipeline<PSOS::Dev::FrameGraph_Debug_TextureCube>();
			Slots::Dev::FrameGraph_Debug_TextureCube cube;
			cube.GetSource()     = src->textureCube;
			cube.GetSourceSize() = res_desc.as_texture().Dimensions.xy;
			compute.set(cube);
		}
		else
		{
			compute.set_pipeline<PSOS::Dev::FrameGraph_Debug_NotImplemented>();
		}
		compute.dispatch(uint3(SZ, 1));

		const bool cursor_inside = cursor_x >= 0 && cursor_y >= 0 && cursor_x < (int)SZ.x && cursor_y < (int)SZ.y;

		// Every frame for 2D / 2D-array textures: the stats words (min / max / average, histogram bins) always, and
		// the pixel under the cursor when it is over the image.
		if (read_pixel)
		{
			PROFILE(L"preview_pixel_readback");
			// This runs when the GPU copy finishes, which can be after the preview window was closed: hold the
			// widget weakly, and do nothing once it is gone or has left the UI (run_on_ui then runs its task
			// right here, on the wrong thread, against labels that are being torn down).
			context->get_list()->get_copy().read<uint>(*m_stats, 0, preview_stats::words,
				[weak = std::weak_ptr<resource_preview>(get_ptr<resource_preview>()), cursor_inside](std::span<uint> r)
			{
				auto self = weak.lock();
				if (!self || !self->user_ui || r.size() < preview_stats::words) return;

				// Min / max / average per channel; min and max are stored as order-preserving float bits, the sum as
				// whole part + 12-bit fraction of each sample clamped to 0..255 (see stats.hlsli).
				std::string min_text, max_text, avg_text;
				{
					auto from_ordered = [](uint u)
					{
						const uint bits = (u & 0x80000000u) ? (u & 0x7FFFFFFFu) : ~u;
						float f;
						std::memcpy(&f, &bits, sizeof(f));
						return f;
					};
					const double samples = std::max<double>(r[7], 1.0);
					float lo[4], hi[4], avg[4];
					for (int c = 0; c < 4; ++c)
					{
						const uint at = preview_stats::minmax_at + c * 4;
						lo[c]  = from_ordered(r[at]);
						hi[c]  = from_ordered(r[at + 1]);
						avg[c] = float((double(r[at + 2]) + double(r[at + 3]) / preview_stats::sum_fraction) / samples);
					}
					min_text = std::format("Min:  {:.4f}  {:.4f}  {:.4f}  {:.4f}", lo[0], lo[1], lo[2], lo[3]);
					max_text = std::format("Max:  {:.4f}  {:.4f}  {:.4f}  {:.4f}", hi[0], hi[1], hi[2], hi[3]);
					avg_text = std::format("Avg:  {:.4f}  {:.4f}  {:.4f}  {:.4f}    (R G B A, {} samples)", avg[0], avg[1], avg[2], avg[3], uint(samples));
				}

				// The histogram bins follow the first 24 words.
				std::vector<uint> bins(r.begin() + preview_stats::readback_words, r.begin() + preview_stats::words);
				const uint total = r[7];

				// Over the image widget but outside the texture (the checker margin) the pick is not valid.
				const bool pick_valid = cursor_inside && r[6] != 0;

				std::string pixel = "Pixel   -", value = "RGBA    -", hex = "Hex     -";
				float v[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				if (pick_valid)
				{
					std::memcpy(v, r.data(), sizeof(v));
					auto byte = [](float f) { return (unsigned)(std::clamp(f, 0.0f, 1.0f) * 255.0f + 0.5f); };
					pixel = std::format("Pixel   {}, {}", (int)r[4], (int)r[5]);
					value = std::format("RGBA    {:.4f}  {:.4f}  {:.4f}  {:.4f}", v[0], v[1], v[2], v[3]);
					hex   = std::format("Hex     #{:02X}{:02X}{:02X}{:02X}", byte(v[0]), byte(v[1]), byte(v[2]), byte(v[3]));
				}
				self->run_on_ui([self, cursor_inside, pick_valid, pixel, value, hex, min_text, max_text, avg_text,
				                 bins = std::move(bins), total, v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3]]() mutable
				{
					self->m_min_lbl->text = min_text;
					self->m_max_lbl->text = max_text;
					self->m_avg_lbl->text = avg_text;
					self->m_histogram->bins  = std::move(bins);
					self->m_histogram->total = total;

					if (!cursor_inside) return;

					// The panel follows the pick: gone over the margin around the texture, back over the texture.
					self->m_pick_valid = pick_valid;
					self->m_hud->visible = pick_valid && self->m_pixel_tools && self->m_cursor_x >= 0;
					if (!pick_valid) return;

					self->m_pixel_lbl->text = pixel;
					self->m_value_lbl->text = value;
					self->m_hex_lbl->text   = hex;

					// The swatch shows the texel the way the preview itself does: same exposure and display gamma.
					const float gain  = std::exp2(float(self->m_exposure));
					const float gamma = self->m_linear ? 1.0f : 1.0f / 2.2f;
					auto shown = [gain, gamma](float f) { return std::pow(std::max(f * gain, 0.0f), gamma); };
					self->m_hud->color = { shown(v0), shown(v1), shown(v2), 1.0f };
					self->m_hud->alpha = std::clamp(v3, 0.0f, 1.0f);
				});
			});
		}
	}
	else
	{
		if (auto* src = dynamic_cast<HAL::StructuredBufferViewBase*>(m_source.get()))
		{
			if (!m_buffer_inited)
			{
				m_buffer_inited  = true;
				size_t count     = src->get_count();
				size_t elem_size = src->get_element_size();
				context->get_list()->get_copy().read_buffer(
					src->resource.get(), 0, count * elem_size,
					// Runs when the GPU copy finishes: the widget may be closed and the source replaced by then, so
					// hold the view itself and the widget weakly instead of `src` and `this`.
					[weak = std::weak_ptr<resource_preview>(get_ptr<resource_preview>()), view = m_source, count, elem_size](std::span<std::byte> memory)
					{
						auto self = weak.lock();
						auto* sb  = dynamic_cast<HAL::StructuredBufferViewBase*>(view.get());
						if (!self || !self->user_ui || !sb) return;

						auto tree_data = sb->describe(memory.data(), memory.size());
						self->run_on_ui([self, tree_data, count, elem_size]()
						{
							self->m_type_lbl->text = std::string("Type:     ") + resource_icons::buffer + " Buffer";
							self->m_dim_lbl->text  = "Size:     "
								+ std::to_string(count) + " x "
								+ std::to_string(elem_size) + "B";
							auto bt = std::static_pointer_cast<GUI::Elements::tree<member_item, object_tree_creator>>(self->m_buffer_tree);
							bt->contents->remove_all();
							bt->init(tree_data.get());
						});
					});
				run_on_ui([this]()
				{
					m_img->visible         = false;
					m_buffer_tree->visible = true;
					set_tools(false, false);
				});
			}
		}
		else
		{
			if (!m_buffer_inited)
			{
				m_buffer_inited = true;
				uint64 sz = res_desc.as_buffer().SizeInBytes;
				run_on_ui([this, sz]()
				{
					m_type_lbl->text       = std::string("Type:     ") + resource_icons::buffer + " Buffer";
					m_dim_lbl->text        = "Size:     " + std::to_string(sz) + "B";
					m_img->visible         = false;
					m_buffer_tree->visible = false;
					set_tools(false, false);
				});
			}
		}
	}
}

bool resource_preview::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
	if (button == mouse_button::RIGHT)
	{
		pressed      = (action == mouse_action::DOWN);
		m_drag_start = pos;
		if (!pressed) { m_dragging = false; set_movable(false); }
	}
	return false;
}

bool resource_preview::on_mouse_move(vec2 pos)
{
	{
		const rect rb = m_img->get_render_bounds();
		const vec2 local = pos - rb.pos;
		const bool inside = local.x >= 0 && local.y >= 0 && local.x < rb.w && local.y < rb.h;
		m_cursor_x = inside && m_pixel_tools ? (int)local.x : -1;
		m_cursor_y = inside && m_pixel_tools ? (int)local.y : -1;

		// Hover panel: beside the cursor, flipped to the other side when it would leave the image.
		m_hud->visible = inside && m_pixel_tools && m_pick_valid && m_img->visible && !m_dragging;
		if (inside && m_pixel_tools)
		{
			const float w = m_hud->size.get().x, h = m_hud->size.get().y;
			const vec2 origin = rb.pos - get_render_bounds().pos;
			float x = local.x + 16.0f;
			float y = local.y + 18.0f;
			if (x + w > rb.w) x = local.x - w - 10.0f;
			if (y + h > rb.h) y = local.y - h - 10.0f;
			m_hud->pos = origin + vec2(std::max(x, 0.0f), std::max(y, 0.0f));
		}
	}

	if (pressed && !m_dragging && (pos - m_drag_start).length() > 3.0f)
	{
		m_dragging  = true;
		m_user_view = true;
		m_prev      = pos;
		set_movable(true);
	}
	if (m_dragging)
	{
		vec2 delta = pos - m_prev;
		{
			Thread::Lockable::guard lk(m_state_mutex);
			m_pan += float2(delta);
			m_cam_3d.input(float2(delta) / 1000.0f);
		}
		m_prev = pos;
	}
	return m_dragging;
}

void resource_preview::on_mouse_leave(vec2 pos)
{
	base::on_mouse_leave(pos);

	// The GUI also calls this after every on_mouse_move that returned false (not dragging), so only a position
	// outside the image is a real leave.
	const rect rb = m_img->get_render_bounds();
	const vec2 local = pos - rb.pos;
	if (local.x >= 0 && local.y >= 0 && local.x < rb.w && local.y < rb.h)
		return;

	m_cursor_x     = -1;
	m_cursor_y     = -1;
	m_hud->visible = false;
}

bool resource_preview::on_wheel(mouse_wheel type, float value, vec2 wpos)
{
	m_user_view = true;
	vec2 pivot = wpos - m_img->get_render_bounds().pos;
	{
		Thread::Lockable::guard lk(m_state_mutex);
		float prev = m_scale;
		m_cam_3d.input(value / 100.0f);
		m_scale *= 1.0f + value / 10.0f;
		m_scale  = Math::clamp(m_scale, 0.01f, 50.0f);
		m_pan    = m_scale / prev * (m_pan - float2(pivot)) + float2(pivot);
	}
	return true;
}

::sizer resource_preview::update_layout(::sizer r, float scale)
{
	::sizer result = base::update_layout(r, scale);
	const rect rb = m_img->get_render_bounds();
	m_view_w = std::max(64, (int)rb.w);
	m_view_h = std::max(64, (int)rb.h);
	return result;
}
