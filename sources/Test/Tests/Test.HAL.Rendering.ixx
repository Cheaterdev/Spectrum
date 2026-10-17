export module Test.HAL.Rendering;

#define TEST_MODULE_ID HALRendering

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import RenderSystem;
import Graphics;

namespace {
	using Test::kReferenceWidth;
	using Test::kReferenceHeight;
	using Test::kReferenceAspect;

	// Mesh renders compared across GPU vendors/backends. Measured AMD (Vulkan)
	// against the NVIDIA (D3D12) goldens: one silhouette pixel whose centre a
	// triangle edge passes through rasterizes differently, and interior shading
	// differs by up to 4/255. A wrong mesh or a missing meshlet changes
	// thousands of pixels, far beyond 0.01%.
	constexpr Test::TextureCompare kMeshCompare{ .tolerance = 6, .max_mismatch_fraction = 0.0001 };

	// The test shaders are files (workdir/shaders/test/*.hlsl) with PSOs declared in
	// sources/Prism/defs/gpu_tests.prism, so a device without a shader compiler gets them precompiled.
	// Cull: left triangle CCW = front face = GREEN, right triangle CW = back face = RED
	// (BackCull shows only green, FrontCull only red, NoCull both; see test/cull.hlsl).
	std::shared_ptr<HAL::TextureResource> run_cull_test(HAL::Device& device, HAL::CullMode mode, LiteralWStr label)
	{
		constexpr uint W = kReferenceWidth, H = kReferenceHeight;
		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {W, H}, 1, 1, HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(label);

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		switch (mode)
		{
		case HAL::CullMode::Back:  gfx.set_pipeline<PSOS::Tests::TestCullBack>();  break;
		case HAL::CullMode::Front: gfx.set_pipeline<PSOS::Tests::TestCullFront>(); break;
		default:                   gfx.set_pipeline<PSOS::Tests::TestCullNone>();  break;
		}
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(6);

		list->execute_and_wait();
		return tex;
	}

	// material_tester through the debug view's real PSO (PSOS::Dev::DebugViewMesh:
	// mesh_shader.hlsl with DEBUG_VIEW + debug_view_mesh.hlsl), coloured by
	// `color_mode`. Direct dispatch, no culling capture: read_meshlet_masks = 0
	// shows every meshlet. `lod_threshold` 0 turns cluster LOD off (original
	// meshlets); above 0 it is the allowed projected error as a fraction of the
	// viewport height, picking a DAG cut. DEBUG_VIEW forces the cluster-LOD
	// amplification shader (one group per LOD node), hence
	// lod_dispatch_mesh_arguments, not dispatch_mesh_arguments.
	std::shared_ptr<HAL::TextureResource> render_debug_view_mesh(Dev::DebugViewColor color_mode, float lod_threshold,
	                                                             float camera_distance, LiteralWStr label)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint W = kReferenceWidth, H = kReferenceHeight;

