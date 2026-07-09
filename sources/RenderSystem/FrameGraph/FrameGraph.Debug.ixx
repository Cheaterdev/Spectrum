export module FrameGraphDebug;

export import "defines.h";
import Core;
import HAL;


import GUI;
import FrameGraph;
import Graphics;
			import windows;

using namespace HAL;

// ---------------------------------------------------------------------------
//  resource_preview — universal GPU resource preview widget.
//
//  Given any HAL::ResourceView (texture 2D / 2D-array / 3D / cube, or a
//  buffer) via set_source(), it renders a pannable / zoomable preview into an
//  image (3D volumes get an orbitable camera; buffers show a value tree).
//  Call render(context) from inside a pass while the source is readable — the
//  FrameGraph timeline does this from process_debug_resource; an asset window
//  can drive it from its own pass.
// ---------------------------------------------------------------------------
export class resource_preview : public GUI::base
{
	GUI::Elements::image::ptr                                       m_img;
	// concrete type is GUI::Elements::tree<member_item, object_tree_creator>;
	// held as base here to avoid template resolution issues in the module interface.
	GUI::base::ptr                                                  m_buffer_tree;
	GUI::Elements::label::ptr                                       m_type_lbl;
	GUI::Elements::label::ptr                                       m_dim_lbl;
	GUI::Elements::label::ptr                                       m_name_lbl;

	// What we are previewing. get_desc()/get_resource() come off the view.
	std::shared_ptr<HAL::ResourceView> m_source;

	// Texture produced on the render thread; handed to m_img via run_on_ui.
	std::shared_ptr<HAL::Texture> m_current_tex;

	// Render-thread-only flags (reset by set_source).
	bool m_fit_done      = false;
	bool m_buffer_inited = false;

	// Cached image widget size: UI thread writes, render thread reads.
	volatile int m_view_w = 64;
	volatile int m_view_h = 64;

	// Selected mip / array slice: UI thread writes, render thread reads.
	volatile int m_sel_mip   = 0;
	volatile int m_sel_array = 0;

	GUI::Elements::combo_box::ptr m_mip_combo;
	GUI::Elements::combo_box::ptr m_array_combo;

	// Pan / zoom / camera state — guarded by m_state_mutex.
	mutable Thread::Lockable::mutex m_state_mutex;
	float2              m_pan   = { 0.0f, 0.0f };
	float               m_scale = 1.0f;
	third_person_camera m_cam_3d;

	bool m_dragging = false;
	vec2 m_drag_start;
	vec2 m_prev;

public:
	using ptr = std::shared_ptr<resource_preview>;

	resource_preview();

	// Point the preview at a new resource. Resets fit/zoom and re-populates the
	// mip / array selectors on the next render. title labels the info panel.
	void set_source(std::shared_ptr<HAL::ResourceView> source, const std::string& title = "");

	// Render the current source into the preview image. Must run inside a pass
	// while source is in a shader-readable state.
	void render(FrameGraph::FrameContext* context);

	virtual bool   on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
	virtual bool   on_mouse_move(vec2 pos) override;
	virtual bool   on_wheel(mouse_wheel type, float value, vec2 wpos) override;
	virtual ::sizer update_layout(::sizer r, float scale) override;
};

export class FrameGraphDebug
{
public:
	static GUI::base::ptr  create_debug_layout(FrameGraph::Graph& graph);
	static GUI::base::ptr  create_timeline_layout(FrameGraph::Graph& graph);
};
