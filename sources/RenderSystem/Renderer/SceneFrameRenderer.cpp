module Graphics:SceneFrameRenderer;

import :EngineAssets;
import :DDGI;
import :Tonemap;
import :Bloom;
import :UpscalingDLSS;

using namespace FrameGraph;

SceneFrameRenderer::SceneFrameRenderer(Scene::ptr scene)
	: VariableContext(L"Scene renderer"), m_vsm(m_pipeline), m_sky(m_pipeline), m_blue_noise(m_pipeline), m_smaa(m_pipeline), m_scene(scene)
{
	// Members constructed before this body landed under whatever Scope was
	// active then; re-parent the ones that are contexts, and nest everything
	// constructed from here on (mesh_renderer, VoxelGI) under this renderer.
	VariableContext::Scope self_scope(*this);
	add_child(m_forward_context.get());
	add_child(m_gpu_context.get());
	add_child(&m_vsm);

	m_forward_renderer = std::make_shared<main_renderer>();
	{
		VariableContext::Scope scope(*m_forward_context);
		m_forward_renderer->register_renderer(m_forward_meshes = std::make_shared<mesh_renderer>());
	}

	// The renderer the Scene pass draws the GBuffer with (SceneInfo::renderer).
	m_gpu_renderer = std::make_shared<main_renderer>();
	{
		VariableContext::Scope scope(*m_gpu_context);
		m_gpu_renderer->register_renderer(m_main_view_meshes = std::make_shared<mesh_renderer>());
	}

	// VSM's invalidation tracker registers scene event handlers once the
	// scene exists.
	m_vsm.attach_scene(m_scene);

	// Its voxel volumes are tiled 3D textures, and its lighting samples VSM's
	// atlas (a tiled depth texture).
	if (RenderSystem::get().device().is_tiled_3d_supported() && RenderSystem::get().device().is_tiled_depth_supported())
		m_voxel_gi = std::make_shared<VoxelGI>(m_pipeline, m_scene, m_vsm);

	// DDGI's [Multiple=5] passes (ddgi.prism) are runtime-wired, not
	// [Static] -- see DDGI.ixx's own comment on ddgi_register_passes.
	ddgi_register_passes(m_pipeline, m_vsm);
}

SceneFrameRenderer::~SceneFrameRenderer()
{
	RenderSystem::get().device().wait_idle();
}