		auto color_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto depth_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::D32_FLOAT, {W, H}, 1, 1,
				HAL::ResFlags::DepthStencil),
			HAL::HeapType::DEFAULT);

		auto mesh_inst = std::make_shared<MeshAssetInstance>(EngineAssets::material_tester.get_asset());

		// The PS colours by hash(object_id) ^ meshlet, and object ids come from a
		// process-wide counter (MeshAsset.cpp's next_object_id), so they depend
		// on how many meshes earlier tests created. Pin them, or the golden's
		// palette changes with test order.
		for (uint i = 0; i < mesh_inst->rendering.size(); ++i)
		{
			auto& part = mesh_inst->rendering[i];
			part.mesh_info.GetObject_id() = i + 1;
			part.compiled_mesh_info = part.mesh_info.compile(device.get_static_gpu_data());
		}

		auto scene = std::make_shared<Scene>();
		scene->add_child(mesh_inst);
		scene->update_transforms();

		auto mn = scene->get_min();
		auto mx = scene->get_max();
		float base_dist = (mx - mn).length();
		if (base_dist < 0.001f) base_dist = 1.0f;

		camera cam;
		vec3 dir = vec3(0.577f, 0.577f, 0.577f);
		cam.target   = (mn + mx) / 2;
		cam.position = cam.target + dir * (base_dist * camera_distance);
		cam.set_projection_params(Math::pi / 4, kReferenceAspect, 0.01f, base_dist * (camera_distance + 2.5f) + 1.0f);
		cam.update();

		auto frame = device.get_frame_manager().begin_frame();
		scene->update(*frame);
		{
			auto upload_list = frame->start_list(L"PrepareUpload", HAL::CommandListType::DIRECT, false);
			SceneFrameManager::get().prepare(upload_list, *scene);
			upload_list->execute_and_wait();
		}

		auto list = frame->start_list(label, HAL::CommandListType::DIRECT, true);

		HAL::Texture2DView color_view(color_tex, *list);
		HAL::Texture2DView depth_view(depth_tex, *list);

		HAL::CompiledRT compiled;
		compiled.table_rtv = color_view.renderTarget;
		compiled.table_dsv = depth_view.depthStencil;

		auto& gfx = list->get_graphics();
		gfx.set_signature(Layouts::DefaultLayout);
		gfx.set_rtv(compiled,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor | HAL::RTOptions::ClearDepth,
			0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline<PSOS::Dev::DebugViewMesh>();
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);

		gfx.set(scene->compiledScene);
		{
			Slots::Frame::FrameInfo frameInfo;
			frameInfo.GetCamera()         = cam.camera_cb.current;
			frameInfo.GetPrevCamera()     = cam.camera_cb.current;
			frameInfo.GetLodThreshold()   = lod_threshold;
			frameInfo.GetBrdf()           = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
			frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d().texture2D;
			gfx.set(frameInfo);
		}
		{
			Slots::Dev::DebugViewFrustum frustum;
			frustum.GetOutside_brightness() = 1.0f;
			gfx.set(frustum);
		}
		{
			Slots::Dev::DebugViewDraw draw;
			draw.GetTint()               = float4(0, 0, 0, 0);
			draw.GetMeshlet_masks()      = universal_meshlet_mask_manager::get().buffer;
			draw.GetRead_meshlet_masks() = 0;
			draw.GetCapture_frame()      = 0;
			draw.GetMeshlet_filter()     = 0;
			draw.GetColor_mode()         = color_mode;
			gfx.set(draw);
		}

		for (auto& m : mesh_inst->rendering)
		{
			gfx.set(m.compiled_mesh_info);
			gfx.set(m.mesh_instance_info);
			gfx.dispatch_mesh(m.lod_dispatch_mesh_arguments);
		}

		list->execute_and_wait();
		return color_tex;
	}
}

