import Graphics;
import GUI;
import HAL;


#include "Platform/Window.h"

import ppl;
import Core;
import FrameGraph;
import FrameGraphDebug;

using namespace FrameGraph;

using namespace HAL;

// Declared rather than pulled in via autogen/context_deps.h: that header
// includes <bitset>, and textually including a standard header in a TU that
// also imports HAL/Graphics reintroduces the std double-definition this
// codebase keeps running into. The function has global module attachment (it
// is declared in a global module fragment and defined in
// autogen/context_snapshot.cpp), so a plain declaration here matches it.
namespace FrameGraph { void update_context_dirty_mask(Graph& graph); }



class tick_timer
{
	std::chrono::time_point<std::chrono::system_clock> last_tick;

public:
	tick_timer()
	{
		last_tick = std::chrono::system_clock::now();
	}

	double tick()
	{
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = now - last_tick;
		last_tick = now;
		return elapsed_seconds.count();
	}
};

class count_meter
{
	double time = 0;
	tick_timer t;
	unsigned int ticks = 0;

	double average = 0;

public:
	bool tick()
	{
		time += t.tick();
		ticks++;

		if (time > 1)
		{
			average = ticks / time;
			ticks = 0;
			time = 0;
			return true;
		}

		return false;
	}

	float get()
	{
		return (float)average;
	}
};

class triangle_drawer : public GUI::Elements::image, public GraphGenerator, VariableContext
{
		Pipelines::MainPipeline pipeline;
	main_renderer::ptr scene_renderer;
	main_renderer::ptr gpu_scene_renderer;
	stencil_renderer::ptr stenciler;

	GUI::Elements::label::ptr info;

	size_t time = 0;

	struct EyeData : public Events::prop_handler
	{
		using ptr = std::shared_ptr<EyeData>;
		first_person_camera cam;


		EyeData(HAL::RootSignature::ptr sig)
		{}
	};

public:
	first_person_camera cam;

	using ptr = std::shared_ptr<triangle_drawer>;
	//	PostProcessGraph::ptr render_graph;


	//Variable<bool> enable_gi = { true, "GI", this };
	//Variable<bool> enable_fsr = { true, "FSR", this };
	Variable<bool> downsampled = { true, "downsampled", this };
	//Variable<bool> enable_denoiser = { true, "denoiser", this };

	//Variable<bool> debug_draw = Variable<bool>(false, "debug_draw",this);
	//	VoxelGI::ptr voxel_renderer;

	GUI::Elements::circle_selector::ptr sun_direction_circle;
	Variable<bool> auto_rotate_sun = { false, "Auto rotate sun", this };

	// Grouping contexts for the mesh_renderer instances constructed below --
	// mesh_renderer always names itself "mesh_renderer" (see MeshRenderer.cpp),
	// so without these every instance across the whole app would land as an
	// identically-named sibling directly under global, with no way to tell
	// them apart in the Properties tree. Declared as members (not locals in
	// the constructor body) so they stay alive for the whole app, same as the
	// mesh_renderer instances parented under them.
	VariableContext main_view_forward_context{ L"Main View / Forward" };
	VariableContext main_view_gpu_context{ L"Main View / GPU" };
	float sun_rotation_angle = 0;
	static constexpr float sun_rotation_speed = 0.2f; // radians/sec

	int visible_count;

	mesh_renderer::ptr meshes_renderer;

	Scene::ptr scene;
	float draw_time;
	MeshAssetInstance::ptr instance;

	VSM vsm;
	SkyRender sky;

	BlueNoise blue_noise;
	VoxelGI::ptr voxel_gi;
		SMAA smaa;
		 ~triangle_drawer()
		 {
			 RenderSystem::get().device().get_queue(CommandListType::DIRECT)->signal_and_wait();
			 RenderSystem::get().device().get_queue(CommandListType::COMPUTE)->signal_and_wait();
	 RenderSystem::get().device().get_queue(CommandListType::COPY)->signal_and_wait();

		 }
		triangle_drawer() : VariableContext(L"triangle_drawer"), pipeline(), blue_noise(pipeline), smaa(pipeline), sky(pipeline), vsm(pipeline)
	{
		// Pushed for the rest of this constructor: any VariableContext-derived
		// member constructed from here on (stenciler, voxel_gi, mesh_renderer
		// via register_renderer, ...) nests under "triangle_drawer" instead of
		// landing as a flat sibling of it under global, with no per-member
		// wiring needed at each construction site. Doesn't cover vsm/
		// main_view_*_context below -- those are member-initializer-list
		// entries (or plain default members) that already finished
		// constructing before this body even starts, so they need the
		// explicit add_child re-parent instead.
		VariableContext::Scope self_scope(*this);

		// vsm/main_view_*_context construct before this body runs (regular data
		// members), so by the ctor-body-start rule they land under whatever was
		// on the Scope stack at that point -- nothing, here, so they'd otherwise
		// sit as flat top-level siblings of "triangle_drawer" instead of nested
		// under it. VariableContext::add_child re-parents them explicitly (it
		// already detaches from the current parent first -- see tree::add_child
		// -- so this is safe even though vsm etc. are already attached to
		// global); qualified because triangle_drawer also inherits GUI::base's
		// unrelated add_child(GUI::base::ptr), which unqualified lookup prefers.
		VariableContext::add_child(&main_view_forward_context);
		VariableContext::add_child(&main_view_gpu_context);
		VariableContext::add_child(&vsm);

		texture.mul_color = { 1, 1, 1, 0 };
		texture.add_color = { 0, 0, 0, 1 };
		// SMAA used to bake this in (pow(x, 1/2.2)) when it ran; now applied
		// here so the viewport looks the same whether SMAA, FSR, or DLSS
		// produced ResultTextureNew.
		texture.gamma = true;


		auto t = CounterManager::get().start_count<triangle_drawer>();
		thinkable = true;
		clickable = true;

		scene.reset(new Scene());
		scene->name = L"Scene";

		scene_renderer = std::make_shared<main_renderer>();
		{
			// Distinguishes this mesh_renderer's Properties-tree entry from the
			// otherwise-identically-named ones below and in AssetRenderer.cpp.
			VariableContext::Scope scope(main_view_forward_context);
			scene_renderer->register_renderer(meshes_renderer = std::make_shared<mesh_renderer>());
		}

		gpu_scene_renderer = std::make_shared<main_renderer>();

		{
			VariableContext::Scope scope(main_view_gpu_context);
			gpu_scene_renderer->register_renderer(std::make_shared<mesh_renderer>());
		}


		//gpu_scene_renderer->register_renderer(gpu_meshes_renderer_static = std::make_shared<gpu_cached_renderer>(scene, MESH_TYPE::STATIC));
		//gpu_scene_renderer->register_renderer(gpu_meshes_renderer_dynamic = std::make_shared<gpu_cached_renderer>(scene, MESH_TYPE::DYNAMIC));
		//cam.position = vec3(0, 5, -30);

		cam.position = vec3(0, 0, 0);

		stenciler.reset(new stencil_renderer(pipeline));
		stenciler->player_cam = &cam;
		stenciler->scene = scene;
		base::add_child(stenciler);

		// VSM is a class member (wired at construction, before scene exists
		// above) -- its Phase 2 invalidation tracker registers scene event
		// handlers here instead, once the scene is actually available.
		vsm.attach_scene(scene);


		info.reset(new GUI::Elements::label);
		info->docking = GUI::dock::TOP;
		info->x_type = GUI::pos_x_type::LEFT;
		info->magnet_text = FW1_LEFT;
		base::add_child(info);
		base::ptr props(new base);
		props->docking = GUI::dock::FILL;
		props->x_type = GUI::pos_x_type::LEFT;
		props->y_type = GUI::pos_y_type::TOP;
		props->width_size = GUI::size_type::MATCH_CHILDREN;
		props->height_size = GUI::size_type::MATCH_CHILDREN;
		base::add_child(props);



		sun_direction_circle.reset(new GUI::Elements::circle_selector);
		sun_direction_circle->docking = GUI::dock::FILL;
		sun_direction_circle->x_type = GUI::pos_x_type::RIGHT;
		sun_direction_circle->y_type = GUI::pos_y_type::TOP;


		base::add_child(sun_direction_circle);
		//	lighting = std::make_shared<LightingNode>();

		sun_direction_circle->on_change.register_handler(this, [this](const float2& value)
			{
				float2 v = value;
				float3 dir = { 0.001 + v.x, sqrt(1.001 - v.length_squared()), -v.y };
				vsm.set_position(dir);
			});

		sun_direction_circle->set_value({ 1, 0 });

		// auto_rotate_sun and VSM's toggles are Variable<bool>s now -- they show
		// up automatically in the Properties debug panel (Properties -> triangle_drawer
		// / VSM) instead of needing a hand-wired check_box_text row here per toggle.

		MeshAsset::ptr asset_ptr = EngineAssets::material_tester.get_asset();


		auto make_material = [](float3 color, float roughness, float metallic)
			{
				MaterialGraph::ptr graph(new MaterialGraph);


				{
					auto value_node = std::make_shared<VectorNode>(vec4(color, 1));
					graph->register_node(value_node);
					value_node->get_output(0)->link(graph->get_base_color());
				}


				{
					auto value_node = std::make_shared<ScalarNode>(roughness);
					graph->register_node(value_node);
					value_node->get_output(0)->link(graph->get_roughness());
				}

				{
					auto value_node = std::make_shared<ScalarNode>(metallic);
					graph->register_node(value_node);
					value_node->get_output(0)->link(graph->get_mettalic());
				}


				auto m = (new materials::universal_material(graph));
				//m->generate_material();
				m->register_new();
				return m->get_ptr<MaterialAsset>();
			};


		//	auto base_mat = make_material({ 1,1,1 }, 1, 0);

		int count = -1;
		float distance = 5;
		for (int i = 0; i <= count; i++)
			for (int j = 0; j <= count; j++)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(asset_ptr));
				//	instance->override_material(0, base_mat);
				//	instance->override_material(1, make_material({ 1,0,0 }, float(i) / count, float(j) / count));

				instance->local_transform[3] = {
					i * distance - count * distance / 2, 0, j * distance - count * distance / 2, 1
				};

