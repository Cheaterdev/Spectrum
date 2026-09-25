module Graphics:DebugView;
import RenderSystem;

import :Context;

using namespace HAL;
using namespace FrameGraph;

namespace
{
	constexpr uint max_debug_meshes = 1024 * 1024;
	constexpr uint max_debug_lines  = 1024 * 1024;

	// Per DebugViewGather bucket: rgb blended over the object colour by a.
	const float4 bucket_tints[] =
	{
		{ 0.0f, 0.0f, 0.0f, 0.0f },   // drawn: object colour
		{ 1.0f, 0.55f, 0.1f, 0.6f },  // rescued by the stage-2 retest
		{ 0.85f, 0.1f, 0.1f, 0.85f }, // culled
		{ 0.2f, 0.8f, 1.0f, 0.6f },   // never rasterized (translucent)
	};
}

debug_view::debug_view() : VariableContext(L"Debug View")
{
	docking   = GUI::dock::FILL;
	thinkable = true;
	clickable = true;

	// Shaders write linear colour into a UNORM target; don't sRGB-decode it.
	texture.mul_color     = { 1, 1, 1, 0 };
	texture.add_color     = { 0, 0, 0, 1 };
	texture.linear_source = true;

	auto& device = RenderSystem::get().device();
	for (int i = 0; i < bucket_count; i++)
	{
		commands[i] = std::make_shared<virtual_gpu_buffer<Table::Meshes::CommandData>>(device, max_debug_meshes, counterType::HELP_BUFFER,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
		commands[i]->buffer.resource->set_name("DebugView_Commands" + std::to_string(i));
	}
	lines = std::make_shared<virtual_gpu_buffer<Table::Dev::DebugLineSegment>>(device, max_debug_lines, counterType::NONE,
		HAL::ResFlags::ShaderResource);
	lines->buffer.resource->set_name("DebugView_Lines");

	capture = std::make_shared<CullCapture>();

	m_render = [this](Passes::Dev::DebugView::Context& data, FrameGraph::FrameContext& context)
	{
		if (!color_target || !scene)
			return;

		auto& list     = *context.get_list();
		auto& graphics = list.get_graphics();
		auto& compute  = list.get_compute();
		auto& copy     = list.get_copy();

		graphics.set_signature(Layouts::DefaultLayout);
		compute.set_signature(Layouts::DefaultLayout);

		UINT mesh_count = draw_scene ? (UINT)scene->command_ids[(int)MESH_TYPE::ALL].size() : 0;

		if (mesh_count)
		{
			PROFILE(L"debug_view_gather");
			for (auto& c : commands)
			{
				c->reserve(list, mesh_count);
				compute.clear_counter(c->buffer);
			}
			capture->prepare(list);

			{
				Slots::Dev::DebugViewGather gather;
				for (int i = 0; i < bucket_count; i++)
					gather.GetCommands()[i] = commands[i]->buffer;
				gather.GetStamps()        = capture->stamps->buffer;
				gather.GetCapture_frame() = frame_capture;
				gather.GetSource()        = frame_source;
				compute.set(gather);
			}
			compute.set(scene->compiledGather[(int)MESH_TYPE::ALL]);
			compute.set(scene->compiledScene);
			compute.set_pipeline<PSOS::Dev::DebugViewGather>();
			compute.dispatch((int)Math::DivideByMultiple(mesh_count, 64), 1, 1);
		}

		if (!frame_lines.empty())
		{
			PROFILE(L"debug_view_line_upload");
			lines->reserve(list, frame_lines.size());
			copy.update(lines->buffer, 0, std::span{ frame_lines });
		}

		RT::Frame::SingleColorDepth rt;
		rt.GetColor() = color_target->texture_2d().renderTarget;
		rt.GetDepth() = depth_target->texture_2d().depthStencil;
		auto targets = rt.compile(list);

		// set_rtv records the target usage only in the operation it opens, and
		// every set_pipeline starts a new one. These textures aren't FrameGraph
		// resources, so the tracker takes the clear as their last use and returns
		// them to their resting layout (COPY_DEST for the depth-only texture,
		// D3D12 #1334 on the next draw). Rebinding after each pipeline switch puts
		// the usage into the draw's own operation.
		auto bind_targets = [&]() { graphics.set_rtv(targets); };

		{
			PROFILE(L"debug_view_clear");
			// Default clear colour: it matches the textures' optimized clear
			// value, any other colour is a slow clear (#820).
			graphics.set_rtv(targets, RTOptions::Default | RTOptions::ClearColor | RTOptions::ClearDepth);
		}

		{
			Slots::Frame::FrameInfo frameInfo;
			frameInfo.GetCamera()     = frame_camera;
			frameInfo.GetPrevCamera() = frame_camera;
			graphics.set(frameInfo);
		}

		if (mesh_count)
		{
			PROFILE(L"debug_view_meshes");
			graphics.set_pipeline<PSOS::Dev::DebugViewMesh>();
			bind_targets();
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
			graphics.set_index_buffer(HAL::Views::IndexBuffer());
			graphics.set(scene->compiledScene);
			{
				Slots::Dev::DebugViewFrustum frustum;
				frustum.GetFrustum() = captured_frustum;
				frustum.GetOutside_brightness() = has_captured_camera ? (float)outside_frustum_brightness : 1.0f;
				graphics.set(frustum);
			}
			for (int i = 0; i < bucket_count; i++)
			{
				Slots::Dev::DebugViewTint tint;
				tint.GetTint() = bucket_tints[i];
				graphics.set(tint);
				graphics.exec_indirect(commands[i]->buffer, mesh_count);
			}
		}

		if (!frame_lines.empty())
		{
			PROFILE(L"debug_view_lines");
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
			UINT vertex_count = (UINT)frame_lines.size() * 6;

			auto draw_lines = [&](float alpha)
			{
				Slots::Dev::DebugLines params;
				params.GetSegments()      = lines->buffer;
				params.GetViewport_size() = float2(target_size);
				params.GetAlpha_scale()   = alpha;
				graphics.set(params);
				graphics.draw(vertex_count);
			};

			if (xray_lines)
			{
				graphics.set_pipeline<PSOS::Dev::DebugLinesOccluded>();
				bind_targets();
				draw_lines(xray_alpha);
			}

			graphics.set_pipeline<PSOS::Dev::DebugLinesDraw>();
			bind_targets();
			draw_lines(1.0f);
		}
	};
}

void debug_view::update_frame(FrameGraph::Graph& graph)
{
	uint64_t drawn = draw_count.load();
	bool on_screen = drawn != seen_draw_count;
	seen_draw_count = drawn;

	graph.get_context<Table::Dev::DebugViewState>().enabled = on_screen;

	// The main view's Scene pass later this frame reads capture->capturing.
	capture->capturing = on_screen && source != Dev::DebugViewSource::All && !freeze;
	if (capture->capturing)
		capture->next_frame();
	frame_source  = source;
	frame_capture = capture->frame;

	if (main_cam && (!freeze || !has_captured_camera))
	{
		captured_inv_view_proj = main_cam->get_inv_view_proj();
		captured_eye           = main_cam->camera_cb.current.position.xyz;
		captured_frustum       = main_cam->camera_cb.current.frustum;
		has_captured_camera    = true;
	}

	if (on_screen && draw_main_frustum)
		submit_main_camera();
	if (on_screen && draw_grid)
		DebugDraw::get().grid(vec3(0, 0, 0), 10.0f, 20, float4(0.4f, 0.45f, 0.5f, 0.5f));

	// Drained even while hidden, or shapes would pile up for the next time the
	// window is shown.
	frame_lines = DebugDraw::get().take();

	if (!on_screen)
		return;

	ivec2 size = ivec2::max(ivec2(get_render_bounds().size), ivec2(64, 64));
	if (!color_target || size != target_size)
	{
		auto& device = RenderSystem::get().device();
		color_target = std::make_shared<HAL::Texture>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { size }, 1, 1,
				HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget));
		depth_target = std::make_shared<HAL::Texture>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R32_TYPELESS, { size }, 1, 1,
				HAL::ResFlags::DepthStencil));
		color_target->resource->set_name("DebugView_Color");
		depth_target->resource->set_name("DebugView_Depth");
		texture.texture = color_target->texture_2d();
		target_size = size;
	}

	if (!cam_placed && main_cam)
	{
		place_behind_main_camera();
		cam_placed = true;
	}

	cam.set_projection_params(Math::pi / 4, float(size.x) / float(size.y), 0.1f, 5000.0f);
	cam.update();
	frame_camera = cam.camera_cb.current;
}

