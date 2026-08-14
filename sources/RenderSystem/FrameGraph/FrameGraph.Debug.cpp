module FrameGraphDebug;
import RenderSystem;

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

					auto pass = s.passes.front();

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
							auto texture = std::make_shared<Texture>(RenderSystem::get().device(), desc, TextureLayout::SHADER_RESOURCE);
							rendered_image->texture.texture = std::make_shared<Texture>(RenderSystem::get().device(), desc);
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
		bar->size        = { 0.0f, 26.0f };

		auto mip_lbl         = std::make_shared<label>();
		mip_lbl->text        = "Mip:";
		mip_lbl->font_size   = 10.0f;
		mip_lbl->docking     = GUI::dock::NONE;
		mip_lbl->width_size  = GUI::size_type::FIXED;
		mip_lbl->height_size = GUI::size_type::FIXED;
		mip_lbl->size        = { 28.0f, 16.0f };
		mip_lbl->pos         = { 4.0f, 5.0f };
		bar->add_child(mip_lbl);

		m_mip_combo              = std::make_shared<combo_box>();
		m_mip_combo->docking     = GUI::dock::NONE;
		m_mip_combo->width_size  = GUI::size_type::FIXED;
		m_mip_combo->height_size = GUI::size_type::FIXED;
		m_mip_combo->size        = { 84.0f, 20.0f };
		m_mip_combo->pos         = { 34.0f, 3.0f };
		m_mip_combo->visible     = false;
		bar->add_child(m_mip_combo);

		auto arr_lbl         = std::make_shared<label>();
		arr_lbl->text        = "Array:";
		arr_lbl->font_size   = 10.0f;
		arr_lbl->docking     = GUI::dock::NONE;
		arr_lbl->width_size  = GUI::size_type::FIXED;
		arr_lbl->height_size = GUI::size_type::FIXED;
		arr_lbl->size        = { 36.0f, 16.0f };
		arr_lbl->pos         = { 124.0f, 5.0f };
		bar->add_child(arr_lbl);

		m_array_combo              = std::make_shared<combo_box>();
		m_array_combo->docking     = GUI::dock::NONE;
		m_array_combo->width_size  = GUI::size_type::FIXED;
		m_array_combo->height_size = GUI::size_type::FIXED;
		m_array_combo->size        = { 84.0f, 20.0f };
		m_array_combo->pos         = { 162.0f, 3.0f };
		m_array_combo->visible     = false;
		bar->add_child(m_array_combo);

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
	bottom_dock->size = { 0.0f, 90.0f };

	auto info_panel     = std::make_shared<GUI::base>();
	info_panel->docking = GUI::dock::FILL;

	float info_y   = 4.0f;
	auto  add_row  = [&](const std::string& text) -> label::ptr
	{
		auto lbl         = std::make_shared<label>();
		lbl->text        = text;
		lbl->font_size   = 10.0f;
		lbl->docking     = GUI::dock::NONE;
		lbl->width_size  = GUI::size_type::MATCH_PARENT;
		lbl->height_size = GUI::size_type::FIXED;
		lbl->size        = { 0.0f, 18.0f };
		lbl->pos         = { 6.0f, info_y };
		info_panel->add_child(lbl);
		info_y += 18.0f;
		return lbl;
	};

	m_name_lbl = add_row("Resource: -");
	m_type_lbl = add_row("Type:     ...");
	m_dim_lbl  = add_row("Size:     ...");

	bottom_dock->get_tabs()->add_page("Info", info_panel);
}