				scene->add_child(instance);
			}

		{
			MeshAsset::ptr ruins_ptr = EngineAssets::plane.get_asset();


			if (ruins_ptr)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(ruins_ptr));
				//	instance->local_transform = mat4x4::translation({ 0,3.8,0 });
				scene->add_child(instance);
			}
		}


		{
			MeshAsset::ptr ruins_ptr = nullptr; // AssetManager::get().find_by_name<MeshAsset>(L"cubes.obj");


			if (ruins_ptr)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(ruins_ptr));
				//	instance->local_transform=mat4x4::translation({ 0,3.8,0 });
				scene->add_child(instance);
			}
		}


		voxel_gi = std::make_shared<VoxelGI>(pipeline,scene,vsm);
	}

	float scale_speed = 0;
	vec2 wheel_pos;

	bool on_wheel(mouse_wheel type, float value, vec2 pos) override
	{
		scale_speed += value * 0.1f;
		wheel_pos = pos;
		return true;
	}

	void think(float dt) override
	{
		//	if (!g_buffer.size.has_value())
		//	return;

		scene->update_transforms();

		if (auto_rotate_sun)
		{
			sun_rotation_angle += dt * sun_rotation_speed;
			// set_value() fires the same on_change handler a manual drag
			// would -- keeps the widget's own visual position in sync too.
			sun_direction_circle->set_value(float2(cos(sun_rotation_angle), sin(sun_rotation_angle)) * 0.99f);
		}

		auto min = scene->get_min();
		auto max = scene->get_max();


		auto points = cam.get_points(min, max);
		auto bounds = points.get_bounds_in(cam.get_view());

		//cam.set_projection_params(std::max(1.0f, bounds.znear - 1), bounds.zfar + 1);
		cam.set_projection_params(1, 1500);

		// Camera only responds to keyboard while the viewport is focused; drop any
		// held movement otherwise so it can't drift after focus is lost.
		if (!stenciler->is_focused())
		{
			cam.move_input = { 0, 0, 0 };
			cam.fast_move  = false;
		}

		cam.frame_move(dt);
	}

	Graph* last_graph = nullptr;
	tick_timer my_timer;
	ResourceAllocInfo* debug_tex_handle = nullptr;

	void generate(Graph& graph) override
	{
		PROFILE(L"triangle_drawer");
		last_graph = &graph;

		ivec2 size = ivec2::max(ivec2(get_render_bounds().size), ivec2(64, 64));
		struct pass_data
		{
			ResourceHandler* o_texture;
			ResourceHandler* sky_cubemap;
			GBufferViewDesc gbuffer;
		};

		scene->update(*graph.builder.current_frame);


		auto& timeinfo = graph.get_context<TimeInfo>();
		auto& skyinfo = graph.get_context<SkyInfo>();
		auto& caminfo = graph.get_context<CameraInfo>();
		auto& sceneinfo = graph.get_context<SceneInfo>();
		auto& vp = graph.get_context<ViewportInfo>();


		g_upscaling_enabled = downsampled;

		// Mirrors g_upscaler_type/g_upscaling_enabled into the SIG context --
		// see UpscalingDLSS.sig's own comment on UpscalerSelectors for why.
		{
			auto& upscaler_ctx = graph.get_context<Table::UpscalerSelectors>();
			upscaler_ctx.upscaler_type     = g_upscaler_type;
			upscaler_ctx.upscaling_enabled = g_upscaling_enabled;
		}
		// Mirrors fixed hardware/SDK capabilities into the SIG context -- see
		// raytracing.sig's own comment on RenderDeviceCapabilities for why.
		{
			auto& device_caps = graph.get_context<Table::RenderDeviceCapabilities>();
			device_caps.rtx_supported    = RenderSystem::get().device().is_rtx_supported();
			device_caps.dlss_available   = nvidia::DLSS::get().available();
			device_caps.dlssrr_available = nvidia::DLSSRR::get().available();
		}

		if (downsampled)
		{
			// Use DLSS's own recommended render resolution for the current
			// mode when available; get_optimal_settings can return eOk with
			// render_size (0,0) for an unsupported mode/size, so validate
			// before using it. Falls back to /1.5 (the FSR path).
			nvidia::DLSSOptimalSettings settings{};
			if (nvidia::DLSS::get().available() &&
			    nvidia::DLSS::get().get_optimal_settings(g_upscaling_dlss_mode, size, /*hdr=*/true, settings) &&
			    settings.render_size.x > 0 && settings.render_size.y > 0)
			{
				if (g_upscaling_dlss_scale_override >= 0.0f)
				{
					float t = g_upscaling_dlss_scale_override / 100.0f;
					vp.frame_size.x = int(settings.render_size_min.x + (settings.render_size_max.x - settings.render_size_min.x) * t);
					vp.frame_size.y = int(settings.render_size_min.y + (settings.render_size_max.y - settings.render_size_min.y) * t);
				}
				else
					vp.frame_size = settings.render_size;
			}
			else
				vp.frame_size = size / 1.5;
		}
		else
			vp.frame_size = size;

		vp.upscale_size = size;

		// Edge-triggered: logs only when the actual computed size driving
		// GBuffer_HiZ/GBuffer_HiZ_UAV's size/8 dimensions changes, whatever
		// the cause (window resize, DLSS mode switch, scale override drag) --
		// for correlating against a suspected resize/DLSS-triggered Hi-Z
		// content bug. Remove once that's tracked down.
		static ivec2 last_logged_frame_size = { -1, -1 };
		static ivec2 last_logged_upscale_size = { -1, -1 };
		if (vp.frame_size != last_logged_frame_size || vp.upscale_size != last_logged_upscale_size)
		{
			Log::get() << "[Viewport] frame_size " << last_logged_frame_size.x << "x" << last_logged_frame_size.y
				<< " -> " << vp.frame_size.x << "x" << vp.frame_size.y
				<< ", upscale_size " << last_logged_upscale_size.x << "x" << last_logged_upscale_size.y
				<< " -> " << vp.upscale_size.x << "x" << vp.upscale_size.y
				<< ", dlss_mode=" << (int)g_upscaling_dlss_mode
				<< ", scale_override=" << g_upscaling_dlss_scale_override << Log::endl;
			last_logged_frame_size = vp.frame_size;
			last_logged_upscale_size = vp.upscale_size;
		}

		sceneinfo.scene = scene;
		sceneinfo.renderer = gpu_scene_renderer;
		caminfo.cam = &cam;
		timeinfo.time = (float)my_timer.tick();
		timeinfo.totalTime += timeinfo.time;
		skyinfo.sunDir = vsm.get_position();

		// Exactly once per frame — a second call anywhere else would desync
		// SL's internal frame counter.
		if (nvidia::Streamline::get().available())
			nvidia::Streamline::get().begin_frame();

		// Jitter only matters when something accumulates it temporally (DLSS)
		// AND DLSS is actually the pass running this frame — g_upscaling_enabled
		// off (downsampled toggled off) or a non-DLSS selection both mean
		// nothing consumes the jitter, so applying it would just add visible
		// instability for no benefit. FSR1/native have no such accumulation
		// either way. No availability re-check: g_upscaler_type can't hold an
		// unavailable type (see its invariant, UpscalingDLSS.ixx).
		vec2 jitter_px(0, 0);
		if (g_upscaling_enabled && g_upscaler_type != UpscalerType::FSR)
		{
			// Halton(2,3); phase count per NVIDIA's guidance: 8*(display/render)^2.
			const float scale_x = float(vp.upscale_size.x) / float(vp.frame_size.x);
			const float scale_y = float(vp.upscale_size.y) / float(vp.frame_size.y);
			const uint32_t phase_count = std::max<uint32_t>(1,
				static_cast<uint32_t>(8.0f * scale_x * scale_y + 0.5f));
			const uint32_t phase = static_cast<uint32_t>(graph.builder.current_frame->get_frame() % phase_count) + 1;

			auto halton = [](uint32_t index, uint32_t base) -> float
				{
					float f = 1.0f, r = 0.0f;
					while (index > 0)
					{
						f /= static_cast<float>(base);
						r += f * (index % base);
						index /= base;
					}
					return r;
				};

			// [-0.5, 0.5] pixels, screen convention: +x = right, +y = down.
			jitter_px = { halton(phase, 2) - 0.5f, halton(phase, 3) - 0.5f };
		}

		// NDC is [-1,1]; Y negated since NDC +y is up, jitter_px +y is down.
		const vec2 jitter_ndc(
			jitter_px.x / float(vp.frame_size.x),
			-jitter_px.y / float(vp.frame_size.y));
		cam.update(jitter_ndc);


	
		// Per-frame mirrors into the Table:: contexts the generated setups read,
		// plus the CPU work that used to live in those setups' bodies. All of it
		// needs an owning instance, which a generated static setup cannot reach.
		// Must run before graph.setup(), and before run_pre_setups() for the
		// same reason: nothing orders one pass's setup relative to another's.
		vsm.update_frame(graph);
		voxel_gi->update_frame(graph);
		stenciler->update_frame(graph);

		{
			PROFILE(L"graph");
			pipeline.add_passes(graph);
			// This pipeline's own [PreSetup] hooks: PreScene's
			// raytrace_scene->new_frame(), NRD_REBLUR_Execute's ensure_pools(),
			// CubeSky's sun-direction diff.
			pipeline.run_pre_setups(graph);
		}

	   	voxel_gi->pass_data(graph.builder);

		vsm.pass_data(graph.builder);

		// Single-threaded allocation-planning pass for all VSM clip levels,
		// run once per frame here (via add_slot_generator's pre_run hook)
		// strictly before VSM_RenderPages' single render() is dispatched
		// to the thread pool -- see the LevelPlan comment in VSM.ixx for why
		// this can't safely happen inside render() itself.
		graph.add_slot_generator([this](Graph& graph)
			{
				vsm.plan_frame(graph);
			});

		graph.add_slot_generator([this](Graph& graph)
			{
				PROFILE(L"FrameInfo");
				auto& time = graph.get_context<TimeInfo>();
				auto& skyinfo = graph.get_context<SkyInfo>();
				auto& cam = graph.get_context<CameraInfo>();


				Slots::FrameInfo frameInfo;
				//// hack zone
				auto sky = graph.builder.get(FrameGraph::ResourceID::sky_cubemap_filtered);
				if (sky && sky->resource)
					frameInfo.GetSky() = *sky->get_handler<Handlers::TextureCube>();

				/////////
				frameInfo.GetSunDir().xyz = skyinfo.sunDir;
				frameInfo.GetTime() = { time.time, time.totalTime, 0, 0 };


				frameInfo.GetCamera() = cam.cam->camera_cb.current;
				frameInfo.GetPrevCamera() = cam.cam->camera_cb.prev;

				frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d();
				frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d();

				// Material texture LOD bias — see FrameData.sig's mipBias comment.
				{
					auto& vp = graph.get_context<ViewportInfo>();
					frameInfo.GetMipBias() = downsampled
						? (std::log2(float(vp.frame_size.x) / float(vp.upscale_size.x)) - 1.0f)
						: 0.0f;
				}

				// Hi-Z pyramid for per-meshlet occlusion; the PSO permutation
				// decides whether it is sampled (scene.sig's HiZOcclusion).
				{
					auto hiz = graph.builder.get(FrameGraph::ResourceID::GBuffer_HiZ_UAV);
					if (hiz && hiz->resource)
						frameInfo.GetMainHiZ() = hiz->get_handler<Handlers::Texture>()->texture2D;
				}

				auto compiled = frameInfo.compile(*graph.builder.current_frame);
				graph.register_slot_setter(compiled);
			});

		graph.add_slot_generator([this](Graph& graph)
			{
				graph.register_slot_setter(scene->compiledScene);
			});
	}

	void draw(base::Context& t) override
	{
		auto saved_srv = texture.texture.texture2D;
		if (t.result_texture_srv) texture.texture.texture2D = t.result_texture_srv;
		image::draw(t);
		texture.texture.texture2D = saved_srv;
	}


	void on_bounds_changed(const rect& r) override
	{
		base::on_bounds_changed(r);
		if (r.w <= 64 || r.h <= 64) return;
		ivec2 size = r.size;
		cam.set_projection_params(Math::pi / 4, float(r.w) / r.h, 1, 1500);
	}
};