void debug_view::submit_main_camera()
{
	if (!has_captured_camera)
		return;

	auto unproject = [&](vec3 ndc)
	{
		vec4 t = vec4(ndc, 1) * captured_inv_view_proj;
		return vec3(t.xyz) / t.w;
	};

	vec3 eye = captured_eye;

	// The main camera's far plane is ~1500 units away, which dwarfs everything
	// near it; cut the frustum at frustum_length along each corner ray instead.
	vec3 c[8];
	for (int i = 0; i < 4; i++)
	{
		vec2 xy = { (i & 1) ? 1.0f : -1.0f, (i & 2) ? 1.0f : -1.0f };
		vec3 p0 = unproject(vec3(xy, 0));
		vec3 p1 = unproject(vec3(xy, 1));
		// Which NDC depth is the near plane depends on reversed-Z; ask the eye.
		bool p0_near = (p0 - eye).length() < (p1 - eye).length();
		vec3 near_p  = p0_near ? p0 : p1;
		vec3 far_p   = p0_near ? p1 : p0;

		float depth = (far_p - near_p).length();
		float t = depth > 0 ? std::min(1.0f, (float)frustum_length / depth) : 1.0f;

		c[i]     = near_p;
		c[i + 4] = near_p + (far_p - near_p) * t;
	}

	auto& dd = DebugDraw::get();
	float4 color = { 1.0f, 0.8f, 0.2f, 1.0f };
	dd.frustum(c, color, 2.0f);
	dd.sphere(eye, 0.25f, color, 2.0f, 16);
}

