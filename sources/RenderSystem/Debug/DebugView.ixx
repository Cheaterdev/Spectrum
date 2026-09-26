export module Graphics:DebugView;

import :Camera;
import :Scene;
import :DebugDraw;
import :CullCapture;

import FrameGraph;
import HAL;
import GUI;

// A separate window showing the scene from a free-flying debug camera, with
// DebugDraw's shapes on top. Rendered by the DebugView pass (debug_view.prism)
// into this widget's own textures, flat-shaded and at native resolution: no
// lighting, upscaling or post-processing.
//
// Every source but All shows what the main view did with each mesh on the
// captured frame, read from its own draws (CullCapture), not re-culled here.
//
// Controls while focused: RMB-drag look, WASD/QE move, Shift fast, wheel
// changes speed, F moves behind the main camera.
export class debug_view : public GUI::Elements::image, public VariableContext
{
public:
	using ptr = s_ptr<debug_view>;

	Variable<bool>  draw_scene        = { true, "Draw scene", this };
	Variable<Dev::DebugViewSource> source = { Dev::DebugViewSource::Status, "Source", this };
	// Stops recording: the stamps and the main-camera frustum stay at the last
	// captured frame while the main camera moves on.
	Variable<bool>  freeze            = { false, "Freeze capture", this };
	Variable<bool>  color_by_meshlet  = { false, "Colour by meshlet", this };
	Variable<bool>  draw_main_frustum = { true, "Main camera frustum", this };
	Variable<bool>  draw_grid         = { true, "Ground grid", this };
	Variable<float> frustum_length    = { 100.0f, "Frustum length", this, 1.0f, 1500.0f };
	// Darkens geometry outside the main camera's frustum; 1 = off.
	Variable<float> outside_frustum_brightness = { 0.3f, "Outside frustum brightness", this, 0.0f, 1.0f };
	Variable<bool>  xray_lines        = { true, "X-ray lines", this };
	Variable<float> xray_alpha        = { 0.25f, "X-ray alpha", this, 0.0f, 1.0f };

	Scene::ptr scene;
	first_person_camera* main_cam = nullptr;

	// Hand to the main view's mesh_renderer, whose GBuffer draws fill it.
	CullCapture::ptr capture;

	// Setup is generated (debug_view.prism) -- render only.
	Passes::Dev::DebugView::render_func_type m_render;

	debug_view();

	template<typename TPipeline>
	explicit debug_view(TPipeline& pipeline) : debug_view()
	{
		pipeline.debugView.render_func = m_render;
	}

	// Once per frame, after the main camera's update and before graph.setup():
	// enables the pass while the window is on screen, starts this frame's
	// capture, sizes the targets, updates the camera and collects this frame's
	// DebugDraw shapes.
	void update_frame(FrameGraph::Graph& graph);

	void draw(GUI::base::Context& c) override;

protected:
	void think(float dt) override;
	bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
	bool on_mouse_move(vec2 pos) override;
	void on_key_action(key_action action, long key, key_mods mods) override;
	bool on_wheel(mouse_wheel type, float value, vec2 pos) override;

private:
	first_person_camera cam;
	bool cam_placed = false;
	void place_behind_main_camera();
	void submit_main_camera();

	// The main camera as of the last captured frame, so the frustum freezes
	// together with the stamps.
	mat4x4 captured_inv_view_proj;
	vec3 captured_eye;
	Table::Frame::Frustum captured_frustum;
	bool has_captured_camera = false;

	bool keys[256] = {};
	bool looking = false;
	vec2 look_last;
	void update_move_input();

	// draw() runs on the UI render threads; update_frame() compares against the
	// value it saw last frame to tell whether the window is still on screen.
	std::atomic<uint64_t> draw_count = 0;
	uint64_t seen_draw_count = 0;

	ivec2 target_size = { 0, 0 };
	std::shared_ptr<HAL::Texture> color_target;
	std::shared_ptr<HAL::Texture> depth_target;

	// One indirect list per DebugViewGather bucket (debug_view.prism).
	static constexpr int bucket_count = 4;
	HAL::virtual_gpu_buffer<Table::Meshes::CommandData>::ptr commands[bucket_count];
	HAL::virtual_gpu_buffer<Table::Dev::DebugLineSegment>::ptr lines;

	// Captured by update_frame() for this frame's render().
	std::vector<Table::Dev::DebugLineSegment> frame_lines;
	Table::Frame::Camera frame_camera;
	Dev::DebugViewSource frame_source = Dev::DebugViewSource::All;
	uint frame_capture = 0;
};