// Small per-node live-value thumbnail shown on material graph nodes (see
// MaterialGraph::create_node_preview_hook). Node itself isn't owned by this
// widget -- it's only valid while the graph editor holding it is open, same
// lifetime assumption as the node's other editor-window widgets.
class node_preview_thumbnail : public GUI::Elements::image
{
	std::shared_ptr<Asset> m_asset;
	FlowGraph::Node*       m_node;
public:
	node_preview_thumbnail(std::shared_ptr<Asset> a, FlowGraph::Node* node) : m_asset(a), m_node(node)
	{
		docking     = GUI::dock::TOP;
		width_size  = GUI::size_type::FIXED;
		height_size = GUI::size_type::FIXED;
		size        = { 128, 128 };
		thinkable   = true; // re-fetch each frame: the slice view is rebuilt whenever the graph regenerates
	}

	void think(float dt) override
	{
		auto mat = m_asset ? m_asset->get_ptr<materials::universal_material>() : nullptr;
		if (!mat) return;

		auto* session = materials::MaterialPreviewSession::find(mat->get_graph().get());
		if (!session) return;

		texture = session->get_slice_view(mat->get_preview_slot(m_node));

		// The shared GUI sampler (anisoBordeSampler) addresses U/V as WRAP,
		// not clamp/border despite the name -- and this texture gets
		// magnified ~2x on screen, so sampling right at the true edge
		// (tc=0 or 1) blends in texels wrapped from the opposite side of
		// the resource. Inset the UV range by half a texel so the GPU
		// never samples that close to the edge. Fixed here, per-widget,
		// rather than in the shared ninepatch shader (tried that -- it
		// affected every other GUI element too and made things worse).
		constexpr float half_texel = 0.5f / materials::MaterialPreviewSession::preview_resolution;
		texture.tc = { half_texel, half_texel, 1.0f - half_texel, 1.0f - half_texel };
	}
};

// Dock tab embedded in a material graph's editor dock (see materials::
// open_material_editor / create_settings_panel_hook wiring below): the
// final compiled pixel shader text, plus toggles for the two editor-only
// live previews (per-node 2D thumbnails and the whole-graph 3D scene
// render) -- both re-render every frame while on, so switching them off is
// a real perf option on a big/slow graph, not just declutter.
class material_settings_panel : public GUI::base
{
	materials::universal_material*   m_material;
	::FlowGraph::graph*              m_graph;
	uint32_t                         m_shown_generation = ~0u;
	GUI::Elements::MultiLineLabel::ptr m_text;

	// Live 3D preview, docked at the very top of the panel -- same
	// SceneTextureRenderer/test-mesh mechanism asset_preview_content uses
	// for the Asset Explorer's material preview, just embedded here too so
	// it's visible while editing without switching tabs.
	std::shared_ptr<SceneTextureRenderer> m_mesh_renderer;
	std::shared_ptr<HAL::Texture>         m_mesh_target;
	GUI::Elements::image::ptr             m_mesh_img;
	bool m_mesh_dragging = false;
	vec2 m_mesh_prev;

public:
	material_settings_panel(materials::universal_material* material, ::FlowGraph::graph* graph)
		: m_material(material), m_graph(graph)
	{
		// Lives inside a dock tab page (see canvas::on_open below) -- the
		// tab control handles sizing/placement, so just fill it.
		docking     = GUI::dock::FILL;
		thinkable   = true;

		m_mesh_renderer = std::make_shared<SceneTextureRenderer>();

		m_mesh_img           = std::make_shared<GUI::Elements::image>();
		m_mesh_img->docking   = GUI::dock::TOP;
		m_mesh_img->width_size  = GUI::size_type::MATCH_PARENT;
		m_mesh_img->height_size = GUI::size_type::FIXED;
		m_mesh_img->size     = { 0.0f, 240.0f };
		add_child(m_mesh_img);

		// Node preview mode -- 2D (flat per-node dispatch, original
		// behavior) or 3D (same graph evaluated over an analytic sphere
		// instead of a flat quad, with the captured value lit -- see
		// MaterialPreviewSession::rebuild_pso / UniversalMaterialPreview.hlsl
		// PREVIEW_3D). The session itself stays open the whole time the
		// graph editor is open; this only switches how it renders.
		{
			auto row = std::make_shared<GUI::base>();
			row->docking     = GUI::dock::TOP;
			row->height_size = GUI::size_type::FIXED;
			row->size        = { 0.0f, 26.0f };

			auto lbl         = std::make_shared<GUI::Elements::label>();
			lbl->text        = "Node preview:";
			lbl->docking     = GUI::dock::NONE;
			lbl->width_size  = GUI::size_type::FIXED;
			lbl->height_size = GUI::size_type::FIXED;
			lbl->size        = { 90.0f, 20.0f };
			lbl->pos         = { 4.0f, 4.0f };
			row->add_child(lbl);

			auto combo         = std::make_shared<GUI::Elements::combo_box>();
			combo->docking     = GUI::dock::NONE;
			combo->width_size  = GUI::size_type::FIXED;
			combo->height_size = GUI::size_type::FIXED;
			combo->size        = { 80.0f, 20.0f };
			combo->pos         = { 96.0f, 3.0f };

			bool is_3d = false;
			if (auto* session = materials::MaterialPreviewSession::find(m_graph))
				is_3d = session->is_3d();
			combo->get_label()->text = is_3d ? "3D" : "2D";

			combo->add_item("2D")->on_select = [this]()
			{
				if (auto* session = materials::MaterialPreviewSession::find(m_graph))
					session->set_3d(false);
			};
			combo->add_item("3D")->on_select = [this]()
			{
				if (auto* session = materials::MaterialPreviewSession::find(m_graph))
					session->set_3d(true);
			};
			row->add_child(combo);

			add_child(row);
		}

		// Own child, not inherited -- MultiLineLabel::on_text_changed() does
		// contents->remove_all() on itself whenever .text is set, which
		// would also wipe the checkboxes above if they were its children
		// too.
		m_text = std::make_shared<GUI::Elements::MultiLineLabel>();
		m_text->docking = GUI::dock::FILL;
		add_child(m_text);
	}

	bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
	{
		if (button == mouse_button::LEFT || button == mouse_button::RIGHT)
		{
			m_mesh_dragging = (action == mouse_action::DOWN);
			m_mesh_prev     = pos;
			set_movable(m_mesh_dragging);
			return true;
		}
		return false;
	}

	bool on_mouse_move(vec2 pos) override
	{
		if (m_mesh_dragging)
		{
			m_mesh_renderer->orbit((pos - m_mesh_prev) * 0.01f);
			m_mesh_prev = pos;
			return true;
		}
		return false;
	}

	bool on_wheel(mouse_wheel type, float value, vec2 pos) override
	{
		m_mesh_renderer->zoom(value);
		return true;
	}

	void think(float dt) override
	{
		// Same toggle as the "Show 3D scene preview" checkbox below -- keep
		// this panel's own preview off, too, when the user's turned it off
		// for perf.
		if (m_material->show_scene_preview)
		{
			// Size the render target to the preview widget; recreate it when
			// the size changes so it follows resizes instead of stretching.
			ivec2 want = ivec2::max(ivec2(m_mesh_img->get_render_bounds().size), ivec2(64, 64));
			if (!m_mesh_target || ivec2(m_mesh_target->get_size().xy) != want)
			{
				m_mesh_target = std::make_shared<HAL::Texture>(RenderSystem::get().device(),
					HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { want }, 1, 6,
						HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess));
				m_mesh_img->texture.texture = m_mesh_target->texture_2d();
			}

			m_mesh_renderer->draw(m_material->get_ptr<MaterialAsset>(), m_mesh_target);
		}

		// Only re-fetch the shader text when it's actually changed -- it's
		// the same structural-regen signal MaterialPreviewSession uses to
		// know when to recompile.
		if (m_shown_generation == m_material->preview_source_generation)
			return;

