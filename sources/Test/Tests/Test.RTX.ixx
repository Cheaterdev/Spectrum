export module Test.RTX;

#define TEST_MODULE_ID RTX

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import RenderSystem;
import Graphics;
import FrameGraph;

SETUP_CATEGORY(Core.RTX, []() {
	if (!GraphicsSystem::create())
	{
		Test::TestRegistry::Instance().SkipCategory("Core.RTX", "no suitable GPU device found");
		return;
	}
	if (!RenderSystem::get().device().is_rtx_supported())
		Test::TestRegistry::Instance().SkipCategory("Core.RTX", "RTX not supported on this device");
});

TEARDOWN_CATEGORY(Core.RTX, []() {
	GraphicsSystem::reset();
});

export namespace Test
{
	// Shoots a primary ray per pixel from the camera against the mitsuba-sphere
	// (material_tester) with a gray metallic material, then traces a shadow ray
	// from each hit point toward the sun.  Writes the Lambertian color result
	// directly to the output UAV — no GBuffer, no PSSM, no FSR.
	TEST(Core.RTX, MaterialTester)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint W = 256, H = 256;

		// Single-mip UAV+SRV texture: RTX writes here directly.
		auto output = std::make_shared<HAL::Texture>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {W, H}, 1, 1,
				HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess));

		// Scene: material_tester sphere with the custom material.
		auto mesh_inst = std::make_shared<MeshAssetInstance>(EngineAssets::material_tester.get_asset());
			auto plane_inst = std::make_shared<MeshAssetInstance>(EngineAssets::plane.get_asset());
//		mesh_inst->override_material(1, mat);

		auto scene = std::make_shared<Scene>();
		scene->add_child(mesh_inst);
		scene->add_child(plane_inst);


		// Camera looking at the sphere from the same angle AssetRenderer uses.
		camera cam;
		cam.position = vec3(30, 10, 30).normalize() * 10.0f;
		cam.target   = vec3(0, 0, 0);
		cam.set_projection_params(Math::pi / 4, 1.0f, 0.5f, 200.0f);
		cam.update();

		// FrameGraph setup.
		FrameGraph::Graph graph;
		UINT frame_idx = 0;

		{
			auto& vp = graph.get_context<ViewportInfo>();
			vp.frame_size   = {W, H};
			vp.upscale_size = {W, H};
		}
		{
			auto& si = graph.get_context<SceneInfo>();
			si.scene    = scene;
			si.renderer = nullptr;
		}
		graph.get_context<CameraInfo>().cam = &cam;
		graph.get_context<SkyInfo>().sunDir = float3(-1, 1, 1).normalize();
		graph.get_context<TimeInfo>() = {};  // time = 0


	
		graph.start_new_frame();
		scene->update_transforms();
		scene->update(*graph.builder.current_frame);

		// Register the output texture as the FrameGraph resource "ColorOutput".
		graph.builder.pass_texture(FrameGraph::ResourceID::ColorOutput, output->resource,
			{}, FrameGraph::ResourceFlags::Required);

		// FrameInfo slot: camera, sun, BRDF LUT.
		graph.add_slot_generator([](FrameGraph::Graph& g) {
			auto& sky = g.get_context<SkyInfo>();
			auto& ti  = g.get_context<TimeInfo>();
			auto& ci  = g.get_context<CameraInfo>();

			Slots::FrameInfo fi;
			fi.GetSunDir().xyz = sky.sunDir;
			fi.GetTime()       = {ti.time, ti.totalTime, 0, 0};
			fi.GetCamera()     = ci.cam->camera_cb.current;
			fi.GetPrevCamera() = ci.cam->camera_cb.prev;
			fi.GetBrdf()           = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
			fi.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d().texture2D;
			g.register_slot_setter(fi.compile(*g.builder.current_frame));
		});


		// SceneData slot: vertex/index buffers for hit shaders.
		graph.add_slot_generator([](FrameGraph::Graph& g) {
			g.register_slot_setter(g.get_context<SceneInfo>().scene->compiledScene);
		});

		RTX::get().update();
			  	
		// Pass 1: build TLAS + prepare RTX hit-group table.
		graph.add_library_pass<Passes::PreScene>(
			PassDefault<Passes::PreScene>::setup,
			PassDefault<Passes::PreScene>::render,
			PassDefault<Passes::PreScene>::flags);
				 
		// Pass 2: primary-ray color pass — no GBuffer, writes to ColorOutput.
		const ivec2 rtx_size {W, H};
		graph.add_library_pass<Passes::RTXColorPass>(
			[](auto& data, FrameGraph::TaskBuilder& builder) -> bool {
				builder.need(data.ColorOutput, FrameGraph::ResourceFlags::UnorderedAccess);
			builder.need(data.scene, FrameGraph::ResourceFlags::UnorderedAccess);

				return true;
			},
			[&](auto& data, FrameGraph::FrameContext& ctx) {
				auto& compute    = ctx.get_list()->get_compute();
				auto& scene_info = ctx.graph->get_context<SceneInfo>();

				// Switch to the RTX root signature for the dispatch.
				compute.set_signature(RTX::get().rtx.m_root_sig);

				ctx.graph->set_slot(SlotID::FrameInfo, compute);
				ctx.graph->set_slot(SlotID::SceneData,  compute);

				// Bind the output UAV through the VoxelOutput slot (same as other raygens).
				Slots::VoxelOutput voxel_output;
				voxel_output.GetNoise() = data.ColorOutput->rwTexture2D;
				compute.set(voxel_output);

				// Dispatch primary rays using ColorRTX raygen.
				RTX::get().render<ColorRTX>(compute,
					scene_info.scene->raytrace_scene, rtx_size);
			},
			FrameGraph::PassFlags::Compute);

		graph.setup();
		graph.compile(frame_idx++);
		graph.render();

		graph.commit_command_lists().wait();

	

		graph.reset();
				   
	
		ASSERT_TEXTURE(output->resource.get(), "rtx_material_tester");
	}
}