void debug_view::place_behind_main_camera()
{
	if (!main_cam)
		return;

	vec3 eye = main_cam->camera_cb.current.position.xyz;
	vec3 dir = main_cam->camera_cb.current.direction.xyz;

	cam.position = eye - dir * 25.0f + vec3(0, 12, 0);
	cam.look_at(eye + dir * 20.0f);
}

void debug_view::draw(GUI::base::Context& c)
{
	draw_count.fetch_add(1);
	image::draw(c);
}

void debug_view::think(float dt)
{
	if (!is_focused())
	{
		cam.move_input = { 0, 0, 0 };
		cam.fast_move  = false;
	}

	cam.frame_move(dt);
}

bool debug_view::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
	if (action == mouse_action::DOWN)
		focus();

	if (button == mouse_button::RIGHT)
	{
		looking   = action == mouse_action::DOWN;
		look_last = pos;
		set_movable(looking);
	}

	return true;
}

bool debug_view::on_mouse_move(vec2 pos)
{
	if (looking)
	{
		cam.add_look(pos - look_last);
		look_last = pos;
	}
	return true;
}

void debug_view::on_key_action(key_action action, long key, key_mods mods)
{
	if (key < 0 || key >= 256)
		return;

	keys[key] = action == key_action::DOWN;
	if (key == 'F' && action == key_action::DOWN)
		place_behind_main_camera();

	update_move_input();
}

bool debug_view::on_wheel(mouse_wheel type, float value, vec2 pos)
{
	cam.move_speed = Math::clamp(cam.move_speed * (1.0f + value * 0.1f), 0.5f, 500.0f);
	return true;
}

void debug_view::update_move_input()
{
	auto k = [this](int vk) { return keys[vk] ? 1.0f : 0.0f; };
	cam.move_input = {
		k('D') - k('A'),
		k('E') - k('Q'),
		k('W') - k('S')
	};
	cam.fast_move = keys[0x10]; // VK_SHIFT
}