		m_shown_generation = m_material->preview_source_generation;
		auto src = m_material->get_pixel_shader_source();
		m_text->text = src.uniforms + src.text;
	}
};

// This frame's asset previews that want a GPU pass, one per claimed
// Passes::AssetPreview instance slot (ui.sig). asset_preview_content::generate()
// appends during create_graph; setup_graph drains the list into the pipeline's
// render_funcs just before add_passes, which then registers exactly the filled
// slots.
//
// A graph context rather than a global: preview widgets only ever drive the UI
// graph, and a context is already per-Graph. Not on GUI::UIContext (where
// pre_draw_infos lives) because resource_preview is a FrameGraphDebug type,
// which is above module GUI.
struct AssetPreviewContext
{
	std::vector<Passes::AssetPreview::render_func_type> renders;
};

// Window content that previews an asset. Dispatches by type:
//   TextureAsset -> universal resource_preview (GPU), self-driven via a pass.
//   BinaryAsset  -> MultiLineLabel (one label per line) in a scroll container.
//   MeshAsset    -> a live render in AssetRenderer's OWN framegraph (a second
//                   main pipeline can't be injected into the shared UI graph),
//                   re-rendered every frame from think() into a target texture.
class asset_preview_content : public GUI::base, public FrameGraph::GraphGenerator
{
	resource_preview::ptr              m_preview; // textures only
	std::shared_ptr<TextureAsset>      m_asset;   // keeps the texture alive
	std::shared_ptr<HAL::ResourceView> m_view;

	std::shared_ptr<MeshAssetInstance>  m_mesh_instance;
	std::shared_ptr<HAL::Texture>       m_mesh_target;
	GUI::Elements::image::ptr           m_mesh_img;
	std::shared_ptr<SceneTextureRenderer> m_mesh_renderer; // own graph, per preview
	std::shared_ptr<MaterialAsset>        m_material;      // material preview
	bool m_mesh_dragging = false;
	vec2 m_mesh_prev;
public:
	using ptr = std::shared_ptr<asset_preview_content>;

	asset_preview_content(std::shared_ptr<Asset> asset)
	{
		docking     = GUI::dock::FILL;
		width_size  = GUI::size_type::MATCH_PARENT;
		height_size = GUI::size_type::MATCH_PARENT;
		clickable   = true; // mesh orbit; texture/binary children handle their own input first

		if (auto tex = asset ? asset->get_ptr<TextureAsset>() : nullptr)
		{
			m_asset   = tex;
			m_preview = std::make_shared<resource_preview>();
			add_child(m_preview);
			if (auto t = tex->get_texture())
			{
				m_view = std::make_shared<HAL::Texture2DView>(t->texture_2d());
				m_preview->set_source(m_view, "asset");
			}
		}
		else if (auto bin = asset ? asset->get_ptr<BinaryAsset>() : nullptr)
		{
			auto text     = std::make_shared<GUI::Elements::MultiLineLabel>();
			text->docking = GUI::dock::FILL;
			text->text    = bin->get_data();
			add_child(text);
		}
		else if (auto mesh = asset ? asset->get_ptr<MeshAsset>() : nullptr)
		{
			m_mesh_instance = mesh->create_instance();
			m_mesh_renderer = std::make_shared<SceneTextureRenderer>();

			m_mesh_img          = std::make_shared<GUI::Elements::image>();
			m_mesh_img->docking = GUI::dock::FILL;
			add_child(m_mesh_img);

			thinkable = true; // re-render each frame via the renderer's own graph
		}
		else if (auto material = asset ? asset->get_ptr<MaterialAsset>() : nullptr)
		{
			m_material      = material;
			m_mesh_renderer = std::make_shared<SceneTextureRenderer>();

			m_mesh_img          = std::make_shared<GUI::Elements::image>();
			m_mesh_img->docking = GUI::dock::FILL;
			add_child(m_mesh_img);

			thinkable = true; // material rendered on a test mesh, same as meshes
		}
	}

	bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
	{
		if (!m_mesh_renderer) return false;
		if (button == mouse_button::LEFT || button == mouse_button::RIGHT)
		{
			m_mesh_dragging = (action == mouse_action::DOWN);
			m_mesh_prev     = pos;
			set_movable(m_mesh_dragging);
			return true;
		}
		return false;
	}

	bool on_mouse_move(vec2 pos) override
	{
		if (m_mesh_renderer && m_mesh_dragging)
		{
			m_mesh_renderer->orbit((pos - m_mesh_prev) * 0.01f);
			m_mesh_prev = pos;
			return true;
		}
		return false;
	}

	bool on_wheel(mouse_wheel type, float value, vec2 pos) override
	{
		if (!m_mesh_renderer) return false;
		m_mesh_renderer->zoom(value);
		return true;
	}

	void think(float dt) override
	{
		if (!m_mesh_renderer || (!m_mesh_instance && !m_material)) return;

		if (m_material)
		{
			auto mat = m_material->get_ptr<materials::universal_material>();
			if (mat && !mat->show_scene_preview) return;

			// Share this preview's camera so MaterialPreviewSession's
			// per-node 3D thumbnails (main.cpp's node_preview_thumbnail)
			// match whatever angle the user last orbited/zoomed here,
			// instead of their own fixed 3/4 view.
			if (mat)
			{
				mat->preview_orbit = m_mesh_renderer->get_orbit();
				mat->preview_zoom  = m_mesh_renderer->get_zoom();
			}
		}

		// Size the render target to the widget; recreate it when the size changes
		// so the mesh follows window resizes instead of being stretched.
		ivec2 want = ivec2::max(ivec2(m_mesh_img->get_render_bounds().size), ivec2(64, 64));
		if (!m_mesh_target || ivec2(m_mesh_target->get_size().xy) != want)
		{
			m_mesh_target = std::make_shared<HAL::Texture>(RenderSystem::get().device(),
				HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { want }, 1, 6,
					HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess));
			m_mesh_img->texture.texture = m_mesh_target->texture_2d();
		}

		if (m_mesh_instance) m_mesh_renderer->draw(m_mesh_instance, m_mesh_target);
		else if (m_material) m_mesh_renderer->draw(m_material, m_mesh_target);
	}

	void generate(FrameGraph::Graph& graph) override
	{
		if (!m_view) return; // only the texture preview needs a GPU pass

		// The pass itself is declared in ui.sig (AssetPreview, [Multiple=16],
		// [Required] because it writes nothing graph-tracked) and its setup is
		// generated — claiming a slot here is the whole registration.
		graph.get_context<AssetPreviewContext>().renders.push_back(
			[this](Passes::AssetPreview::Context&, FrameGraph::FrameContext& ctx) { m_preview->render(&ctx); });
	}
};

UINT64 frame_counter = 0;



class FrameFlowGraph : public ::FlowGraph::graph
{};

class PassNode : public::FlowGraph::Node, public GUI::Elements::FlowGraph::VisualGraph
{
	Graph& graph;
	std::wstring_view pass_name;

public:
	PassNode(Graph& graph, std::wstring_view pass_name) : graph(graph), pass_name(pass_name)
	{}

	void operator()(::FlowGraph::GraphContext*) override
	{}

	GUI::base::ptr create_editor_window() override
	{
		GUI::Elements::image::ptr img(new GUI::Elements::image);
		img->texture = Skin::get().Shadow;
		img->padding = img->texture.padding;
		img->size = { 256, 256 };
		img->width_size = GUI::size_type::FIXED;
		img->height_size = GUI::size_type::FIXED;
		return img;
	}
};
namespace GUI
{


}


// Floating panel (shadow + dark background), styled like the FlowGraph canvas
// toolbar. Used to overlay the viewport with the debug-view selector.
class debug_toolbar_panel : public GUI::base
{
public:
	void draw(GUI::base::Context& c) override
	{
		GUI::Texture shadow = Skin::get().Shadow;
		shadow.margins = { 12, 12, 12, 12 };
		c.renderer->draw(c, shadow, get_render_bounds());

		GUI::Texture bg = Skin::get().FlowWindow.Inactive;
		bg.mul_color = float4(22, 28, 38, 255) / 255.0f;
		c.renderer->draw(c, bg, get_render_bounds());
	}
};

class GraphRender : public Window, public GUI::user_interface
{
	HAL::SwapChain::ptr swap_chain;

	tick_timer main_timer;
	ivec2 new_size;

	std::future<void> task_future;
	std::future<void> task_end;
	std::promise<void> promise_end;

	Graph graph;
	bool alive = false;
	count_meter fps;

	GUI::Elements::label::ptr label_fps;
	GUI::Elements::label::ptr label_tiles;
	GUI::Elements::label::ptr label_graph_cache;
	GUI::Elements::label::ptr instance_info;

	GUI::Elements::stat_graph::ptr graph_fps;
	GUI::Elements::stat_graph::ptr graph_frametime;
	GUI::Elements::stat_graph::ptr graph_vram;
	GUI::Elements::stat_graph::ptr graph_upload;
	GUI::Elements::stat_graph::ptr graph_readback;

	std::shared_ptr<triangle_drawer> drawer;
	std::shared_ptr<triangle_drawer> drawer2;
	std::shared_ptr<FrameFlowGraph> frameFlowGraph;
	GUI::base::ptr area;


	size_t graph_usage = 0;
		Pipelines::UIPipeline pipeline;
public:
	void on_destroy() override
	{

		Application::get().shutdown();
	}

