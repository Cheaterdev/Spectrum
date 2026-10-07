export module Test.SceneParity;

#define TEST_MODULE_ID PARITY

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import RenderSystem;
import Graphics;
import FrameGraph;

// The app's own scene frame (SceneFrameRenderer: MainPipeline with VSM, sky,
// GI, post), rendered headless with every input fixed -- scene, camera, sun,
// frame time, no jitter, no upscaling -- and compared against goldens. The
// goldens are made on D3D12; on Vulkan the same tests check that both backends
// produce the same frame.

SETUP_CATEGORY(RenderSystem.SceneParity, []() {
	if (!GraphicsSystem::create())
	{
		Test::TestRegistry::Instance().SkipCategory("RenderSystem.SceneParity", "no suitable GPU device found");
		return;
	}
	// The frame shadows through VSM, whose atlas is a tiled depth texture.
	if (!RenderSystem::get().device().is_tiled_depth_supported())
		Test::TestRegistry::Instance().SkipCategory("RenderSystem.SceneParity", "no sparse depth textures for the VSM atlas");
});

TEARDOWN_CATEGORY(RenderSystem.SceneParity, []() {
	GraphicsSystem::reset();
});

namespace
{
	using namespace FrameGraph;
	using namespace Pipelines::FrameGraphTests;

	constexpr int   W = Test::kReferenceWidth, H = Test::kReferenceHeight;
	constexpr float kFrameTime = 1.0f / 60.0f;

	// The app's default scene (triangle_drawer): the material tester on a
	// plane, seen from a fixed camera under a fixed sun.
	struct SceneFrame
	{
		Scene::ptr scene = std::make_shared<Scene>();
		camera     cam;
		std::unique_ptr<SceneFrameRenderer> renderer;
		Graph      graph;
		int        frame = 0;

		// Runs every frame after begin_frame, so it overrides the selectors
		// begin_frame mirrors from the subsystems -- picks the configuration.
		std::function<void(Graph&)> configure;

		SceneFrame()
		{
			scene->add_child(std::make_shared<MeshAssetInstance>(EngineAssets::material_tester.get_asset()));
			scene->add_child(std::make_shared<MeshAssetInstance>(EngineAssets::plane.get_asset()));

			renderer = std::make_unique<SceneFrameRenderer>(scene);
			renderer->set_sun_direction(vec3(0.35f, 1.0f, 0.25f).normalize());

			cam.position = vec3(30, 10, 30).normalize() * 10.0f;
			cam.target   = vec3(0, 0, 0);
			cam.set_projection_params(Math::pi / 4, Test::kReferenceAspect, 1.0f, 1500.0f);
		}

		// One frame of the scene, with `capture`'s passes after MainPipeline.
		template<class... CapturePipelines>
		void run(CapturePipelines&... captures)
		{
			graph.start_new_frame();
			scene->update_transforms();
			if (RenderSystem::get().device().is_rtx_supported())
				RTX::get().update();

			SceneFrameParams params;
			params.cam         = &cam;
			params.dt          = kFrameTime;
			params.output_size = { W, H };
			params.upscale     = false;
			params.jitter      = false;
			renderer->begin_frame(graph, params);
			if (configure)
				configure(graph);
			renderer->end_frame(graph);
			(captures.add_passes(graph), ...);

			graph.setup();
			graph.compile(frame++);
			graph.render();
			graph.commit_command_lists().wait();
			graph.reset();

			if (RenderSystem::get().device().is_lost())
				throw Test::TestFailure("device lost at scene frame " + std::to_string(frame));
		}

		bool materials_ready() const
		{
			for (auto& node : scene->get_childs())
				if (auto instance = std::dynamic_pointer_cast<MeshAssetInstance>(node))
					for (auto& info : instance->rendering)
						if (info.material && !info.material->is_ready_to_draw())
							return false;
			return true;
		}