void resource_preview::set_source(std::shared_ptr<HAL::ResourceView> source, const std::string& title)
{
	m_source        = source;
	m_title         = title;
	m_fit_done      = false;
	m_buffer_inited = false;
	run_on_ui([this, title]()
	{
		if (m_name_lbl) m_name_lbl->text = "Resource: " + (title.empty() ? std::string("-") : title);
		m_mip_combo->visible   = false;
		m_array_combo->visible = false;
	});
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
				std::string t = is_cube ? "TextureCube"
				              : is_3d   ? "Texture3D" : "Texture2D";
				std::string s = std::to_string(d.x) + "x" + std::to_string(d.y)
				              + (is_cube ? "x6"
				              : is_3d   ? "x" + std::to_string(d.z) : "");
				run_on_ui([this, t, s]()
				{
					m_type_lbl->text = "Type:     " + t;
					m_dim_lbl->text  = "Size:     " + s;
				});

				UINT mip_count   = res_desc.as_texture().MipLevels;
				UINT array_count = res_desc.as_texture().ArraySize;
				run_on_ui([this, mip_count]()
				{
					m_mip_combo->remove_items();
					for (UINT i = 0; i < mip_count; ++i)
					{
						auto item = m_mip_combo->add_item("Mip " + std::to_string(i));
						item->on_select = [this, i]() { m_sel_mip = (int)i; };
					}
					if (mip_count > 0)
						m_mip_combo->get_label()->text = "Mip 0";
					m_mip_combo->visible = (mip_count > 1);
				});
				run_on_ui([this, array_count]()
				{
					m_array_combo->remove_items();
					for (UINT i = 0; i < array_count; ++i)
					{
						auto item = m_array_combo->add_item("Array " + std::to_string(i));
						item->on_select = [this, i]() { m_sel_array = (int)i; };
					}
					if (array_count > 0)
						m_array_combo->get_label()->text = "Array 0";
					m_array_combo->visible = (array_count > 1);
				});
			}
		}

		auto& compute = context->get_list()->get_compute();
		{
			Slots::FrameGraph_Debug_Common common;
			common.GetTarget()             = m_current_tex->texture_2d().rwTexture2D;
			common.GetTargetSize()         = SZ;
			common.GetSelectedMip()        = (UINT)m_sel_mip;
			common.GetSelectedArrayIndex() = (UINT)m_sel_array;
			compute.set(common);
		}

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

			if (array_size > 1)
			{
				compute.set_pipeline<PSOS::FrameGraph_Debug_Texture2DArray>();
				Slots::FrameGraph_Debug_Texture2DArray tex2darr;
				tex2darr.GetSource()     = *src;
				tex2darr.GetSourceSize() = res_desc.as_texture().Dimensions.xy;
				tex2darr.GetOffset()     = snap_pan;
				tex2darr.GetScale()      = snap_scale;
				compute.set(tex2darr);
			}
			else
			{
				compute.set_pipeline<PSOS::FrameGraph_Debug_Texture2D>();
				Slots::FrameGraph_Debug_Texture2D tex2d;
				tex2d.GetSource()     = *src;
				tex2d.GetSourceSize() = res_desc.as_texture().Dimensions.xy;
				tex2d.GetOffset()     = snap_pan;
				tex2d.GetScale()      = snap_scale;
				compute.set(tex2d);
			}
		}
		else if (auto* src = dynamic_cast<HAL::Texture3DView*>(m_source.get()))
		{
			auto snap_cam_cb = [&]() {
				Thread::Lockable::guard lk(m_state_mutex);
				m_cam_3d.set_projection_params(Math::pi / 4,
					float(SZ.x) / float(SZ.y), 1.0f, 1500.0f);
				m_cam_3d.frame_move(0.1f);
				m_cam_3d.update();
				return m_cam_3d.camera_cb.current;
			}();
			compute.set_pipeline<PSOS::FrameGraph_Debug_Texture3D>();
			Slots::FrameGraph_Debug_Texture3D tex3d;
			tex3d.GetSource()     = *src;
			tex3d.GetSourceSize() = res_desc.as_texture().Dimensions;
			tex3d.GetCamera()     = snap_cam_cb;
			compute.set(tex3d);
		}
		else if (auto* src = dynamic_cast<HAL::CubeView*>(m_source.get()))
		{
			compute.set_pipeline<PSOS::FrameGraph_Debug_TextureCube>();
			Slots::FrameGraph_Debug_TextureCube cube;
			cube.GetSource()     = src->textureCube;
			cube.GetSourceSize() = res_desc.as_texture().Dimensions.xy;
			compute.set(cube);
		}
		else
		{
			compute.set_pipeline<PSOS::FrameGraph_Debug_NotImplemented>();
		}
		compute.dispatch(uint3(SZ, 1));
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
					[this, src, count, elem_size](std::span<std::byte> memory)
					{
						auto tree_data = src->describe(memory.data(), memory.size());
						run_on_ui([this, tree_data, count, elem_size]()
						{
							m_type_lbl->text = "Type:     Buffer";
							m_dim_lbl->text  = "Size:     "
								+ std::to_string(count) + " x "
								+ std::to_string(elem_size) + "B";
							auto bt = std::static_pointer_cast<GUI::Elements::tree<member_item, object_tree_creator>>(m_buffer_tree);
							bt->contents->remove_all();
							bt->init(tree_data.get());
						});
					});
				run_on_ui([this]()
				{
					m_img->visible         = false;
					m_buffer_tree->visible = true;
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
					m_type_lbl->text       = "Type:     Buffer";
					m_dim_lbl->text        = "Size:     " + std::to_string(sz) + "B";
					m_img->visible         = false;
					m_buffer_tree->visible = false;
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
	if (pressed && !m_dragging && (pos - m_drag_start).length() > 3.0f)
	{
		m_dragging = true;
		m_prev     = pos;
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

bool resource_preview::on_wheel(mouse_wheel type, float value, vec2 wpos)
{
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