	virtual void render()
	{
		PROFILE(L"render");
		if (swap_chain) swap_chain->resize(new_size);

		{
			std::lock_guard<std::mutex> g(m);

			if (GetAsyncKeyState('R'))
			{
				RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->signal_and_wait();
				//	RenderSystem::get().device().get_queue(HAL::CommandListType::COMPUTE)->signal_and_wait();
				//	RenderSystem::get().device().get_queue(HAL::CommandListType::COPY)->signal_and_wait();

				//   AssetManager::get().reload_resources();
				HAL::pixel_shader::reload_all();
				HAL::vertex_shader::reload_all();
				HAL::geometry_shader::reload_all();
				HAL::hull_shader::reload_all();
				HAL::domain_shader::reload_all();
				HAL::compute_shader::reload_all();
				HAL::library_shader::reload_all();
				HAL::mesh_shader::reload_all();
				HAL::amplification_shader::reload_all();

//				HAL::Texture::reload_all();
			}

			auto f_gc = thread_pool::get().enqueue([]()
			{
				PROFILE(L"GarbageCollect");
				RenderSystem::get().device().get_heap_factory().GarbageCollect();
			});

			Profiler::get().on_frame(frame_counter++);
			

			GUI::user_interface::size = new_size;
			if (fps.tick())
			{
				size_t total = 0;
				//for (auto& h : HAL::DescriptorHeapManager::get().heaps)
				//{
				//	if (h)
				//	{
				//		total += h->used_size();
				//	}
				//}


				size_t total_gpu = 0;


				label_fps->text = std::to_string(fps.get()) + " " + std::to_string(RenderSystem::get().device().get_vram()) + " "
					+ std::to_string(total) + " " + std::to_string(total_gpu) + " " + std::to_string(graph_usage);
			}

			float frame_dt = (float)main_timer.tick();

			// fire independent work onto thread pool
			auto f_asset = thread_pool::get().enqueue([]()
			{
				PROFILE(L"AssetManager");
				AssetManager::get().tact();
			});

			auto f_rtx = thread_pool::get().enqueue([]()
			{
				PROFILE(L"RTX update");
				RTX::get().update();
			});

			auto f_times = thread_pool::get().enqueue([this, frame_dt]()
			{
				PROFILE(L"push_times");
				auto& dev = RenderSystem::get().device();
				if (graph_fps)       graph_fps->push(1.0f / frame_dt, frame_dt);
				if (graph_frametime) graph_frametime->push(frame_dt * 1000.0f, frame_dt);

				static float vram_accum = 0.0f;
				static size_t vram_cached = 0;
				vram_accum += frame_dt;
				if (vram_accum >= 1.0f) { vram_accum = 0.0f; vram_cached = dev.get_vram(); }
				if (graph_vram)      graph_vram->push((float)vram_cached, frame_dt);

				if (graph_upload)    graph_upload->push((float)dev.get_upload_heap(), frame_dt);
				if (graph_readback)  graph_readback->push((float)dev.get_readback_heap(), frame_dt);
			});

			
			// wait for all three before setup_graph
			{ PROFILE(L"async_wait"); f_asset.wait(); f_rtx.wait(); f_times.wait();  f_gc.wait(); }
		   	process_ui(frame_dt);

			swap_chain->wait_for_free();

			PROFILE(L"CPU FRAME");
			setup_graph();

			{
				PROFILE(L"add_passes");

				// Hand this frame's asset previews (collected in create_graph)
				// to AssetPreview's instance slots. Every slot is rewritten,
				// including the empty tail: add_passes registers only the
				// filled ones, and a slot left from last frame would capture a
				// widget that may since have been destroyed. More live
				// previews than MaxCount silently drops the excess.
				{
					auto& previews = graph.get_context<AssetPreviewContext>().renders;
					for (uint32_t i = 0; i < Passes::AssetPreview::MaxCount; ++i)
						pipeline.assetPreview.render_funcs[i] =
							i < previews.size() ? previews[i] : Passes::AssetPreview::render_func_type{};
				}

				pipeline.add_passes(graph);

			}

			// Register the swapchain as a passed resource only after all passes
			// exist: max_passes (used to pre-size per-state append storage) is
			// captured from passes.size() at resource-creation time.
			graph.builder.pass_texture(FrameGraph::ResourceID::swapchain, swap_chain->get_current_frame(), swap_chain->get_fence(), ResourceFlags::Required);

			// This pipeline's [PreSetup] passes' real side effects (PreScene's
			// raytrace_scene->new_frame(), NRD_REBLUR_Execute's ensure_pools(),
			// CubeSky's sun-direction diff) run here, unconditionally, before
			// any pass's own setup() -- see pass_defaults.h's own comment on
			// the option.
			pipeline.run_pre_setups(graph);

			// Snapshots every context field and diffs it against last frame, so
			// a pass whose condition inputs are unchanged can reuse its previous
			// SetupResult (autogen/context_deps.h). Must run after
			// run_pre_setups() -- [PreSetup] hooks write context fields -- and
			// before setup() reads the mask.
			FrameGraph::update_context_dirty_mask(graph);

			// Contexts are frozen from here: every setup_func is generated and
			// only reads Table:: contexts, and run_pre_setups() above was the
			// last thing that could write one. So the graph this frame will
			// build is fully determined now, which is what makes the key
			// computable before setup() rather than partway through it.
			const uint64_t graph_key = graph.compute_graph_key();

			// Replay a stored plan when one matches, otherwise derive the graph
			// and record it. LoadGraph is what removes the serial per-pass
			// declare loop; it is gated until validation says the key is sound.
			const bool replayed = graph.use_graph_plan && graph.LoadGraph(graph_key);
			label_graph_cache->text = replayed ? "Graph: cached" : "Graph: live";
			if (!replayed)
			{
				graph.setup();
				graph.DumpGraph(graph_key);
			}
			else if (graph.validate_graph_plan)
			{
				// Round-trip check: re-dumping the state LoadGraph just restored
				// must reproduce the very plan it came from. DumpGraph compares
				// against the stored entry and counts any difference, so a replay
				// that restored something wrong shows up as a mismatch instead of
				// as a corrupted barrier several hundred frames later.
				graph.DumpGraph(graph_key);
			}

			graph.compile(swap_chain->m_frameIndex);
				  if (!replayed)	   	graph.DumpGraph(graph_key);
			graph.render();


			auto fence = graph.commit_command_lists();

			{
				PROFILE(L"reset");
				graph.reset();
			}

			if (frame_counter <= 5)
				Log::get() << "[Render] calling present frame " << frame_counter << Log::endl;
			swap_chain->present();
			if (frame_counter <= 5)
				Log::get() << "[Render] present returned frame " << frame_counter << Log::endl;
		}


		if (alive)
		{

			task_future = thread_pool::get().enqueue([this]()
				{
					render();
				});
		}
		else
		{

			promise_end.set_value();
		}
	}


	void setup_graph()
	{
		PROFILE(L"setup_graph");
		graph.start_new_frame();

		// Rebuilt from scratch every frame by the widget tree below: a context
		// outlives the frame, so a preview window closed since the last one
		// would otherwise leave behind a callback capturing its destroyed
		// `this` for add_passes to register.
		graph.get_context<AssetPreviewContext>().renders.clear();

		{
			PROFILE(L"create_graph");
			create_graph(graph);
		}

		static bool gen = false;

		if (!gen && GetAsyncKeyState('N'))
		{
			{
				//	GUI::Elements::Debug::TimerWatcher::ptr t(new GUI::Elements::Debug::TimerWatcher());
				//	t->init(&Profiler::get());
				//	dock->get_tabs()->add_page("Profiler", t);

				//	GUI::Elements::Debug::TimeGraph::ptr t2(new GUI::Elements::Debug::TimeGraph());



				//docker->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(frameFlowGraph));
			}


			gen = true;
			frameFlowGraph->clear();

			struct res_stage
			{};
			std::map<ResourceAllocInfo*, ::FlowGraph::parameter::ptr> resource_stages;
			std::map<Pass*, float> pass_positions;
			std::map<CommandListType, float> list_positions;


			for (auto& chain : graph.builder.alloc_resources)
			{
				if (chain.empty()) continue;
				auto& info = chain.active();
				if (info.passed)
				{
					auto input = frameFlowGraph->register_input(info.name());
					resource_stages[&info] = input;
				}
			}
			uint offset = 0;
			for (auto pass : graph.builder.enabled_passes)
			{
				if (!pass->enabled)
					continue;

				auto node = std::make_shared<PassNode>(graph, pass->name.ptr);
				node->name = convert(std::wstring(pass->name.ptr)) + " " + std::to_string(pass->id);

				CommandListType pass_type = check(pass->flags & PassFlags::Compute)
					? CommandListType::COMPUTE
					: CommandListType::DIRECT;
				if (!pass->enabled)
				{
					node->color = float4(1, 0, 0, 1);
				}

				if (check(pass->flags & PassFlags::Required))
				{
					node->color = float4(1, 1, 0, 1);
				}

				float my_pos = list_positions[pass_type];

				for (auto& sync_pass : pass->sync_state.values)
				{
					if (!sync_pass) continue;
					my_pos = std::max(my_pos, pass_positions[const_cast<Pass*>(sync_pass)]);
				}

				node->pos = { my_pos + 800, check(pass->flags & PassFlags::Compute) * 500 };


				pass_positions[pass] = node->pos.x;
				list_positions[pass_type] = node->pos.x;

				frameFlowGraph->register_node(node);
				offset = std::max(offset, uint(node->pos.x));

				for (auto& [version, resource_flags] : pass->used.resources)
				{
					auto* info = graph.builder.get(version);
					if (!info) continue;

					if (!FrameGraph::UsedResources::contains(pass->used.resource_creations, version))
					{
						auto input = node->register_input(info->name());
						auto prev = resource_stages[info];

						if (prev)
						{
							prev->link(input);
						}

						if (check(resource_flags & FrameGraph::WRITEABLE_FLAGS))
						{
							auto output = node->register_output(info->name());
							resource_stages[info] = output;
						}
					}
				}


				for (auto& [version, resource_flags] : pass->used.resources)
				{
					auto* info = graph.builder.get(version);
					if (!info) continue;

					if (FrameGraph::UsedResources::contains(pass->used.resource_creations, version))
					{
						auto output = node->register_output(info->name());
						resource_stages[info] = output;
					}
				}
			}


			for (auto& chain : graph.builder.alloc_resources)
			{
				if (chain.empty()) continue;
				auto& info = chain.active();
				if (info.passed && check(info.flags & ResourceFlags::Required))
				{
					auto input = frameFlowGraph->register_output(info.name());
					auto prev = resource_stages[&info];

					if (prev)
					{
						prev->link(input);
					}
				}
			}
			frameFlowGraph->pos_in = { -800, 0 };
			frameFlowGraph->pos_out = { offset + 800, 0 };
		}
	}