		// Frames until every material's pipelines are built: they compile in
		// the background and their draws are skipped until then.
		void wait_for_materials()
		{
			FGT_RasterCapturePipeline no_capture;   // no render_func set: adds no passes
			const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(120);
			do
				run(no_capture);
			while (!materials_ready() && std::chrono::steady_clock::now() < deadline);
			if (!materials_ready())
				throw Test::TestFailure("scene materials were not ready after 120 s");
		}
	};

	// How many frames the compile wait takes varies from run to run, and the
	// temporal state a frame carries (auto exposure, denoiser history, DDGI
	// probes) depends on how many came before it. So the wait happens on a
	// throwaway renderer and graph; the frame under test then starts fresh and
	// runs a fixed count. The compiled pipelines are global and carry over.
	constexpr int kSettleFrames = 32;

	void warm_up(const std::function<void(Graph&)>& configure)
	{
		SceneFrame warm;
		warm.configure = configure;
		warm.wait_for_materials();
	}

	std::shared_ptr<HAL::TextureResource> as_texture(const auto& handler)
	{
		return std::dynamic_pointer_cast<HAL::TextureResource>(handler.info->resource);
	}

	// Compares every capture before failing, so one run shows each stage
	// that differs -- a GBuffer mismatch alone points at the scene pass, one
	// only in the result at lighting or post.
	struct Captures
	{
		std::string failures;

		void check(const std::shared_ptr<HAL::TextureResource>& tex, const std::string& name, const Test::TextureCompare& compare = {})
		{
			if (!tex)
			{
				failures += "\n       '" + name + "' was not captured";
				return;
			}
			try { Test::check_texture_reference(tex.get(), name, compare); }
			catch (const Test::TestFailure& e) { failures += std::string("\n       ") + e.what(); }
		}

		void finish() const
		{
			if (!failures.empty())
				throw Test::TestFailure("scene captures differ:" + failures);
		}
	};

	// Allowed mismatch per stage, about twice what Vulkan shows against the
	// D3D12 goldens (NVIDIA, 2026-10-07). DXIL and SPIR-V don't compile float
	// math identically, so single pixels drift by up to ~20/255; a real bug
	// is structural (the reflection miss bug was 19% of the raw reflections).
	// The lit stages also vary run to run on one backend (up to ~80 pixels).
	// Goldens assume exact trilinear: NVIDIA's D3D12 driver needs "Texture
	// filtering - Quality" = High quality, or it blends mips approximately.
	const Test::TextureCompare kGBufferCompare  = { .max_mismatch_fraction = 0.00002 };
	const Test::TextureCompare kTracedCompare   = { .max_mismatch_fraction = 0.005, .float_encoding = Test::FloatEncoding::Linear };
	const Test::TextureCompare kDenoisedCompare = { .max_mismatch_fraction = 0.003, .float_encoding = Test::FloatEncoding::Linear };
	const Test::TextureCompare kResultCompare   = { .max_mismatch_fraction = 0.001, .float_encoding = Test::FloatEncoding::Linear };

