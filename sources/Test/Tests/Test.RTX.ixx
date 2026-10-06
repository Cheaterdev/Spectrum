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

// Acceleration structures + inline RayQuery: available without ray tracing
// pipelines (Vulkan, for now), so a category of its own.
SETUP_CATEGORY(Core.RayQuery, []() {
	if (!GraphicsSystem::create())
	{
		Test::TestRegistry::Instance().SkipCategory("Core.RayQuery", "no suitable GPU device found");
		return;
	}
	if (!RenderSystem::get().device().is_ray_query_supported())
		Test::TestRegistry::Instance().SkipCategory("Core.RayQuery", "ray queries not supported on this device");
});

TEARDOWN_CATEGORY(Core.RayQuery, []() {
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
		constexpr uint W = kReferenceWidth, H = kReferenceHeight;

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
		cam.set_projection_params(Math::pi / 4, kReferenceAspect, 0.5f, 200.0f);
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

		// FrameInfo slot: camera, sun, BRDF LUT.
		graph.add_slot_generator([](FrameGraph::Graph& g) {
			auto& sky = g.get_context<SkyInfo>();
			auto& ti  = g.get_context<TimeInfo>();
			auto& ci  = g.get_context<CameraInfo>();

			Slots::Frame::FrameInfo fi;
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
		graph.add_library_pass<Passes::Frame::PreScene>(
			PassDefault<Passes::Frame::PreScene>::setup,
			PassDefault<Passes::Frame::PreScene>::render,
			PassDefault<Passes::Frame::PreScene>::flags);
				 
		// Pass 2: primary-ray color pass — no GBuffer, writes to ColorOutput.
		// The generated need_always()/create_always() link `scene` (PreScene)
		// and create ColorOutput at ViewportInfo::frame_size. ColorOutput can't
		// be supplied via pass_texture: create_always() replaces the passed
		// handler, leaving the pass writing through a null UAV. Required keeps
		// the pass alive with no consumer; the output is read back below.
		const ivec2 rtx_size {W, H};
		// Captured during render: ColorOutput is transient, so the graph no
		// longer holds it once the frame is committed.
		std::shared_ptr<HAL::TextureResource> color_output;
		graph.add_library_pass<Passes::Raytrace::Dev::RTXColorPass>(
			[](auto&, FrameGraph::TaskBuilder&) -> FrameGraph::SetupResult { return true; },
			[&](auto& data, FrameGraph::FrameContext& ctx) {
				color_output = std::dynamic_pointer_cast<HAL::TextureResource>(data.ColorOutput.info->resource);

				auto& compute    = ctx.get_list()->get_compute();
				auto& scene_info = ctx.graph->get_context<SceneInfo>();

				// Switch to the RTX root signature for the dispatch.
				compute.set_signature(RTX::get().rtx.m_root_sig);

				ctx.graph->set_slot(SlotID::FrameInfo, compute);
				ctx.graph->set_slot(SlotID::SceneData,  compute);

				// Bind the output UAV through the dedicated ColorRTXOutput slot.
				Slots::Raytrace::ColorRTXOutput output;
				output.GetOutput() = data.ColorOutput->rwTexture2D;
				compute.set(output);

				// Dispatch primary rays using ColorRTX raygen.
				RTX::get().render<Raytrace::Dev::ColorRTX>(compute,
					scene_info.scene->raytrace_scene, rtx_size);
			},
			FrameGraph::PassFlags::Compute | FrameGraph::PassFlags::Required);

		// RTXColorPass [Always]-needs sky_cubemap_filtered to pull in the sky
		// chain, which this graph doesn't have. FrameInfo.Sky is left unset
		// above, so the miss shader never samples it -- it only has to exist.
		auto sky_stub = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R11G11B10_FLOAT, {1, 1}, 6, 1),
			HAL::HeapType::DEFAULT);
		graph.builder.pass_texture(FrameGraph::ResourceID::sky_cubemap_filtered, sky_stub);

		// PreScene's [PreSetup] hook (raytrace_scene->new_frame()); a Pipeline
		// runs it via run_pre_setups(), a hand-built graph has to call it.
		// Without it the TLAS stays empty and every ray misses.
		PassDefault<Passes::Frame::PreScene>::pre_setup(graph);

		graph.setup();
		graph.compile(frame_idx++);
		graph.render();

		graph.commit_command_lists().wait();

		graph.reset();

		ASSERT_TRUE(color_output != nullptr);
		// A handful of pixels shift with texture re-bakes (texture-cache rebuild
		// moved one shading sample); 0.1% keeps real regressions visible.
		ASSERT_TEXTURE_EX(color_output.get(), "rtx_material_tester",
			.max_mismatch_fraction = 0.001, .float_encoding = Test::FloatEncoding::Encoded);
	}

	// One triangle BLAS, two instances of it at different x offsets and depths,
	// traced with an inline RayQuery straight down -Z (fgt_ray_query.hlsl). Each
	// texel's hit distance is checked against the CPU: 1 - depth inside an
	// instance's triangle, 0 outside. Exercises the BLAS/TLAS builds, instance
	// transforms, the instance -> BLAS reference and the TLAS descriptor.
	TEST(Core.RayQuery, RayQuery_InstancedTriangles)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint size = 128;

		const float3 verts[3] = { { -0.3f, -0.4f, 0.0f }, { 0.3f, -0.4f, 0.0f }, { 0.0f, 0.4f, 0.0f } };   // CCW in xy
		const uint   indices[3] = { 0, 1, 2 };
		struct Placement { float x, z; };
		const Placement placements[2] = { { -0.45f, 0.0f }, { 0.45f, -0.5f } };

		auto vb = HAL::StructuredBufferView<float3>(device, 3);
		auto ib = HAL::StructuredBufferView<uint>(device, 3);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto blas_list = queue->get_free_list();
		blas_list->begin(L"RayQuery BLAS");
		blas_list->get_copy().update(vb, 0, std::span<float3>(const_cast<float3*>(verts), 3));
		blas_list->get_copy().update(ib, 0, std::span<uint>(const_cast<uint*>(indices), 3));

		HAL::GeometryDesc geom = {};
		geom.Type                = HAL::GeometryType::TRIANGLES;
		geom.Flags               = HAL::GeometryFlags::OPAQUE;
		geom.IndexBuffer         = ib.get_resource_address();
		geom.IndexCount          = 3;
		geom.IndexFormat         = HAL::Format::R32_UINT;
		geom.VertexBuffer        = vb.get_resource_address();
		geom.VertexCount         = 3;
		geom.VertexFormat        = HAL::Format::R32G32B32_FLOAT;
		geom.VertexStrideInBytes = sizeof(float3);
		HAL::RaytracingAccelerationStructure blas(device, { geom }, blas_list);
		blas_list->execute_and_wait();

		std::vector<HAL::InstanceDesc> instances;
		for (uint i = 0; i < 2; ++i)
		{
			HAL::InstanceDesc inst = {};
			for (int r = 0; r < 3; ++r)
				for (int c = 0; c < 4; ++c)
					inst.transform.rows[r][c] = r == c ? 1.0f : 0.0f;
			inst.transform.rows[0][3] = placements[i].x;
			inst.transform.rows[2][3] = placements[i].z;
			inst.instance_id = i;
			inst.mask = 0xFF;
			inst.acceleration_structure = blas.get_gpu_address().get_ptr();
			instances.push_back(inst);
		}
		HAL::RaytracingAccelerationStructure tlas(device, instances);   // builds and waits

		auto output = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R32_FLOAT, { size, size }, 1, 1, HAL::ResFlags::UnorderedAccess),
			HAL::HeapType::DEFAULT);

		auto list = queue->get_free_list();
		list->begin(L"RayQuery trace");
		HAL::Texture2DView output_view(output, *list);
		auto& compute = list->get_compute();
		compute.set_signature(Layouts::DefaultLayout);
		{
			Slots::FrameGraphTests::FGTRayQuery rq;
			rq.GetSize()   = uint2(size, size);
			rq.GetScene()  = tlas.get_handle();
			rq.GetOutput() = output_view.rwTexture2D;
			compute.set(rq);
		}
		compute.set_pipeline<PSOS::FrameGraphTests::FGTRayQuery>();
		compute.dispatch(ivec2(size, size), ivec2(8, 8));
		list->execute_and_wait();

		auto data = Test::readback_texture(output.get());
		auto& mip = data->array[0]->mips[0];

		// Signed distance to the triangle's edges (positive inside); texels within
		// two texels of an edge are rasterization-rule territory and go unchecked.
		auto inside_distance = [&](float x, float y) {
			float d = std::numeric_limits<float>::max();
			for (int e = 0; e < 3; ++e)
			{
				const float3& a = verts[e];
				const float3& b = verts[(e + 1) % 3];
				const float ex = b.x - a.x, ey = b.y - a.y;
				d = std::min(d, (ex * (y - a.y) - ey * (x - a.x)) / std::sqrt(ex * ex + ey * ey));
			}
			return d;
		};
		const float margin = 2.0f * 2.0f / size;

		size_t checked_hits = 0, checked_misses = 0;
		for (uint py = 0; py < size; ++py)
			for (uint px = 0; px < size; ++px)
			{
				const float x = (px + 0.5f) / size * 2 - 1;
				const float y = -((py + 0.5f) / size * 2 - 1);

				float expected = 0;
				bool  ambiguous = false;
				for (auto& pl : placements)
				{
					const float d = inside_distance(x - pl.x, y);
					if (std::abs(d) < margin) ambiguous = true;
					else if (d > 0) expected = 1.0f - pl.z;
				}
				if (ambiguous) continue;

				const float got = reinterpret_cast<const float*>(mip->data.data() + (size_t)py * mip->width_stride)[px];
				if (std::abs(got - expected) > 1e-4f)
					throw Test::TestFailure("texel (" + std::to_string(px) + "," + std::to_string(py) + ") hit distance "
						+ std::to_string(got) + ", expected " + std::to_string(expected));
				(expected > 0 ? checked_hits : checked_misses)++;
			}

		ASSERT_TRUE(checked_hits > 1000);
		ASSERT_TRUE(checked_misses > 1000);
	}
}