	GUI::Elements::dock_base::ptr docker;

	// Doesn't make GraphRender itself derive VariableContext (it already
	// inherits GUI::base's add_child(GUI::base::ptr) via GUI::user_interface;
	// adding a second, unrelated add_child from VariableContext would make
	// every existing unqualified add_child(...) call elsewhere in this class
	// ambiguous) -- a plain owned context, same as AssetRenderer::preview_context.
	std::unique_ptr<VariableContext> graph_context = VariableContext::create(L"GraphRender");

	GraphRender()
	{
		// graph is a plain member, already fully constructed (default Graph())
		// by the time this body runs, so it needs this explicit re-parent
		// rather than a Scope wrapping its construction. Without it, it lands
		// as a flat "Graph" sibling under global instead of nested here -- and
		// since Graph's own VariableContext ctor always uses the fixed literal
		// name "Graph" (see FrameGraph.cpp), it's otherwise indistinguishable
		// from AssetRenderer/SceneTextureRenderer's own Graph members.
		graph_context->add_child(&graph);

		//scale = 1.25f;
		Window::input_handler = this;
		HAL::swap_chain_desc desc;
		desc.format = HAL::Format::B8G8R8A8_UNORM;
		desc.fullscreen = nullptr;
		desc.stereo = false;
		desc.window = this;
		swap_chain = std::make_shared<HAL::SwapChain>(RenderSystem::get().device(), desc);

		set_capture = [this](bool v)
			{
				if (v)
					SetCapture(get_hwnd());
				else
					ReleaseCapture();
			};

		on_resize(get_size());


		{
			GUI::Elements::image::ptr back(new GUI::Elements::image);
			back->texture = Skin::get().Fill;
			back->texture.tiled = true;
			back->width_size = GUI::size_type::MATCH_PARENT;
			back->height_size = GUI::size_type::MATCH_PARENT;
			add_child(back);
		}

		EVENT("Start UI");
		// 		if (false)
		{
			area.reset(new GUI::base());
			area->docking = GUI::dock::FILL;
			add_child(area);
			auto d = std::make_shared<GUI::Elements::dock_base>();
			docker = d;
			d->docking = GUI::dock::FILL;

			// Material editor / "edit material" / "create material" all used
			// to spawn their own floating window+dock; open them as a page
			// in the center dock instead, the same place "Debug Graph"
			// (below) opens FrameGraphDebug. The manager only ever builds
			// the graph's canvas (create_canvas) -- this is the one place
			// that decides where it's actually hosted.
			GUI::Elements::FlowGraph::manager::on_open_tab = [this](std::string name, GUI::base::ptr content) -> GUI::Elements::tab_button::ptr
			{
				return docker->get_tabs()->add_page(name, content);
			};
#ifndef HAL_BACKEND_VULKAN
			{
				EVENT("Start Drawer");
				drawer.reset(new triangle_drawer());
				drawer->docking = GUI::dock::FILL;

				// Floating debug-view selector over the viewport: each button sets
				// graph's DebugContext::mode, which repoints what UI_Render composites.
				{
					// force_non_dlssrr: NRD_REBLUR_Execute/NRD_GBufferPack (and
					// so RTXIndirectDenoised/Preview, NRD_ViewZ,
					// NRD_NormalRoughness) are gated OFF under DLSS-RR now --
					// it does its own reconstruction/denoising, so NRD is
					// redundant there and doesn't run (see NRD_GBufferPack's
					// own comment, [[project-nrd-integration]]). Was the
					// opposite direction (forced DLSS-RR ON) before that gate
					// flipped; selecting one of these views without also
					// leaving DLSS-RR would show stale/empty content
					// (UI_Render silently falls back when the resource
					// wasn't produced this frame), so this entry flips the
					// upscaler too instead of requiring two separate
					// dropdown changes. FSR is always available, so it's the
					// one certain fallback (see the upscaler dropdown below).
					struct DbgOpt { const char* name; FrameGraph::DebugMode mode; bool force_non_dlssrr = false; };
					static const DbgOpt dbg_opts[] = {
						{ "Final",         FrameGraph::DebugMode::Final },
						{ "Albedo",        FrameGraph::DebugMode::Albedo },
						{ "Motion",        FrameGraph::DebugMode::Motion },
						{ "Voxel Trace",   FrameGraph::DebugMode::VoxelTrace },
						{ "RTX",           FrameGraph::DebugMode::RTX },
						{ "RTX Indirect (REBLUR)", FrameGraph::DebugMode::RTXIndirectDenoised, true },
						{ "RTX Indirect (REBLUR), Unpacked", FrameGraph::DebugMode::RTXIndirectDenoisedUnpacked, true },
						{ "NRD ViewZ",             FrameGraph::DebugMode::NRDViewZ, true },
						{ "NRD Normal/Roughness",  FrameGraph::DebugMode::NRDNormalRoughness, true },
						{ "Raw Depth Mips",        FrameGraph::DebugMode::RawDepthMips },
					};

					auto toolbar = std::make_shared<debug_toolbar_panel>();
					toolbar->docking     = GUI::dock::NONE;
					toolbar->pos         = vec2(8, 8);
					toolbar->height_size = GUI::size_type::MATCH_CHILDREN;
					toolbar->width_size  = GUI::size_type::MATCH_CHILDREN;
					toolbar->padding     = { 4, 4, 4, 4 };

					auto debug_combo = std::make_shared<GUI::Elements::combo_box>();
					debug_combo->docking = GUI::dock::TOP;
					debug_combo->size = { 140, 24 };
					for (auto& o : dbg_opts)
					{
						auto mode = o.mode;
						auto force_non_dlssrr = o.force_non_dlssrr;
						debug_combo->add_item(o.name)->on_select =
							[this, mode, force_non_dlssrr]()
							{
								graph.get_context<FrameGraph::DebugContext>().mode = mode;
								if (force_non_dlssrr && g_upscaler_type == UpscalerType::DLSSRR)
									set_upscaler_type(UpscalerType::FSR);
							};
					}
					toolbar->add_child(debug_combo);

					// Upscaler selector, writes g_upscaler_type. Only lists
					// options actually available on this hardware (FSR is
					// always valid); on/off is still via the separate
					// "downsampled" toggle (g_upscaling_enabled), not an
					// option here.
					{
						struct UpscalerOpt { const char* name; UpscalerType type; };
						std::vector<UpscalerOpt> upscaler_opts = { { "FSR", UpscalerType::FSR } };
						// Same predicate set_upscaler_type() clamps with, so the
						// list can never offer an option the setter would refuse.
						if (upscaler_is_available(UpscalerType::DLSS))
							upscaler_opts.push_back({ "DLSS", UpscalerType::DLSS });
						if (upscaler_is_available(UpscalerType::DLSSRR))
							upscaler_opts.push_back({ "DLSS-RR", UpscalerType::DLSSRR });

						auto upscaler_combo = std::make_shared<GUI::Elements::combo_box>();
						upscaler_combo->docking = GUI::dock::TOP;
						upscaler_combo->size = { 140, 24 };
						for (auto& o : upscaler_opts)
						{
							auto type = o.type;
							auto type_name = std::string(o.name);
							upscaler_combo->add_item(o.name)->on_select =
								[type, type_name]()
								{
									Log::get() << "[Upscaler] type changed " << (int)g_upscaler_type
										<< " -> " << (int)type << " (" << type_name << ")" << Log::endl;
									set_upscaler_type(type);
								};
							if (type == g_upscaler_type)
								upscaler_combo->get_label()->text = o.name;
						}
						toolbar->add_child(upscaler_combo);
					}

					// Indirect/Reflection source selectors, writing
					// g_indirect_source/g_reflection_source (see
					// [[project-nrd-integration]]): which raw signal feeds
					// NRD -- the voxel-cone-traced one (VoxelScreen/
					// ScreenReflection) or the raw RTX reference
					// (IndirectRTX/ReflectionRTX). Purely a quality/
					// performance comparison; NRD denoises either.
					{
						struct IndirectSrcOpt { const char* name; IndirectSource src; };
						static const IndirectSrcOpt indirect_src_opts[] = {
							{ "My VCT",   IndirectSource::MyVCT },
							{ "RTX Ref",  IndirectSource::RTXReference },
						};

						auto indirect_src_combo = std::make_shared<GUI::Elements::combo_box>();
						indirect_src_combo->docking = GUI::dock::TOP;
						indirect_src_combo->size = { 140, 24 };
						for (auto& o : indirect_src_opts)
						{
							auto src = o.src;
							indirect_src_combo->add_item(o.name)->on_select =
								[this, src]()
								{
									graph.get_context<Table::IndirectGISelectors>().indirect_source = src;
								};
							if (src == graph.get_context<Table::IndirectGISelectors>().indirect_source)
								indirect_src_combo->get_label()->text = o.name;
						}
						toolbar->add_child(indirect_src_combo);

						struct ReflectionSrcOpt { const char* name; ReflectionSource src; };
						static const ReflectionSrcOpt reflection_src_opts[] = {
							{ "My Reflection", ReflectionSource::MyReflection },
							{ "RTX Ref",       ReflectionSource::RTXReference },
						};

						auto reflection_src_combo = std::make_shared<GUI::Elements::combo_box>();
						reflection_src_combo->docking = GUI::dock::TOP;
						reflection_src_combo->size = { 140, 24 };
						for (auto& o : reflection_src_opts)
						{
							auto src = o.src;
							reflection_src_combo->add_item(o.name)->on_select =
								[this, src]()
								{
									graph.get_context<Table::IndirectGISelectors>().reflection_source = src;
								};
							if (src == graph.get_context<Table::IndirectGISelectors>().reflection_source)
								reflection_src_combo->get_label()->text = o.name;
						}
						toolbar->add_child(reflection_src_combo);
					}

					// DLSS-quality selector, writes g_upscaling_dlss_mode.
					// "Off" isn't offered — DLSS on/off is via FSR/DLSS
					// mutual exclusion elsewhere, not DLSSMode::Off.
					if (nvidia::DLSS::get().available())
					{
						struct DlssOpt { const char* name; nvidia::DLSSMode mode; };
						static const DlssOpt dlss_opts[] = {
							{ "Max Performance",   nvidia::DLSSMode::MaxPerformance },
							{ "Balanced",          nvidia::DLSSMode::Balanced },
							{ "Max Quality",       nvidia::DLSSMode::MaxQuality },
							{ "Ultra Quality",     nvidia::DLSSMode::UltraQuality },
							{ "Ultra Performance", nvidia::DLSSMode::UltraPerformance },
							{ "DLAA",              nvidia::DLSSMode::DLAA },
						};

						auto dlss_combo = std::make_shared<GUI::Elements::combo_box>();
						dlss_combo->docking = GUI::dock::TOP;
						dlss_combo->size = { 140, 24 };
						for (auto& o : dlss_opts)
						{
							// Skip modes SL reports unsupported (e.g. UltraQuality
							// at 1920x1080 returns render_size (0,0)).
							nvidia::DLSSOptimalSettings probe{};
							if (!nvidia::DLSS::get().get_optimal_settings(o.mode, { 1920, 1080 }, /*hdr=*/true, probe))
								continue;

							auto mode = o.mode;
							auto mode_name = std::string(o.name);
							dlss_combo->add_item(o.name)->on_select =
								[mode, mode_name]()
								{
									Log::get() << "[DLSS] mode changed " << (int)g_upscaling_dlss_mode
										<< " -> " << (int)mode << " (" << mode_name << ")" << Log::endl;
									g_upscaling_dlss_mode = mode;
								};
							if (mode == g_upscaling_dlss_mode)
								dlss_combo->get_label()->text = o.name;
						}
						toolbar->add_child(dlss_combo);

						// Manual render-scale override: percent between DLSS's
						// reported render_size_min/max for the current mode.
						// "Recommended" resets to DLSS's own optimal size.
						auto scale_row = std::make_shared<GUI::base>();
						scale_row->docking     = GUI::dock::TOP;
						scale_row->height_size = GUI::size_type::FIXED;
						scale_row->size        = { 140, 22 };
						toolbar->add_child(scale_row);

						auto scale_value_label = std::make_shared<GUI::Elements::label>();
						scale_value_label->docking     = GUI::dock::RIGHT;
						scale_value_label->width_size  = GUI::size_type::FIXED;
						scale_value_label->size        = { 44, 0 };
						scale_value_label->magnet_text = FW1_RIGHT | FW1_VCENTER | FW1_NOWORDWRAP;
						scale_row->add_child(scale_value_label);

						auto scale_slider = std::make_shared<GUI::Elements::float_slider>();
						scale_slider->min         = 0.0f;
						scale_slider->max         = 100.0f;
						scale_slider->value       = g_upscaling_dlss_scale_override >= 0.0f ? g_upscaling_dlss_scale_override : 50.0f;
						scale_slider->docking     = GUI::dock::FILL;
						scale_slider->height_size = GUI::size_type::FIXED;
						scale_slider->size        = { 0, 14 };
						scale_slider->y_type      = GUI::pos_y_type::CENTER;
						scale_row->add_child(scale_slider);

						auto update_scale_label = [scale_value_label]()
						{
							scale_value_label->text = g_upscaling_dlss_scale_override >= 0.0f
								? std::to_string(int(g_upscaling_dlss_scale_override)) + "%"
								: "Rec.";
						};
						update_scale_label();

						scale_slider->on_change = [update_scale_label](float v)
						{
							Log::get() << "[DLSS] scale override changed " << g_upscaling_dlss_scale_override
								<< " -> " << v << Log::endl;
							g_upscaling_dlss_scale_override = v;
							update_scale_label();
						};

						auto recommended_but = std::make_shared<GUI::Elements::button>();
						recommended_but->get_label()->text = "Recommended";
						recommended_but->docking     = GUI::dock::TOP;
						recommended_but->height_size = GUI::size_type::FIXED;
						recommended_but->size        = { 140, 22 };
						recommended_but->on_click = [scale_slider, update_scale_label](GUI::Elements::button::ptr)
						{
							Log::get() << "[DLSS] scale override reset to Recommended (was "
								<< g_upscaling_dlss_scale_override << ")" << Log::endl;
							g_upscaling_dlss_scale_override = -1.0f;

							// Move the slider handle to reflect where "recommended"
							// sits, for display only — 1920x1080 is a representative
							// probe (the real window size isn't known this early;
							// same limitation as the DLSS-quality combobox above).
							nvidia::DLSSOptimalSettings settings{};
							if (nvidia::DLSS::get().get_optimal_settings(g_upscaling_dlss_mode, { 1920, 1080 }, true, settings) &&
							    settings.render_size_max.x > settings.render_size_min.x)
							{
								float t = float(settings.render_size.x - settings.render_size_min.x) /
								          float(settings.render_size_max.x - settings.render_size_min.x);
								scale_slider->value = t * 100.0f;
							}
							update_scale_label();
						};
						toolbar->add_child(recommended_but);
					}

					drawer->GUI::base::add_child(toolbar);
				}

				d->get_tabs()->add_page("Game", drawer);
				EVENT("End Drawer");
			}
#endif

			{
				//	auto text = std::make_shared<GUI::Elements::MultiLineLabel>();
				////	auto f = FileSystem::get().get_file(to_path(L"main.cpp"))->load_all();


					//text->text = f;
					///d->get_tabs()->add_page("text", text);
			}


	

			{
				GUI::Elements::list_box::ptr l(new GUI::Elements::list_box());
				auto dock = d->get_dock(GUI::dock::BOTTOM);
				dock->size = { 100, 100 };
				area->add_child(d);
				dock->get_tabs()->add_page("TaskViewer", std::make_shared<GUI::Elements::Debug::TaskViewer>());

				{
					auto b = std::make_shared<GUI::base>();


					auto folders = std::make_shared<GUI::Elements::tree<VariableContext>>();
					folders->size = { 200, 150 };
					folders->docking = GUI::dock::LEFT;

					auto table = std::make_shared<GUI::base>();
					table->docking = GUI::dock::FILL;

					b->add_child(folders);
					b->add_child(table);

					folders->on_select = [this, table](VariableContext* elem)
						{
							table->remove_all();

							for (auto v : elem->variables)
							{
								auto property = GUI::Elements::create_property(*v);
								table->add_child(property);
							}
						};

					folders->init(&VariableContext::get());

					dock->get_tabs()->add_page("Properties", b);
				}
				dock->get_tabs()->add_page("output", std::make_shared<GUI::Elements::Debug::OutputWindow>());
				//       GUI::Elements::tree::ptr t(new GUI::Elements::tree());
				//                    t->init(drawer->scene.get());
				//                    dock->get_tabs()->add_page("Scene", t);
				{
					//	GUI::Elements::Debug::TimerWatcher::ptr t(new GUI::Elements::Debug::TimerWatcher());
					//	t->init(&Profiler::get());
					//	dock->get_tabs()->add_page("Profiler", t);

					GUI::Elements::Debug::TimeGraph::ptr t2(new GUI::Elements::Debug::TimeGraph());

					dock->get_tabs()->add_page("Graph", t2);


					frameFlowGraph = std::make_shared<FrameFlowGraph>();

					{
						auto canva = GUI::Elements::FlowGraph::manager::get().create_canvas(frameFlowGraph);
						auto btn   = dock->get_tabs()->add_page(frameFlowGraph->name, canva);
						GUI::Elements::FlowGraph::manager::get().register_tab(frameFlowGraph, btn);
					}
				}

				{
					auto stats_panel = std::make_shared<GUI::base>();
					stats_panel->docking      = GUI::dock::FILL;
					stats_panel->width_size   = GUI::size_type::MATCH_PARENT;
					stats_panel->height_size  = GUI::size_type::MATCH_PARENT;

					graph_fps.reset(new GUI::Elements::stat_graph());
					graph_fps->title     = "FPS";
					graph_fps->size      = {0, 70};
					graph_fps->min_range = 150.0f;
					graph_fps->mode      = GUI::Elements::stat_graph::draw_mode::lines;
					stats_panel->add_child(graph_fps);

					graph_frametime.reset(new GUI::Elements::stat_graph());
					graph_frametime->title      = "Frame Time";
					graph_frametime->unit       = "ms";
					graph_frametime->size       = {0, 70};
					graph_frametime->min_range  = 20.0f;
					graph_frametime->line_color = {0.95f, 0.70f, 0.20f, 1.00f};
					graph_frametime->fill_color = {0.95f, 0.70f, 0.20f, 0.15f};
					graph_frametime->mode       = GUI::Elements::stat_graph::draw_mode::lines;
					stats_panel->add_child(graph_frametime);

					graph_vram.reset(new GUI::Elements::stat_graph());
					graph_vram->title      = "VRAM";
					graph_vram->size       = {0, 70};
					graph_vram->min_range  = 100.0f;
					graph_vram->line_color = {0.90f, 0.30f, 0.30f, 1.00f};
					graph_vram->fill_color = {0.90f, 0.30f, 0.30f, 0.15f};
					graph_vram->mode       = GUI::Elements::stat_graph::draw_mode::lines;
					stats_panel->add_child(graph_vram);

					graph_upload.reset(new GUI::Elements::stat_graph());
					graph_upload->title      = "Upload Heap";
					graph_upload->unit       = "MB";
					graph_upload->size       = {0, 70};
					graph_upload->min_range  = 50.0f;
					graph_upload->line_color = {0.40f, 0.70f, 1.00f, 1.00f};
					graph_upload->fill_color = {0.40f, 0.70f, 1.00f, 0.15f};
					graph_upload->mode       = GUI::Elements::stat_graph::draw_mode::lines;
					stats_panel->add_child(graph_upload);

					graph_readback.reset(new GUI::Elements::stat_graph());
					graph_readback->title      = "Readback Heap";
					graph_readback->unit       = "MB";
					graph_readback->size       = {0, 70};
					graph_readback->min_range  = 50.0f;
					graph_readback->line_color = {1.00f, 0.60f, 0.20f, 1.00f};
					graph_readback->fill_color = {1.00f, 0.60f, 0.20f, 0.15f};
					graph_readback->mode       = GUI::Elements::stat_graph::draw_mode::lines;
					stats_panel->add_child(graph_readback);

					dock->get_tabs()->add_page("Stats", stats_panel);
					dock->size = {100, 230};
				}
			}

			{
				{
					GUI::Elements::menu_strip::ptr menu(new GUI::Elements::menu_strip());
					auto file = menu->add_item("File")->get_menu();
					auto edit = menu->add_item("Edit")->get_menu();
					auto help = menu->add_item("Help"); // ->get_menu();
					file->add_item("New")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
						{
							add_task([this]()
								{
									if (!drawer) return;
									drawer->scene->remove_all();
								});
						};
					file->add_item("Open")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
						{
							add_task([this]()
								{
									if (!drawer) return;
									try
									{
										auto f = FileSystem::get().get_file(to_path(L"scene.dat"))->load_all();
										Serializer::deserialize(f, *drawer->scene);
									}
									catch (std::exception e)
									{
										message_box("error", "cant open", [](bool)
											{
											});
									}
								});
						};
					file->add_item("Save")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
						{
							if (!drawer) return;
							auto data = Serializer::serialize(*drawer->scene);
							FileSystem::get().save_data(to_path(L"scene.dat"), data);
						};
					file->add_item("Quit")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
						{
							on_destroy();
						};

					edit->add_item("Debug Graph")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
						{

							docker->get_tabs()->add_page("GraphDebug", FrameGraphDebug::create_debug_layout(graph));

						};

					auto add = edit->add_item("Add smth")->get_menu();
					add->add_item("Mesh");
					add->add_item("Material");
					add->add_item("Sound");
					//->get_menu()->add_item("12454");
					add_child(menu);
				}
				{
					GUI::Elements::status_bar::ptr bar(new GUI::Elements::status_bar());
					label_fps = GUI::Elements::label::ptr(new GUI::Elements::label());
					instance_info = GUI::Elements::label::ptr(new GUI::Elements::label());
					label_tiles = GUI::Elements::label::ptr(new GUI::Elements::label());
					label_graph_cache = GUI::Elements::label::ptr(new GUI::Elements::label());

					bar->add_child(label_fps);
					bar->add_child(label_tiles);
					bar->add_child(label_graph_cache);

					bar->add_child(instance_info);
					instance_info->docking = GUI::dock::RIGHT;
					label_tiles->margin = { 20, 0, 0, 0 };
					label_graph_cache->margin = { 20, 0, 0, 0 };
					add_child(bar);
				}


				{
					EVENT("Start Asset Explorer");
					auto dock = d->get_dock(GUI::dock::RIGHT);
					GUI::Elements::asset_explorer::ptr cont(new GUI::Elements::asset_explorer());
					dock->get_tabs()->add_page("Asset Explorer", cont);
					dock->size = { 400, 400 };

					// Open a texture asset in a preview window ("Preview" in the
					// asset context menu). Lives here (above the Graphics module) so
					// it can use resource_preview.
					GUI::Elements::asset_item::on_open_preview = [this](std::shared_ptr<Asset> a)
					{
						if (!a) return;
						auto wnd     = std::make_shared<GUI::Elements::window>();
						auto content = std::make_shared<asset_preview_content>(a);
						wnd->add_child(content);
						wnd->pos  = { 250, 250 };
						wnd->size = { 520, 560 };
						if (auto ui = get_user_ui()) ui->add_child(wnd);
					};

					// Live material preview embedded in the material graph's output node.
					MaterialGraph::create_preview_hook = [](std::shared_ptr<Asset> a) -> GUI::base::ptr
					{
						if (!a) return nullptr;
						auto box         = std::make_shared<GUI::base>();
						box->width_size  = GUI::size_type::FIXED;
						box->height_size = GUI::size_type::FIXED;
						box->size        = { 160, 160 };
						box->add_child(std::make_shared<asset_preview_content>(a));
						return box;
					};

					// Per-node live-value thumbnail (see node_preview_thumbnail).
					MaterialGraph::create_node_preview_hook = [](std::shared_ptr<Asset> a, FlowGraph::Node* node) -> GUI::base::ptr
					{
						if (!a || !node) return nullptr;
						return std::make_shared<node_preview_thumbnail>(a, node);
					};

					// Builds the "Material Settings" panel embedded in a
					// material graph's editor dock -- see materials::
					// open_material_editor, which is what actually wraps the
					// canvas+panel together now (called wherever a material
					// graph gets opened, e.g. AssetExplorer's "Edit").
					// Materials can't build main.cpp's material_settings_panel
					// directly (it's app-layer GUI), hence the hook.
					materials::create_settings_panel_hook = [](materials::universal_material* mat, ::FlowGraph::graph* g) -> GUI::base::ptr
					{
						return std::make_shared<material_settings_panel>(mat, g);
					};

					// Attach/detach a materials::MaterialPreviewSession for as
					// long as a material graph is open in an editor canvas --
					// lives here (above both GUI and Graphics) since canvas
					// (GUI) can't know about Materials, and Materials can't
					// know when a canvas opens/closes.
					GUI::Elements::FlowGraph::canvas::on_open = [](GUI::Elements::FlowGraph::canvas* c)
					{
						materials::MaterialPreviewSession::open(c->g);
					};
					GUI::Elements::FlowGraph::canvas::on_close = [](GUI::Elements::FlowGraph::canvas* c)
					{
						materials::MaterialPreviewSession::close(c->g);
					};
					EVENT("End Asset Explorer");
				}
			}
		}
	}

