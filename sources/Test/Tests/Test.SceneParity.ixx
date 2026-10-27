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

	// Runs frames until the Quanta streamer has nothing left to do: new feedback keeps arriving, none of it
	// wants a page, nothing is loading and nothing was loaded since. False if that never happens.
	bool settle_streaming(SceneFrame& frame, FGT_RasterCapturePipeline& capture, int max_frames = 1500)
	{
		int calm = 0;
		Quanta::StreamStats last = Quanta::Residency::get().stats();
		for (int i = 0; i < max_frames && calm < 20; ++i)
		{
			frame.run(capture);
			auto stats = Quanta::Residency::get().stats();
			if (stats.snapshots > last.snapshots)
				calm = (stats.wanted == 0 && stats.loading_pages == 0 && stats.loads == last.loads) ? calm + 1 : 0;
			last = stats;
		}
		return calm >= 20;
	}

	// The fraction of the albedo (as a count of bytes) that differs, and how many bytes are not the corner value.
	struct PictureDiff
	{
		size_t differing = 0;
		size_t covered = 0;
		size_t size = 0;
	};

	PictureDiff compare_pictures(const HAL::texture_data::ptr& a, const HAL::texture_data::ptr& b)
	{
		const auto& x = a->array[0]->mips[0]->data;
		const auto& y = b->array[0]->mips[0]->data;
		PictureDiff out;
		out.size = std::min(x.size(), y.size());
		for (size_t i = 0; i < out.size; ++i)
		{
			out.differing += x[i] != y[i];
			out.covered += x[i] != x[i % 4];
		}
		return out;
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

	// Quanta's debug splat (quanta.prism) draws the LOD cut of a freshly imported mesh into the frame. It is
	// off by default, so the goldens above are unaffected; this only checks it does something and that the
	// frame stays valid (the D3D12 debug layer reports into log.txt). What it should look like is judged by eye.
	TEST(RenderSystem.SceneParity, QuantaSplatDrawsTheCut)
	{
		// A fresh import carries Quanta data. Created and registered the way the MaterialX tests do (a raw
		// `new`, left to the asset manager): its storage points at the asset without owning it, so a
		// make_shared asset freed at the end of the test leaves the storage dangling for the teardown.
		auto* raw = new MeshAsset(L"resources/sphere.obj");
		raw->register_new(L"quanta_splat_test");   // an instance reads its asset's name, which that storage holds
		auto mesh = raw->get_ptr<MeshAsset>();
		ASSERT_FALSE(mesh->quanta.empty());

		// A fresh scene per render and a fixed frame count, like check_frame: the temporal state (denoisers,
		// exposure) makes anything else drift from frame to frame by far more than the splat changes.
		auto make_scene = [&](SceneFrame& frame) {
			auto instance = std::make_shared<MeshAssetInstance>(mesh);
			mat4x4 place = mat4x4::translation(vec3(3, 1, 3));   // between the camera and the material tester
			instance->set_local_transform(place);
			frame.scene->add_child(instance);
		};

		{
			SceneFrame warm;
			make_scene(warm);
			warm.wait_for_materials();
		}

		auto render = [&](bool splat) {
			SceneFrame frame;
			make_scene(frame);

			std::shared_ptr<HAL::TextureResource> result;
			FGT_RasterCapturePipeline capture;
			capture.fGT_CaptureResult.render_func = [&](auto& data, FrameContext&) { result = as_texture(data.ResultTexture); };

			Quanta::set_splat_enabled(splat);
			for (int i = 0; i < kSettleFrames; ++i)
				frame.run(capture);
			return Test::readback_texture(result.get());
		};
		auto differing = [](const HAL::texture_data::ptr& a, const HAL::texture_data::ptr& b) {
			const auto& x = a->array[0]->mips[0]->data;
			const auto& y = b->array[0]->mips[0]->data;
			size_t count = 0;
			for (size_t i = 0; i < std::min(x.size(), y.size()); ++i)
				count += x[i] != y[i];
			return count;
		};

		auto off = render(false);
		auto splat = render(true);
		auto off_again = render(false);
		Quanta::set_splat_enabled(false);

		// Frame-to-frame noise with the splat off sets the floor the splat has to clear.
		const size_t noise = differing(off, off_again);
		const size_t drawn = differing(splat, off_again);
		if (drawn < noise + 500)
			throw Test::TestFailure("Quanta splat changed " + std::to_string(drawn) + " bytes against a noise floor of " + std::to_string(noise));
	}

	// Quanta's draw (QUANTA permutations of the GBuffer mesh shader) must put the same picture in the GBuffer as
	// the mesh's own meshlets: the clusters are the same and so is the LOD cut, only the data source differs.
	// The GBuffer is not temporal, so the two renders agree to the byte apart from ties in the depth test.
	TEST(RenderSystem.SceneParity, QuantaDrawMatchesTheMeshlets)
	{
		auto* raw = new MeshAsset(L"resources/sphere.obj");   // see QuantaSplatDrawsTheCut for why a raw `new`
		raw->register_new(L"quanta_draw_test");
		auto mesh = raw->get_ptr<MeshAsset>();
		ASSERT_FALSE(mesh->quanta.empty());

		auto make_scene = [&](SceneFrame& frame) {
			auto instance = std::make_shared<MeshAssetInstance>(mesh);
			mat4x4 place = mat4x4::translation(vec3(3, 1, 3));
			instance->set_local_transform(place);
			frame.scene->add_child(instance);
		};

		{
			SceneFrame warm;
			make_scene(warm);
			warm.wait_for_materials();
			Quanta::set_draw_enabled(true);   // builds the QUANTA permutations of the pipelines too
			warm.wait_for_materials();
			Quanta::set_draw_enabled(false);
		}

		auto render = [&](bool quanta) {
			SceneFrame frame;
			make_scene(frame);

			std::shared_ptr<HAL::TextureResource> albedo;
			FGT_RasterCapturePipeline capture;
			capture.fGT_CaptureAlbedo.render_func = [&](auto& data, FrameContext&) { albedo = as_texture(data.GBuffer_Albedo); };

			Quanta::set_draw_enabled(quanta);
			for (int i = 0; i < kSettleFrames; ++i)
				frame.run(capture);
			return Test::readback_texture(albedo.get());
		};

		auto meshlets = render(false);
		auto quanta = render(true);
		Quanta::set_draw_enabled(false);

		const auto& x = meshlets->array[0]->mips[0]->data;
		const auto& y = quanta->array[0]->mips[0]->data;
		ASSERT_EQ(x.size(), y.size());
		size_t differing = 0, covered = 0;
		for (size_t i = 0; i < x.size(); ++i)
		{
			differing += x[i] != y[i];
			covered += x[i] != x[i % 4];   // not the corner pixel's value: something is drawn there
		}
		// Equal frames prove nothing if the mesh is out of view in both.
		if (covered < x.size() / 100)
			throw Test::TestFailure("the mesh covers almost nothing of the GBuffer albedo (" + std::to_string(covered) + " bytes)");
		if (differing > x.size() / 5000)
			throw Test::TestFailure("Quanta draw's GBuffer albedo differs from the meshlets' in " + std::to_string(differing) + " of " + std::to_string(x.size()) + " bytes");
	}

	// Streaming's safety net: with every page but the roots hidden from the page table, the Quanta draw must still
	// cover the surface (coarser clusters stand in for the missing finer ones) and must say which pages it wanted.
	// With everything resident it must want nothing.
	TEST(RenderSystem.SceneParity, QuantaDrawKeepsTheSurfaceWithoutFinePages)
	{
		auto* raw = new MeshAsset(L"resources/sphere.obj");   // see QuantaSplatDrawsTheCut for why a raw `new`
		raw->register_new(L"quanta_missing_pages_test");
		auto mesh = raw->get_ptr<MeshAsset>();
		ASSERT_FALSE(mesh->quanta.empty());

		auto make_scene = [&](SceneFrame& frame) {
			auto instance = std::make_shared<MeshAssetInstance>(mesh);
			mat4x4 place = mat4x4::translation(vec3(3, 1, 3));
			instance->set_local_transform(place);
			frame.scene->add_child(instance);
		};

		{
			SceneFrame warm;
			make_scene(warm);
			warm.wait_for_materials();
			Quanta::set_draw_enabled(true);
			warm.wait_for_materials();
			Quanta::set_draw_enabled(false);
		}

		struct Result
		{
			size_t covered = 0;
			size_t wanted = 0;
			size_t touched = 0;
			size_t hidden = 0;
		};

		auto render = [&](bool hide) {
			SceneFrame frame;
			make_scene(frame);

			auto& residency = Quanta::Residency::get();
			std::vector<std::shared_ptr<Quanta::Resident>> residents;
			Result out;
			for (auto& quanta : mesh->quanta)
			{
				if (!quanta)
					continue;
				residents.push_back(residency.acquire(quanta));
				for (uint32_t p = 0; hide && p < quanta->pages.size(); p++)
					if (!quanta->pages[p].root)
					{
						residency.set_page_present(*residents.back(), p, false);
						out.hidden++;
					}
			}

			std::shared_ptr<HAL::TextureResource> albedo;
			FGT_RasterCapturePipeline capture;
			capture.fGT_CaptureAlbedo.render_func = [&](auto& data, FrameContext&) { albedo = as_texture(data.GBuffer_Albedo); };

			Quanta::set_draw_enabled(true);
			for (int i = 0; i < kSettleFrames; ++i)
				frame.run(capture);
			Quanta::set_draw_enabled(false);

			auto picture = Test::readback_texture(albedo.get());
			const auto& x = picture->array[0]->mips[0]->data;
			for (size_t i = 0; i < x.size(); ++i)
				out.covered += x[i] != x[i % 4];

			// The feedback of the last frame, for the mesh's page ids.
			const uint32_t epoch = residency.epoch();
			auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
			list->begin(L"QuantaFeedbackReadback");
			std::vector<std::future<bool>> pending;
			std::vector<std::vector<uint>> feedback(residents.size());
			for (size_t r = 0; r < residents.size(); r++)
				pending.push_back(list->get_copy().read<uint>(residency.feedback, residents[r]->page_id_base() * 2,
					residents[r]->mesh->pages.size() * 2, [&, r](std::span<uint> s) { feedback[r].assign(s.begin(), s.end()); }));
			list->execute_and_wait();
			for (auto& p : pending)
				p.wait();

			for (auto& page_feedback : feedback)
				for (size_t p = 0; p + 1 < page_feedback.size(); p += 2)
				{
					// The last frame or the one before it: the final frame's draws may not have run yet when the
					// readback list is queued, and the page set is the same in both.
					const uint32_t stamp = page_feedback[p] >> 16;
					out.wanted += page_feedback[p] != 0 && (stamp == (epoch & 0xFFFF) || stamp == ((epoch - 1) & 0xFFFF));
					out.touched += page_feedback[p + 1] == epoch || page_feedback[p + 1] == epoch - 1;
				}
			return out;
		};

		const Result full = render(false);
		const Result coarse = render(true);

		ASSERT_TRUE(coarse.hidden > 0);   // otherwise the sphere fits in its root pages and nothing is tested
		if (full.covered < 1000)
			throw Test::TestFailure("the mesh covers almost nothing of the GBuffer albedo (" + std::to_string(full.covered) + " bytes)");
		if (coarse.covered * 100 < full.covered * 90)
			throw Test::TestFailure("with the fine pages missing the mesh covers " + std::to_string(coarse.covered) + " bytes against " + std::to_string(full.covered));
		ASSERT_EQ(size_t(0), full.wanted);
		ASSERT_TRUE(full.touched > 0);
		if (coarse.wanted == 0)
			throw Test::TestFailure("no page was wanted with " + std::to_string(coarse.hidden) + " hidden; touched " + std::to_string(coarse.touched)
				+ ", covered " + std::to_string(coarse.covered) + " against " + std::to_string(full.covered) + " (wanted with all resident: " + std::to_string(full.wanted) + ")");
	}

	// Streaming: a mesh held by its root pages alone starts out coarse, the draw asks for the pages it lacks, the
	// streamer loads them, and once nothing is wanted any more the picture is the all-resident one.
	TEST(RenderSystem.SceneParity, QuantaStreamingReachesTheResidentPicture)
	{
		auto* raw = new MeshAsset(L"resources/sphere.obj");   // see QuantaSplatDrawsTheCut for why a raw `new`
		raw->register_new(L"quanta_streaming_test");
		auto mesh = raw->get_ptr<MeshAsset>();
		ASSERT_FALSE(mesh->quanta.empty());

		auto make_scene = [&](SceneFrame& frame) {
			auto instance = std::make_shared<MeshAssetInstance>(mesh);
			mat4x4 place = mat4x4::translation(vec3(3, 1, 3));
			instance->set_local_transform(place);
			frame.scene->add_child(instance);
		};

		{
			SceneFrame warm;
			make_scene(warm);
			warm.wait_for_materials();
			Quanta::set_draw_enabled(true);
			warm.wait_for_materials();
			Quanta::set_draw_enabled(false);
		}

		struct Result
		{
			HAL::texture_data::ptr albedo;
			Quanta::StreamStats stats;
			bool settled = false;
		};

		auto render = [&](bool streamed) {
			Quanta::set_streaming(streamed, 512);
			Result out;
			{
				SceneFrame frame;
				make_scene(frame);

				std::shared_ptr<HAL::TextureResource> albedo;
				FGT_RasterCapturePipeline capture;
				capture.fGT_CaptureAlbedo.render_func = [&](auto& data, FrameContext&) { albedo = as_texture(data.GBuffer_Albedo); };

				Quanta::set_draw_enabled(true);
				if (!streamed)
				{
					for (int i = 0; i < kSettleFrames; ++i)
						frame.run(capture);
				}
				else
				{
					// Settled: new snapshots keep arriving and none wants a page, loads nothing and waits on nothing.
					int calm = 0;
					Quanta::StreamStats last;
					for (int i = 0; i < 1000 && calm < 20; ++i)
					{
						frame.run(capture);
						auto stats = Quanta::Residency::get().stats();
						if (stats.snapshots > last.snapshots)
							calm = (stats.wanted == 0 && stats.loading_pages == 0 && stats.loads == last.loads) ? calm + 1 : 0;
						last = stats;
					}
					out.settled = calm >= 20;
				}
				Quanta::set_draw_enabled(false);

				out.albedo = Test::readback_texture(albedo.get());
				out.stats = Quanta::Residency::get().stats();
			}
			Quanta::set_streaming(false);
			return out;
		};

		const Result resident = render(false);
		const Result streamed = render(true);

		if (!streamed.settled)
			throw Test::TestFailure("streaming never settled: " + std::to_string(streamed.stats.loads) + " loads, " + std::to_string(streamed.stats.wanted) + " pages still wanted");
		ASSERT_TRUE(streamed.stats.loads > 0);   // otherwise the root pages were enough and nothing was streamed

		const auto& x = resident.albedo->array[0]->mips[0]->data;
		const auto& y = streamed.albedo->array[0]->mips[0]->data;
		ASSERT_EQ(x.size(), y.size());
		size_t differing = 0, covered = 0;
		for (size_t i = 0; i < x.size(); ++i)
		{
			differing += x[i] != y[i];
			covered += x[i] != x[i % 4];
		}
		if (covered < x.size() / 100)
			throw Test::TestFailure("the mesh covers almost nothing of the GBuffer albedo (" + std::to_string(covered) + " bytes)");
		if (differing > x.size() / 5000)
			throw Test::TestFailure("the streamed picture differs from the resident one in " + std::to_string(differing) + " of " + std::to_string(x.size()) + " bytes after " + std::to_string(streamed.stats.loads) + " loads");
	}

	// The same, with the pages read from the asset file by DirectStorage straight into the pool.
	TEST(RenderSystem.SceneParity, QuantaStreamingReadsPagesFromTheAssetFile)
	{
		const std::filesystem::path path = std::filesystem::temp_directory_path() / "quanta_streaming_asset_test.bin";

		auto* raw = new MeshAsset(L"resources/sphere.obj");   // see QuantaSplatDrawsTheCut for why a raw `new`
		raw->register_new(L"quanta_streaming_file_test");
		auto mesh = raw->get_ptr<MeshAsset>();
		ASSERT_FALSE(mesh->quanta.empty());
		{
			FileDataStorage storage(path);
			storage.start_save();
			mesh->save_partitions(storage);
			storage.save();
		}
		auto resident_quanta = mesh->quanta;   // the in-memory ones, for the reference picture
		mesh->quanta.clear();
		{
			FileDataStorage storage(path);
			mesh->load_partitions(storage, nullptr);
		}
		ASSERT_EQ(resident_quanta.size(), mesh->quanta.size());
		for (auto& quanta : mesh->quanta)
			ASSERT_FALSE(quanta->file.empty());   // pages carry their offsets in this file

		auto make_scene = [&](SceneFrame& frame) {
			auto instance = std::make_shared<MeshAssetInstance>(mesh);
			mat4x4 place = mat4x4::translation(vec3(3, 1, 3));
			instance->set_local_transform(place);
			frame.scene->add_child(instance);
		};

		{
			SceneFrame warm;
			make_scene(warm);
			warm.wait_for_materials();
			Quanta::set_draw_enabled(true);
			warm.wait_for_materials();
			Quanta::set_draw_enabled(false);
		}

		auto render = [&](bool streamed, Quanta::StreamStats* stats, bool* settled) {
			Quanta::set_streaming(streamed, 512);
			HAL::texture_data::ptr picture;
			{
				SceneFrame frame;
				make_scene(frame);

				std::shared_ptr<HAL::TextureResource> albedo;
				FGT_RasterCapturePipeline capture;
				capture.fGT_CaptureAlbedo.render_func = [&](auto& data, FrameContext&) { albedo = as_texture(data.GBuffer_Albedo); };

				Quanta::set_draw_enabled(true);
				if (streamed)
					*settled = settle_streaming(frame, capture);
				else
					for (int i = 0; i < kSettleFrames; ++i)
						frame.run(capture);
				Quanta::set_draw_enabled(false);

				picture = Test::readback_texture(albedo.get());
				*stats = Quanta::Residency::get().stats();
			}
			Quanta::set_streaming(false);
			return picture;
		};

		Quanta::StreamStats resident_stats, streamed_stats;
		bool settled = true;
		auto resident = render(false, &resident_stats, &settled);
		auto streamed = render(true, &streamed_stats, &settled);
		std::filesystem::remove(path);

		Log::get() << "Quanta streaming from file: " << streamed_stats.loads << " loads, " << streamed_stats.file_loads
			<< " from the file, pool " << streamed_stats.pool_pages << Log::endl;

		if (!settled)
			throw Test::TestFailure("streaming never settled: " + std::to_string(streamed_stats.loads) + " loads, " + std::to_string(streamed_stats.wanted) + " pages still wanted");
		// Pages wanted before the pool's mapping has run are copied from memory instead, so not all of them.
		ASSERT_TRUE(streamed_stats.file_loads > 0);

		PictureDiff diff = compare_pictures(resident, streamed);
		if (diff.covered < diff.size / 100)
			throw Test::TestFailure("the mesh covers almost nothing of the GBuffer albedo (" + std::to_string(diff.covered) + " bytes)");
		if (diff.differing > diff.size / 5000)
			throw Test::TestFailure("the picture from DirectStorage pages differs from the resident one in " + std::to_string(diff.differing) + " of " + std::to_string(diff.size) + " bytes");
	}

	// Two meshes share a pool that holds only one's detail: when the view moves from one to the other the first
	// one's pages are evicted for the second's, and the second ends up drawn like it would be resident.
	TEST(RenderSystem.SceneParity, QuantaStreamingEvictsWhatTheViewLeft)
	{
		auto* raw_a = new MeshAsset(L"resources/sphere.obj");   // see QuantaSplatDrawsTheCut for why a raw `new`
		raw_a->register_new(L"quanta_evict_a");
		auto* raw_b = new MeshAsset(L"resources/sphere.obj");
		raw_b->register_new(L"quanta_evict_b");
		auto mesh_a = raw_a->get_ptr<MeshAsset>();
		auto mesh_b = raw_b->get_ptr<MeshAsset>();
		ASSERT_FALSE(mesh_a->quanta.empty());
		ASSERT_FALSE(mesh_b->quanta.empty());

		uint32_t roots = 0, total = 0;
		for (auto& quanta : mesh_a->quanta)
		{
			roots += quanta->root_page_count;
			total += static_cast<uint32_t>(quanta->pages.size());
		}
		ASSERT_TRUE(total > roots);

		const vec3 where(3, 1, 3);

		{
			SceneFrame warm;
			warm.scene->add_child(std::make_shared<MeshAssetInstance>(mesh_a));
			warm.wait_for_materials();
			Quanta::set_draw_enabled(true);
			warm.wait_for_materials();
			Quanta::set_draw_enabled(false);
		}

		// What one near sphere needs on top of the roots, measured with room to spare.
		uint64_t detail_pages = 0;
		{
			Quanta::set_streaming(true, 512);
			SceneFrame frame;
			auto instance = std::make_shared<MeshAssetInstance>(mesh_a);
			mat4x4 near_place = mat4x4::translation(where);
			instance->set_local_transform(near_place);
			frame.scene->add_child(instance);
			FGT_RasterCapturePipeline capture;
			capture.fGT_CaptureAlbedo.render_func = [](auto&, FrameContext&) {};
			const uint64_t loads_before = Quanta::Residency::get().stats().loads;
			Quanta::set_draw_enabled(true);
			ASSERT_TRUE(settle_streaming(frame, capture));
			Quanta::set_draw_enabled(false);
			detail_pages = Quanta::Residency::get().stats().loads - loads_before;
			Quanta::set_streaming(false);
		}
		ASSERT_TRUE(detail_pages > 0);

		// Roots of both and the detail of one, nothing to spare: the other's detail only fits by evicting.
		Quanta::set_streaming(true, 2 * roots + static_cast<uint32_t>(detail_pages));

		HAL::texture_data::ptr last;
		Quanta::StreamStats stats;
		{
			SceneFrame frame;
			auto a = std::make_shared<MeshAssetInstance>(mesh_a);
			auto b = std::make_shared<MeshAssetInstance>(mesh_b);
			mat4x4 near_place = mat4x4::translation(where);
			mat4x4 far_place = mat4x4::translation(where);
			far_place.scaling(0.001f);   // a speck inside the other sphere's place
			a->set_local_transform(near_place);
			b->set_local_transform(far_place);
			frame.scene->add_child(a);
			frame.scene->add_child(b);

			std::shared_ptr<HAL::TextureResource> albedo;
			FGT_RasterCapturePipeline capture;
			capture.fGT_CaptureAlbedo.render_func = [&](auto& data, FrameContext&) { albedo = as_texture(data.GBuffer_Albedo); };

			Quanta::set_draw_enabled(true);
			ASSERT_TRUE(settle_streaming(frame, capture));
			const uint64_t evictions_before = Quanta::Residency::get().stats().evictions;

			// The view moves to the other mesh.
			a->set_local_transform(far_place);
			b->set_local_transform(near_place);
			const bool settled = settle_streaming(frame, capture);
			Quanta::set_draw_enabled(false);

			stats = Quanta::Residency::get().stats();
			last = Test::readback_texture(albedo.get());
			if (!settled)
				throw Test::TestFailure("streaming never settled after the move: " + std::to_string(stats.wanted) + " pages still wanted, " + std::to_string(stats.evictions) + " evictions");
			if (stats.evictions <= evictions_before)
				throw Test::TestFailure("the second mesh's detail fitted without evicting anything (pool " + std::to_string(stats.pool_pages)
					+ " pages, " + std::to_string(stats.free_pages) + " free, roots " + std::to_string(roots) + ", detail " + std::to_string(detail_pages)
					+ ", loads " + std::to_string(stats.loads) + ", evictions " + std::to_string(stats.evictions) + ")");
		}
		Quanta::set_streaming(false);

		// The reference: the same sphere alone, everything resident.
		HAL::texture_data::ptr reference;
		{
			SceneFrame frame;
			auto instance = std::make_shared<MeshAssetInstance>(mesh_b);
			mat4x4 near_place = mat4x4::translation(where);
			instance->set_local_transform(near_place);
			frame.scene->add_child(instance);

			std::shared_ptr<HAL::TextureResource> albedo;
			FGT_RasterCapturePipeline capture;
			capture.fGT_CaptureAlbedo.render_func = [&](auto& data, FrameContext&) { albedo = as_texture(data.GBuffer_Albedo); };
			Quanta::set_draw_enabled(true);
			for (int i = 0; i < kSettleFrames; ++i)
				frame.run(capture);
			Quanta::set_draw_enabled(false);
			reference = Test::readback_texture(albedo.get());
		}

		PictureDiff diff = compare_pictures(reference, last);
		if (diff.covered < diff.size / 100)
			throw Test::TestFailure("the mesh covers almost nothing of the GBuffer albedo (" + std::to_string(diff.covered) + " bytes)");
		if (diff.differing > diff.size / 5000)
			throw Test::TestFailure("after the eviction the picture differs from the resident one in " + std::to_string(diff.differing) + " of " + std::to_string(diff.size) + " bytes");
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
