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
// Emoji marking the kind of a resource, UTF-8 spelled out as bytes so the source encoding doesn't matter.
// Each is a single code point (palette, ice cube, game die, package, tree).
export namespace resource_icons
{
	inline constexpr const char* texture_2d = "\xF0\x9F\x8E\xA8"; // U+1F3A8
	inline constexpr const char* texture_3d = "\xF0\x9F\xA7\x8A"; // U+1F9CA
	inline constexpr const char* cube       = "\xF0\x9F\x8E\xB2"; // U+1F3B2
	inline constexpr const char* buffer     = "\xF0\x9F\x93\xA6"; // U+1F4E6
	inline constexpr const char* bvh         = "\xF0\x9F\x8C\xB3"; // U+1F333
}

// Small panel that follows the cursor over a preview: a swatch of the picked colour (opaque half / alpha-blended half)
// with text rows added as children by the owner.
export class preview_hud : public GUI::base
{
public:
	using ptr = std::shared_ptr<preview_hud>;

	float4 color = { 0.0f, 0.0f, 0.0f, 1.0f }; // as displayed in the preview (display gamma already applied)
	float  alpha = 1.0f;

	virtual void draw(Context& c) override;
};

// Log-scaled histogram of the previewed texture, one set of bars per enabled channel. Fed from the GPU histogram
// pass (bins in stats.hlsli order: 4 channels x 256), drawn in the preview's Stats tab.
export class histogram_view : public GUI::base
{
public:
	using ptr = std::shared_ptr<histogram_view>;

	std::vector<uint> bins = std::vector<uint>(4 * 256, 0u);
	uint total        = 0;     // samples that went into the bins
	uint channel_mask = 0xF;   // bit 0..3 = R,G,B,A, same as the preview's channel tick boxes

	virtual void draw(Context& c) override;
};

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
	std::string                        m_title;   // kept for refresh_source's full-reset path

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

	// Selector extents (UI thread); the prev/next buttons clamp against them.
	int m_mip_count   = 1;
	int m_array_count = 1;

	// Visible channels, bit 0..3 = R,G,B,A: UI thread writes, render thread reads.
	volatile int m_channel_mask = 0xF;
	GUI::Elements::button::ptr m_prev_mip, m_next_mip, m_prev_array, m_next_array;

	volatile int m_refit = 0; // set by the Reset button, consumed by render()

	// Pixel under the cursor, in preview-image pixels (-1 = cursor is elsewhere): UI thread writes, render thread reads.
	volatile int m_cursor_x = -1;
	volatile int m_cursor_y = -1;

	// Which toolbar tools apply depends on the resource: R/G/B/A need a shader that honours the channel mask
	// (2D, 2D-array, cube); Reset, the pixel readout and the stats need the flat pan/zoom view (2D, 2D-array).
	// Buffers and 3D volumes have neither. The tools that don't apply are hidden.
	volatile bool m_pixel_tools = true;
	bool          m_pick_valid  = false;   // UI thread: the texel under the cursor is inside the texture, not the margin around it
	GUI::Elements::check_box::ptr m_channel_check[4];
	GUI::Elements::label::ptr     m_channel_label[4];
	GUI::Elements::button::ptr    m_reset_btn;

	// A tick box with its letters: how a tool of the second toolbar row is held so it can be hidden.
	struct tick_tool
	{
		GUI::Elements::check_box::ptr box;
		GUI::Elements::label::ptr     text;
		void show(bool v) const { if (box) box->visible = v; if (text) text->visible = v; }
	};

	// Second toolbar row. Exposure and the linear view work for 2D, 2D-array and cube; the alpha checkerboard and
	// the normal-map view only for 2D / 2D-array. UI thread writes, render thread reads.
	GUI::base::ptr                 m_bar;
	GUI::Elements::label::ptr      m_ev_label;
	GUI::Elements::float_slider::ptr m_ev_slider;
	tick_tool                      m_linear_tool, m_checker_tool, m_normal_tool;
	volatile float m_exposure = 0.0f;   // stops (EV)
	volatile int   m_linear   = 0;      // 1 = show linear values, no display gamma
	volatile int   m_checker  = 0;      // 1 = composite over a checkerboard by alpha
	volatile int   m_normal   = 0;      // 1 = normal-map view

	// Info tab (type, size, format, ...) and Stats tab (min / max / average, from the histogram pass).
	GUI::Elements::label::ptr m_format_lbl, m_mips_lbl, m_memory_lbl, m_flags_lbl;
	GUI::Elements::label::ptr m_min_lbl, m_max_lbl, m_avg_lbl;
	histogram_view::ptr       m_histogram;

	void set_tools(bool channels, bool flat_view);

	// The Reset button: re-fit the view and put every display setting back (exposure, linear, checker, normal view, channels).
	void reset_view();

	// Floating readout next to the cursor: texel position, values, hex and a colour swatch.
	preview_hud::ptr          m_hud;
	GUI::Elements::label::ptr m_pixel_lbl, m_value_lbl, m_hex_lbl;

	// Once the user pans or zooms, a resize no longer re-fits the image. m_fit_size is the view size of the last fit.
	bool  m_user_view = false;
	uint2 m_fit_size  = { 0, 0 };

	// Picked texel + histogram bins written by the preview shaders (layout in stats.hlsli). Render thread only.
	std::shared_ptr<HAL::StructuredBufferView<uint>> m_stats;

	// UI thread. Mip and array selectors are shown only when they have more than one entry.
	void set_selectors_visible(bool mips, bool arrays);
	void select_mip(int mip);
	void select_array(int index);

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

	// Re-point at the current frame's incarnation of the same logical resource.
	// FrameGraph placement reassigns ResourceAllocInfo::view every frame
	// (aliasing can move it; resize recreates it) — call this before render()
	// each capture. Same view: no-op. Same desc, new view (moved by aliasing):
	// swaps the source, keeps pan/zoom/UI. Different desc (resize): full
	// set_source reset.
	void refresh_source(std::shared_ptr<HAL::ResourceView> source);

	// Render the current source into the preview image. Must run inside a pass
	// while source is in a shader-readable state.
	void render(FrameGraph::FrameContext* context);

	virtual bool   on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
	virtual bool   on_mouse_move(vec2 pos) override;
	virtual void   on_mouse_leave(vec2 pos) override;
	virtual bool   on_wheel(mouse_wheel type, float value, vec2 wpos) override;
	virtual ::sizer update_layout(::sizer r, float scale) override;
};

export class FrameGraphDebug
{
public:
	// Phone layout: the details dock is hidden until something is selected, then covers the timeline
	// (with a back button), and one finger pans it. Set before create_debug_layout.
	static inline bool compact = false;
	static inline float ui_scale = 1.0f;   // multiplies the timeline's fixed sizes; host it in a layout-scale-1 parent

	static GUI::base::ptr  create_debug_layout(FrameGraph::Graph& graph);
	static GUI::base::ptr  create_timeline_layout(FrameGraph::Graph& graph);
};