	~GraphRender() override
	{

	}


	void stop()
	{

		alive = false;
		task_end.wait();

		while (Events::Runner::has_tasks())
		{
			PROFILE(L"process_tasks");
			Events::Runner::process_tasks();
		}
	}

	void start()
	{
		promise_end = {};
		task_end = promise_end.get_future();
		alive = true;
		//task_future = thread_pool::get().enqueue([this]()
		//	{
		render();
		//	});

	}
	void on_resize(vec2 size) override
	{
		Log::get() << "[Resize] window resize requested " << size.x << "x" << size.y
			<< " (clamped from previous new_size " << new_size.x << "x" << new_size.y << ")" << Log::endl;
		new_size = vec2::max(size, vec2{ 64, 64 });

		/*bool was_alive = alive;
	if (was_alive) stop();

	//Sleep(10);
	if (was_alive)
	{

		start();
		RenderSystem::get().device().get_queue(CommandListType::DIRECT)->signal_and_wait();
		RenderSystem::get().device().get_queue(CommandListType::COMPUTE)->signal_and_wait();
		RenderSystem::get().device().get_queue(CommandListType::COPY)->signal_and_wait();
}*/

	}


	void on_size_changed(const vec2& r) override
	{
		user_interface::on_size_changed(r);
	}
};