void SceneFrameRenderer::begin_frame(Graph& graph, const SceneFrameParams& params)
{
	m_params = params;
	const ivec2 size = params.output_size;

	m_scene->update(*graph.builder.current_frame);

	auto& timeinfo  = graph.get_context<TimeInfo>();
	auto& skyinfo   = graph.get_context<SkyInfo>();
	auto& caminfo   = graph.get_context<CameraInfo>();
	auto& sceneinfo = graph.get_context<SceneInfo>();
	auto& vp        = graph.get_context<ViewportInfo>();

	g_upscaling_enabled = params.upscale;

	// Mirrors g_upscaler_type/g_upscaling_enabled into the SIG context --
	// see UpscalingDLSS.prism's own comment on UpscalerSelectors for why.
	{
		auto& upscaler_ctx = graph.get_context<Table::Post::Upscale::UpscalerSelectors>();
		upscaler_ctx.upscaler_type     = g_upscaler_type;
		upscaler_ctx.upscaling_enabled = g_upscaling_enabled;
	}
	// Mirrors fixed hardware/SDK capabilities into the SIG context -- see
	// raytracing.prism's own comment on RenderDeviceCapabilities for why.
	{
		auto& device_caps = graph.get_context<Table::Raytrace::RenderDeviceCapabilities>();
		device_caps.rtx_supported  = RenderSystem::get().device().is_rtx_supported();
		device_caps.dlss_available = nvidia::DLSS::get().available();
	}

	if (params.upscale)
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
	if (vp.frame_size != m_last_logged_frame_size || vp.upscale_size != m_last_logged_upscale_size)
	{
		Log::get() << "[Viewport] frame_size " << m_last_logged_frame_size.x << "x" << m_last_logged_frame_size.y
			<< " -> " << vp.frame_size.x << "x" << vp.frame_size.y
			<< ", upscale_size " << m_last_logged_upscale_size.x << "x" << m_last_logged_upscale_size.y
			<< " -> " << vp.upscale_size.x << "x" << vp.upscale_size.y
			<< ", dlss_mode=" << (int)g_upscaling_dlss_mode
			<< ", scale_override=" << g_upscaling_dlss_scale_override << Log::endl;
		m_last_logged_frame_size = vp.frame_size;
		m_last_logged_upscale_size = vp.upscale_size;
	}

	sceneinfo.scene    = m_scene;
	sceneinfo.renderer = m_gpu_renderer;
	caminfo.cam        = params.cam;
	timeinfo.time      = params.dt;
	timeinfo.totalTime += timeinfo.time;
	timeinfo.frame     = m_frame++;
	skyinfo.sunDir     = m_vsm.get_position();

	// Exactly once per frame — a second call anywhere else would desync
	// SL's internal frame counter.
	if (nvidia::Streamline::get().available())
		nvidia::Streamline::get().begin_frame();

	// Jitter only matters when something accumulates it temporally (DLSS)
	// AND DLSS is actually the pass running this frame — g_upscaling_enabled
	// off or a non-DLSS selection both mean nothing consumes the jitter, so
	// applying it would just add visible instability for no benefit. FSR1/
	// native have no such accumulation either way. No availability re-check:
	// g_upscaler_type can't hold an unavailable type (see its invariant,
	// UpscalingDLSS.ixx).
	vec2 jitter_px(0, 0);
	if (params.jitter && g_upscaling_enabled && g_upscaler_type != Post::Upscale::UpscalerType::FSR)
	{
		// Halton(2,3); phase count per NVIDIA's guidance: 8*(display/render)^2.
		const float scale_x = float(vp.upscale_size.x) / float(vp.frame_size.x);
		const float scale_y = float(vp.upscale_size.y) / float(vp.frame_size.y);
		const uint32_t phase_count = std::max<uint32_t>(1,
			static_cast<uint32_t>(8.0f * scale_x * scale_y + 0.5f));
		const uint32_t phase = static_cast<uint32_t>(timeinfo.frame % phase_count) + 1;

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
	params.cam->update(jitter_ndc);

	// Per-frame mirrors into the Table:: contexts the generated setups read,
	// plus the CPU work that used to live in those setups' bodies. All of it
	// needs an owning instance, which a generated static setup cannot reach.
	// Must run before graph.setup(), and before run_pre_setups() for the
	// same reason: nothing orders one pass's setup relative to another's.
	m_vsm.update_frame(graph);
	if (m_voxel_gi) m_voxel_gi->update_frame(graph);
	ddgi_update_selectors(graph);
	tonemap_update_selectors(graph);
	bloom_update_selectors(graph);

	m_lod_threshold = cluster_lod ? lod_error_pixels / std::max(float(vp.frame_size.y), 1.0f) : 0.0f;
	m_main_view_meshes->cluster_lod = m_lod_threshold > 0;
}

void SceneFrameRenderer::end_frame(Graph& graph)
{
	{
		PROFILE(L"graph");
		m_pipeline.add_passes(graph);
		// This pipeline's own [PreSetup] hooks: PreScene's
		// raytrace_scene->new_frame(), NRD_REBLUR_Execute's ensure_pools(),
		// CubeSky's sun-direction diff.
		m_pipeline.run_pre_setups(graph);
	}

	if (m_voxel_gi) m_voxel_gi->pass_data(graph.builder);

	m_vsm.pass_data(graph.builder);

	// Single-threaded allocation-planning pass for all VSM clip levels,
	// run once per frame here (via add_slot_generator's pre_run hook)
	// strictly before VSM_RenderPages' single render() is dispatched
	// to the thread pool -- see the LevelPlan comment in VSM.ixx for why
	// this can't safely happen inside render() itself.
	graph.add_slot_generator([this](Graph& graph)
		{
			m_vsm.plan_frame(graph);
		});

	graph.add_slot_generator([this](Graph& graph)
		{
			PROFILE(L"FrameInfo");
			auto& time    = graph.get_context<TimeInfo>();
			auto& skyinfo = graph.get_context<SkyInfo>();
			auto& cam     = graph.get_context<CameraInfo>();

			Slots::Frame::FrameInfo frameInfo;
			auto sky = graph.builder.get(FrameGraph::ResourceID::sky_cubemap_filtered);
			if (sky && sky->resource)
				frameInfo.GetSky() = *sky->get_handler<Handlers::TextureCube>();

			auto sky_diffuse = graph.builder.get(FrameGraph::ResourceID::sky_cubemap_filtered_diffuse);
			if (sky_diffuse && sky_diffuse->resource)
				frameInfo.GetSky_diffuse() = *sky_diffuse->get_handler<Handlers::TextureCube>();

			frameInfo.GetSunDir().xyz = skyinfo.sunDir;
			frameInfo.GetTime() = { time.time, time.totalTime, 0, 0 };

			frameInfo.GetCamera() = cam.cam->camera_cb.current;
			frameInfo.GetPrevCamera() = cam.cam->camera_cb.prev;

			frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d();
			frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d();

			// Material texture LOD bias — see FrameData.prism's mipBias comment.
			{
				auto& vp = graph.get_context<ViewportInfo>();
				frameInfo.GetMipBias() = m_params.upscale
					? (std::log2(float(vp.frame_size.x) / float(vp.upscale_size.x)) - 1.0f)
					: 0.0f;
			}

			// Hi-Z pyramid for per-meshlet occlusion; the PSO permutation
			// decides whether it is sampled (scene.prism's HiZOcclusion).
			{
				auto hiz = graph.builder.get(FrameGraph::ResourceID::GBuffer_HiZ_UAV);
				if (hiz && hiz->resource)
					frameInfo.GetMainHiZ() = hiz->get_handler<Handlers::Texture>()->texture2D;
			}

			// Dev::RTXDebugFlags bitmask -- see DDGI.ixx's ddgi_sky_fallback_disabled
			// comment for why this DDGI-owned toggle is mirrored here instead of
			// living on DDGIInfo (its effect isn't DDGI-exclusive).
			frameInfo.GetDebugFlags() = ddgi_sky_fallback_disabled() ? (uint32_t)Dev::RTXDebugFlags::DisableSkyFallback : 0u;
			frameInfo.GetLodThreshold() = m_lod_threshold;

			auto compiled = frameInfo.compile(*graph.builder.current_frame);
			graph.register_slot_setter(compiled);
		});

	graph.add_slot_generator([this](Graph& graph)
		{
			graph.register_slot_setter(m_scene->compiledScene);
		});
}