	// Renders the scene in the configuration `configure` sets for
	// kSettleFrames frames and checks the last one against the goldens
	// parity_<name>_*: GBuffer albedo/normals and the final result, and with
	// `rt_stages` the ray-traced lighting before and after NRD -- set apart,
	// they show which stage a final-result mismatch starts in.
	void check_frame(const std::string& name, const std::function<void(Graph&)>& configure = {}, bool rt_stages = false)
	{
		warm_up(configure);

		SceneFrame frame;
		frame.configure = configure;
		if (!frame.materials_ready())
			throw Test::TestFailure("scene materials not ready after the warm-up");

		std::shared_ptr<HAL::TextureResource> albedo, normals, result;
		FGT_RasterCapturePipeline capture;
		capture.fGT_CaptureAlbedo.render_func  = [&](auto& data, FrameContext&) { albedo  = as_texture(data.GBuffer_Albedo); };
		capture.fGT_CaptureNormals.render_func = [&](auto& data, FrameContext&) { normals = as_texture(data.GBuffer_Normals); };
		capture.fGT_CaptureResult.render_func  = [&](auto& data, FrameContext&) { result  = as_texture(data.ResultTexture); };

		std::shared_ptr<HAL::TextureResource> indirect_noise, reflection_noise, indirect_denoised, reflection_denoised;
		std::shared_ptr<HAL::TextureResource> reflection_noise_half, reflection_dir_pdf_half;
		FGT_RTCapturePipeline rt;
		if (rt_stages)
		{
			rt.fGT_CaptureRTXReflectionNoiseHalf.render_func  = [&](auto& data, FrameContext&) { reflection_noise_half   = as_texture(data.RTXReflectionNoiseHalf); };
			rt.fGT_CaptureRTXReflectionDirPdfHalf.render_func = [&](auto& data, FrameContext&) { reflection_dir_pdf_half = as_texture(data.RTXReflectionDirPdfHalf); };
			rt.fGT_CaptureRTXIndirectNoise.render_func      = [&](auto& data, FrameContext&) { indirect_noise      = as_texture(data.RTXIndirectNoise); };
			rt.fGT_CaptureRTXReflectionNoise.render_func    = [&](auto& data, FrameContext&) { reflection_noise    = as_texture(data.RTXReflectionNoise); };
			rt.fGT_CaptureRTXIndirectDenoised.render_func   = [&](auto& data, FrameContext&) { indirect_denoised   = as_texture(data.RTXIndirectDenoised); };
			rt.fGT_CaptureRTXReflectionDenoised.render_func = [&](auto& data, FrameContext&) { reflection_denoised = as_texture(data.RTXReflectionDenoised); };
		}

		for (int i = 0; i < kSettleFrames; ++i)
			frame.run(capture, rt);

		Captures checks;
		checks.check(albedo,  "parity_" + name + "_albedo",  kGBufferCompare);
		checks.check(normals, "parity_" + name + "_normals", kGBufferCompare);
		if (rt_stages)
		{
			checks.check(reflection_noise_half,   "parity_" + name + "_rtx_reflection_noise_half",   kTracedCompare);
			checks.check(reflection_dir_pdf_half, "parity_" + name + "_rtx_reflection_dir_pdf_half", kTracedCompare);
			checks.check(indirect_noise,      "parity_" + name + "_rtx_indirect_noise",      kTracedCompare);
			checks.check(reflection_noise,    "parity_" + name + "_rtx_reflection_noise",    kTracedCompare);
			checks.check(indirect_denoised,   "parity_" + name + "_rtx_indirect_denoised",   kDenoisedCompare);
			checks.check(reflection_denoised, "parity_" + name + "_rtx_reflection_denoised", kDenoisedCompare);
		}
		checks.check(result,  "parity_" + name + "_result", kResultCompare);
		checks.finish();
	}
}

export namespace Test
{
	// The default frame as the app renders it: GBuffer, VSM shadows, sky, the
	// default GI and reflection sources (ray traced where available), DDGI,
	// bloom and tonemap.
	TEST(RenderSystem.SceneParity, DefaultFrame)
	{
		check_frame("default", {}, RenderSystem::get().device().is_rtx_supported());
	}

	// No ray-traced lighting: voxel cone traced indirect, screen-space
	// reflections, DDGI off. Set apart from DefaultFrame, it tells a lighting
	// mismatch in the ray-traced and denoised path from one in the rest.
	TEST(RenderSystem.SceneParity, NoRayTracedLighting)
	{
		check_frame("no_rt_lighting", [](Graph& g) {
			auto& gi = g.get_context<Table::Denoise::NRD::IndirectGISelectors>();
			gi.indirect_source   = GI::IndirectSource::MyVCT;
			gi.reflection_source = Reflections::ReflectionSource::MyReflection;
			g.get_context<Table::GI::DDGI::DDGISelectors>().enabled = false;
		});
	}
}