class RenderApplication : public Application
{
	friend class Singleton<Application>;


	std::shared_ptr<GraphRender> main_window;
#ifdef OCULUS_SUPPORT
	std::shared_ptr<OVRRender> ovr;
#endif
	// TODO:: shared_objects
	std::atomic<bool> need_reload_resource;
	std::atomic<bool> need_print_screen;

protected:
	RenderApplication()
	{
		THREAD_SCOPE(GUI);
		FileSystem::get().register_provider(std::make_shared<native_file_provider>());
		GraphicsSystem::create();
		main_window = std::make_shared<GraphRender>();
		main_window->start();
	}

	~RenderApplication() override
	{
		shutdown();
		main_window->stop();
		main_window = nullptr;
		scheduler::reset();
		GraphicsSystem::reset();
	}


	void on_tick() override
	{
	//	if (GetAsyncKeyState(VK_ESCAPE))
	//		shutdown();

		Window::process_messages();
	}

	std::vector<std::string> file_open(const std::string& Name, const std::string& StartPath,
		const std::string& Extension) override
	{
		return Window::file_open(Name, StartPath, Extension);
	}
};

void SetupDebug()
{
	Log::create<WinErrorLogger>();
	FileTXTLogger::create();
	VSOutputLogger::create();
	//  ConsoleLogger::create();
	Log::get().set_logging_level(Log::LEVEL_ALL);
	// Here we can disable some of notification types
	ClassLogger<resource_system>::get().set_logging_level(Log::LEVEL_INFO);
	ClassLogger<Resource>::get().set_logging_level(Log::LEVEL_INFO);
	ClassLogger<HAL::Resource>::get().set_logging_level(Log::LEVEL_INFO);
	Log::get() << Log::LEVEL_INFO << "info text" << Log::endl;
	Log::get() << Log::LEVEL_WARNING << "warning text" << Log::endl;
	Log::get() << Log::LEVEL_DEBUG << "debug text" << Log::endl;
	Log::get() << Log::LEVEL_ERROR << "error text" << Log::endl;
}

struct test
{
	std::string str = "wtf";
	vec4 data = { 1, 2, 3, 4 };

	std::vector<vec2> vec;

	test()
	{
		vec.emplace_back(1, 2);
		vec.emplace_back(3, 4);
		vec.emplace_back(5, 6);
	}

	template <class T = void>
	void foo() requires(false)
	{}

	SERIALIZE()
	{
		ar&  NVP(str)& NVP(data)& NVP(vec);
	}
} v;

#include <shellscalingapi.h>


int APIENTRY WinMain(_In_ HINSTANCE hinst,
	_In_opt_ HINSTANCE,
	_In_ LPTSTR,
	_In_ int)
{
//	setlocale(LC_ALL, "");
	CoInitialize(NULL);
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	FlowGraph::FlowSystem::get().register_node("file", []()-> FlowGraph::graph::ptr
		{
			auto f = FileSystem::get().get_file(to_path("graph.flg"));

			if (f)
				return Serializer::deserialize<FlowGraph::graph>(f->load_all());

			return nullptr;
		});

	auto result_code = 0;
	SetupDebug();

	EVENT("start");
	Application::create<RenderApplication>();
	EVENT("create");


	// There can be error while creating, so test
	if (Application::is_good())
	{
		EVENT("run");
		Application::get().run(); // TODO: Create task and run engine (scheduling tasks)
		Application::reset();
	}

	else
		result_code = S_FALSE;

	CounterManager::get().print();
	EVENT("end");
	CoUninitialize();

	return result_code;
}