export namespace Test
{
	TEST(Core.HAL, RenderTriangle)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH  = kReferenceWidth;
		constexpr uint HEIGHT = kReferenceHeight;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		// Orange triangle on a dark background (test/triangle.hlsl, PSOS::Tests::TestTriangle).
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"RenderTriangle");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		auto& gfx = list->get_graphics();
		// Non-zero clear color so the test actually validates clearing — a black
		// (zero) background is indistinguishable from zeroed/uninitialized memory.
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));

		gfx.set_pipeline<PSOS::Tests::TestTriangle>();
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(3);

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "triangle");
	}

	TEST(Core.HAL, RenderInstancing)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = kReferenceWidth, HEIGHT = kReferenceHeight;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		// 9 instances of a small triangle arranged in a 3×3 grid; SV_InstanceID drives position and
		// color (test/instancing.hlsl, PSOS::Tests::TestInstancing).
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"RenderInstancing");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		auto& gfx = list->get_graphics();
		// Non-zero clear color so the test actually validates clearing — a black
		// (zero) background is indistinguishable from zeroed/uninitialized memory.
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));

		gfx.set_pipeline<PSOS::Tests::TestInstancing>();
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(3, 0, 9);  // 3 vertices per instance, 9 instances

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "instancing");
	}

	TEST(Core.HAL, RenderCube)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH  = kReferenceWidth;
		constexpr uint HEIGHT = kReferenceHeight;

		auto color_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto depth_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::D32_FLOAT, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::DepthStencil),
			HAL::HeapType::DEFAULT);

		// Cube: 6 faces × 2 tris × 3 verts = 36, each face a distinct color, depth-tested with reversed-Z
		// (test/cube.hlsl, PSOS::Tests::TestCube).
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"RenderCube");

		HAL::Texture2DView color_view(color_tex, *list);
		HAL::Texture2DView depth_view(depth_tex, *list);

		HAL::CompiledRT compiled;
		compiled.table_rtv = color_view.renderTarget;
		compiled.table_dsv = depth_view.depthStencil;

		auto& gfx = list->get_graphics();
		// Clear color + depth only (D32_FLOAT has no stencil; depth=0 = far in reversed-depth)
		gfx.set_rtv(compiled,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor | HAL::RTOptions::ClearDepth,
			0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));

		gfx.set_pipeline<PSOS::Tests::TestCube>();
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(36);

		list->execute_and_wait();

		ASSERT_TEXTURE(color_tex.get(), "cube");
	}

	// Left = GREEN (CCW in NDC = front face), Right = RED (CW in NDC = back face).
	// NoCull: both triangles visible.
	TEST(Core.HAL, RenderCull_None)
	{
		auto rt = run_cull_test(RenderSystem::get().device(), HAL::CullMode::None, L"RenderCull_None"_cs);
		ASSERT_TEXTURE(rt.get(), "cull_none");
	}

	// BackCull: back face (CW in NDC, RED right triangle) discarded — only GREEN left triangle visible.
	TEST(Core.HAL, RenderCull_Back)
	{
		auto rt = run_cull_test(RenderSystem::get().device(), HAL::CullMode::Back, L"RenderCull_Back"_cs);
		ASSERT_TEXTURE(rt.get(), "cull_back");
	}

	// FrontCull: front face (CCW in NDC, GREEN left triangle) discarded — only RED right triangle visible.
	TEST(Core.HAL, RenderCull_Front)
	{
		auto rt = run_cull_test(RenderSystem::get().device(), HAL::CullMode::Front, L"RenderCull_Front"_cs);
		ASSERT_TEXTURE(rt.get(), "cull_front");
	}

	// Regression test for the direct (non-indirect) dispatch_mesh pattern
	// MaterialPreviewSession::dispatch() uses (see PreviewSession.cpp):
	// material_tester drawn with a real mesh/AS + a trivial
	// normal-visualization PS, no COMPILED_FUNC, no capture writes. Catches
	// the actual bug this was built to isolate -- Scene::update() alone only
	// refreshes CPU-side handle allocations, not the GPU-visible node/mesh/
	// material buffers mesh_shader.hlsl reads; SceneFrameManager::prepare()
	// is the real upload (normally done once per frame by PreSceneSystem.cpp's
	// PreScene pass) and is easy to forget when bypassing the FrameGraph.
	TEST(Core.HAL, RenderMeshDirect)
	{
		if (!RenderSystem::get().device().is_mesh_shader_supported())
			throw ::Test::TestSkipped("mesh shaders not supported");

		auto& device = RenderSystem::get().device();
		constexpr uint W = kReferenceWidth, H = kReferenceHeight;

		auto color_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto depth_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::D32_FLOAT, {W, H}, 1, 1,
				HAL::ResFlags::DepthStencil),
			HAL::HeapType::DEFAULT);

		auto mesh_inst = std::make_shared<MeshAssetInstance>(EngineAssets::material_tester.get_asset());
		auto scene = std::make_shared<Scene>();
		scene->add_child(mesh_inst);
		scene->update_transforms();

		// Auto-fit camera from the mesh's actual bounds, same as
		// MaterialPreviewSession::dispatch() -- the RTX test's hardcoded
		// camera assumes a specific scale that may not suit rasterization's
		// near/far clipping the same way it suits ray tracing.
		auto mn = scene->get_min();
		auto mx = scene->get_max();
		float base_dist = (mx - mn).length();
		if (base_dist < 0.001f) base_dist = 1.0f;

		camera cam;
		vec3 dir = vec3(0.577f, 0.577f, 0.577f);
		cam.target   = (mn + mx) / 2;
		cam.position = cam.target + dir * (base_dist * 1.5f);
		cam.set_projection_params(Math::pi / 4, kReferenceAspect, 0.01f, base_dist * 4.0f + 1.0f);
		cam.update();

		auto frame = device.get_frame_manager().begin_frame();
		scene->update(*frame);

		// Scene::update() alone only updates CPU-side handle allocations --
		// SceneFrameManager::prepare() is the actual GPU upload (see
		// PreSceneSystem.cpp's PreScene pass, which every real render calls
		// once per frame). Without it the mesh/node/material buffers this
		// mesh shader reads are stale/empty.
		{
			auto upload_list = frame->start_list(L"PrepareUpload", HAL::CommandListType::DIRECT, false);
			SceneFrameManager::get().prepare(upload_list, *scene);
			upload_list->execute_and_wait();
		}

		// PSOS::Tests::TestMeshDirect: gbuffer/mesh_shader.hlsl's mesh + amplification stages, and
		// test/mesh_direct_normal.hlsl's pixel shader.

		// Same list scene->update(*frame) itself is tied to -- not a separate
		// queue->get_free_list(), which isn't synchronized against it.
		auto list = frame->start_list(L"RenderMeshDirect", HAL::CommandListType::DIRECT, true);

		HAL::Texture2DView color_view(color_tex, *list);
		HAL::Texture2DView depth_view(depth_tex, *list);

		HAL::CompiledRT compiled;
		compiled.table_rtv = color_view.renderTarget;
		compiled.table_dsv = depth_view.depthStencil;

		auto& gfx = list->get_graphics();
		gfx.set_signature(Layouts::DefaultLayout);
		gfx.set_rtv(compiled,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor | HAL::RTOptions::ClearDepth,
			0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline<PSOS::Tests::TestMeshDirect>();
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);

		gfx.set(scene->compiledScene);
		{
			Slots::Frame::FrameInfo frameInfo;
			frameInfo.GetCamera() = cam.camera_cb.current;
			frameInfo.GetBrdf()           = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
			frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d().texture2D;
			gfx.set(frameInfo);
		}

		for (auto& m : mesh_inst->rendering)
		{
			gfx.set(m.compiled_mesh_info);
			gfx.set(m.mesh_instance_info);
			gfx.dispatch_mesh(m.dispatch_mesh_arguments);
		}

		list->execute_and_wait();

		Test::check_texture_reference(color_tex.get(), "mesh_direct_normal", kMeshCompare);
	}

	// Every original meshlet in its own colour (debug view, Meshlet mode, LOD off).
	TEST(Core.HAL, RenderMeshlets_PerMeshletColors)
	{
		if (!RenderSystem::get().device().is_mesh_shader_supported())
			throw ::Test::TestSkipped("mesh shaders not supported");

		auto tex = render_debug_view_mesh(Dev::DebugViewColor::Meshlet, 0.0f, 1.5f, L"RenderMeshlets_PerMeshletColors");
		Test::check_texture_reference(tex.get(), "meshlets_per_meshlet_colors", kMeshCompare);
	}

	// Cluster-LOD cut, coloured by DAG level (debug view, LodLevel mode). A far
	// camera with a coarse threshold, so the cut reaches into the coarser levels
	// and the LOD walk in the amplification shader is exercised, not bypassed.
	TEST(Core.HAL, RenderMeshlets_LodLevels)
	{
		if (!RenderSystem::get().device().is_mesh_shader_supported())
			throw ::Test::TestSkipped("mesh shaders not supported");

		auto tex = render_debug_view_mesh(Dev::DebugViewColor::LodLevel, 0.002f, 2.0f, L"RenderMeshlets_LodLevels");
		Test::check_texture_reference(tex.get(), "meshlets_lod_levels", kMeshCompare);
	}

	// Geometry shader: VS emits one point at the origin; GS expands it into a
	// full-screen orange triangle.  Verifies the GS stage is invoked and produces
	// visible output — a regression target for geometry shader pipeline breakage.
	TEST(Core.HAL, RenderGeometryShader)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint W = kReferenceWidth, H = kReferenceHeight;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		// test/geometry_shader.hlsl, PSOS::Tests::TestGeometryShader.
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"RenderGeometryShader");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
			vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline<PSOS::Tests::TestGeometryShader>();
		gfx.set_topology(HAL::PrimitiveTopologyType::POINT, HAL::PrimitiveTopologyFeed::LIST);
		gfx.draw(1);  // one point → GS expands to one triangle

		list->execute_and_wait();
		ASSERT_TEXTURE(tex.get(), "geometry_shader");
	}
}
