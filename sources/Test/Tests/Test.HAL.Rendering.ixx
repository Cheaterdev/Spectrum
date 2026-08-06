export module Test.HAL.Rendering;

#define TEST_MODULE_ID HALRendering

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import RenderSystem;
import Graphics;

namespace {
	// Left triangle  — CCW in NDC (signed area > 0) = front face = GREEN
	// Right triangle — CW  in NDC (signed area < 0) = back  face = RED
	// With BackCull:  only GREEN visible.
	// With FrontCull: only RED  visible.
	// With NoCull:    both visible.
	static constexpr const char* kCullHLSL = R"hlsl(
static const float2 kPos[6] = {
    float2(-0.7,-0.5), float2(-0.1,-0.5), float2(-0.4, 0.5),  // CCW = front face
    float2( 0.1,-0.5), float2( 0.4, 0.5), float2( 0.7,-0.5),  // CW  = back  face
};
static const float4 kColor[2] = {
    float4(0.0,1.0,0.0,1.0),  // GREEN = front face
    float4(1.0,0.0,0.0,1.0),  // RED   = back  face
};
struct VSOut { float4 pos : SV_Position; float4 col : COLOR0; };
VSOut VS(uint vid : SV_VertexID)
{
    VSOut o;
    o.pos = float4(kPos[vid], 0.0, 1.0);
    o.col = kColor[vid / 3];
    return o;
}
float4 PS(VSOut i) : SV_Target { return i.col; }
)hlsl";

	std::shared_ptr<HAL::TextureResource> run_cull_test(HAL::Device& device, HAL::CullMode mode, LiteralWStr label)
	{
		constexpr uint W = 256, H = 256;
		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {W, H}, 1, 1, HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		SimpleGraphicsPSO mpso("TestCull");
		mpso.root_signature = Layouts::NoneLayout;
		mpso.vertex        = { kCullHLSL, "VS", HAL::ShaderOptions::None, {}, true };
		mpso.pixel         = { kCullHLSL, "PS", HAL::ShaderOptions::None, {}, true };
		mpso.rtv_formats   = { HAL::Format::R8G8B8A8_UNORM };
		mpso.enable_depth  = false;
		mpso.cull          = mode;
		mpso.topology      = HAL::PrimitiveTopologyType::TRIANGLE;
		auto pso = mpso.create(device);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(label);

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline(pso);
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(6);

		list->execute_and_wait();
		return tex;
	}
}

export namespace Test
{
	TEST(Core.HAL, RenderTriangle)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH  = 256;
		constexpr uint HEIGHT = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		// Orange triangle on black background. VS positions via SV_VertexID in clip space.
		static constexpr const char* kShaderHLSL = R"hlsl(
float4 VS(uint vertID : SV_VertexID) : SV_Position
{
    float2 pos[3];
    pos[0] = float2(-0.5, -0.5);
    pos[1] = float2( 0.0,  0.5);
    pos[2] = float2( 0.5, -0.5);
    return float4(pos[vertID], 0.0, 1.0);
}
float4 PS() : SV_Target
{
    return float4(1.0, 0.5, 0.0, 1.0);
}
)hlsl";

		SimpleGraphicsPSO mpso("TestTriangle");
		mpso.root_signature = Layouts::NoneLayout;
		mpso.vertex        = { kShaderHLSL, "VS", HAL::ShaderOptions::None, {}, true };
		mpso.pixel         = { kShaderHLSL, "PS", HAL::ShaderOptions::None, {}, true };
		mpso.rtv_formats   = { HAL::Format::R8G8B8A8_UNORM };
		mpso.enable_depth  = false;
		mpso.cull          = HAL::CullMode::None;
		mpso.topology      = HAL::PrimitiveTopologyType::TRIANGLE;

		auto pso = mpso.create(device);

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

		gfx.set_pipeline(pso);
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(3);

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "triangle");
	}

	TEST(Core.HAL, RenderInstancing)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 256, HEIGHT = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		// 9 instances of a small triangle arranged in a 3×3 grid.
		// SV_InstanceID drives position (column/row offset) and color.
		static constexpr const char* kInstHLSL = R"hlsl(
static const float2 kLocalPos[3] = {
    float2(-0.12, -0.10),
    float2( 0.12, -0.10),
    float2( 0.00,  0.12),
};
static const float4 kColor[9] = {
    float4(1,0,0,1),   float4(0,1,0,1),   float4(0,0,1,1),
    float4(1,1,0,1),   float4(1,0,1,1),   float4(0,1,1,1),
    float4(1,0.5,0,1), float4(0,0.5,1,1), float4(0.5,1,0,1),
};
struct VSOut { float4 pos : SV_Position; float4 col : COLOR0; };
VSOut VS(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    uint col = iid % 3;
    uint row = iid / 3;
    float2 offset = float2((float(col) - 1.0) * 0.62,
                           (1.0 - float(row)) * 0.62);
    VSOut o;
    o.pos = float4(kLocalPos[vid] + offset, 0.0, 1.0);
    o.col = kColor[iid];
    return o;
}
float4 PS(VSOut i) : SV_Target { return i.col; }
)hlsl";

		SimpleGraphicsPSO mpso("TestInstancing");
		mpso.root_signature = Layouts::NoneLayout;
		mpso.vertex        = { kInstHLSL, "VS", HAL::ShaderOptions::None, {}, true };
		mpso.pixel         = { kInstHLSL, "PS", HAL::ShaderOptions::None, {}, true };
		mpso.rtv_formats   = { HAL::Format::R8G8B8A8_UNORM };
		mpso.enable_depth  = false;
		mpso.cull          = HAL::CullMode::None;
		mpso.topology      = HAL::PrimitiveTopologyType::TRIANGLE;

		auto pso = mpso.create(device);

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

		gfx.set_pipeline(pso);
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(3, 0, 9);  // 3 vertices per instance, 9 instances

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "instancing");
	}

	TEST(Core.HAL, RenderCube)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH  = 256;
		constexpr uint HEIGHT = 256;

		auto color_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto depth_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::D32_FLOAT, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::DepthStencil),
			HAL::HeapType::DEFAULT);

		// Cube: 6 faces × 2 tris × 3 verts = 36, each face a distinct color.
		// VS builds clip-space position from SV_VertexID using hardcoded positions +
		// a fixed rotation + reversed-depth perspective (near=1, far=20, fovY=60°).
		static constexpr const char* kCubeHLSL = R"hlsl(
static const float3 kPos[36] = {
    // Front (+Z)
    float3(-0.5,-0.5, 0.5), float3( 0.5,-0.5, 0.5), float3( 0.5, 0.5, 0.5),
    float3(-0.5,-0.5, 0.5), float3( 0.5, 0.5, 0.5), float3(-0.5, 0.5, 0.5),
    // Back (-Z)
    float3( 0.5,-0.5,-0.5), float3(-0.5,-0.5,-0.5), float3(-0.5, 0.5,-0.5),
    float3( 0.5,-0.5,-0.5), float3(-0.5, 0.5,-0.5), float3( 0.5, 0.5,-0.5),
    // Left (-X)
    float3(-0.5,-0.5,-0.5), float3(-0.5,-0.5, 0.5), float3(-0.5, 0.5, 0.5),
    float3(-0.5,-0.5,-0.5), float3(-0.5, 0.5, 0.5), float3(-0.5, 0.5,-0.5),
    // Right (+X)
    float3( 0.5,-0.5, 0.5), float3( 0.5,-0.5,-0.5), float3( 0.5, 0.5,-0.5),
    float3( 0.5,-0.5, 0.5), float3( 0.5, 0.5,-0.5), float3( 0.5, 0.5, 0.5),
    // Top (+Y)
    float3(-0.5, 0.5, 0.5), float3( 0.5, 0.5, 0.5), float3( 0.5, 0.5,-0.5),
    float3(-0.5, 0.5, 0.5), float3( 0.5, 0.5,-0.5), float3(-0.5, 0.5,-0.5),
    // Bottom (-Y)
    float3(-0.5,-0.5,-0.5), float3( 0.5,-0.5,-0.5), float3( 0.5,-0.5, 0.5),
    float3(-0.5,-0.5,-0.5), float3( 0.5,-0.5, 0.5), float3(-0.5,-0.5, 0.5),
};
static const float4 kColor[6] = {
    float4(1,0,0,1), float4(0,1,0,1), float4(0,0,1,1),
    float4(1,1,0,1), float4(1,0,1,1), float4(0,1,1,1),
};
struct VSOut { float4 pos : SV_Position; float4 col : COLOR0; };
VSOut VS(uint vid : SV_VertexID)
{
    float3 p = kPos[vid];
    // Rotate 45 deg around Y  (cos=0.7071, sin=0.7071)
    float3 q;
    q.x =  p.x * 0.7071 + p.z * 0.7071;
    q.y =  p.y;
    q.z = -p.x * 0.7071 + p.z * 0.7071;
    p = q;
    // Rotate 30 deg around X  (cos=0.8660, sin=0.5)
    q.x =  p.x;
    q.y =  p.y * 0.8660 - p.z * 0.5;
    q.z =  p.y * 0.5    + p.z * 0.8660;
    p = q;
    // Place cube at z=2.5 (camera at origin, looking +Z)
    p.z += 2.5;
    // Reversed-depth perspective: near=1, far=20, fovY=60 deg (f=cot(30)=1.7321)
    // A = -near/(far-near) = -1/19, B = far/(far-near) = 20/19
    float f = 1.7321;
    float A = -0.052632;
    float B =  1.052632;
    VSOut o;
    o.pos = float4(p.x * f, p.y * f, p.z * A + B, p.z);
    o.col = kColor[vid / 6];
    return o;
}
float4 PS(VSOut i) : SV_Target { return i.col; }
)hlsl";

		SimpleGraphicsPSO mpso("TestCube");
		mpso.root_signature = Layouts::NoneLayout;
		mpso.vertex        = { kCubeHLSL, "VS", HAL::ShaderOptions::None, {}, true };
		mpso.pixel         = { kCubeHLSL, "PS", HAL::ShaderOptions::None, {}, true };
		mpso.rtv_formats   = { HAL::Format::R8G8B8A8_UNORM };
		mpso.ds            = HAL::Format::D32_FLOAT;
		mpso.enable_depth  = true;
		mpso.depth_write   = true;
		mpso.cull          = HAL::CullMode::None;
		mpso.topology      = HAL::PrimitiveTopologyType::TRIANGLE;

		auto pso = mpso.create(device);

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

		gfx.set_pipeline(pso);
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
		auto& device = RenderSystem::get().device();
		constexpr uint W = 256, H = 256;

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
		cam.set_projection_params(Math::pi / 4, 1.0f, 0.01f, base_dist * 4.0f + 1.0f);
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

		static constexpr const char* kNormalPS = R"hlsl(
#include "mesh_shader.hlsl"
float4 PS(vertex_output i) : SV_Target { return float4(i.normal * 0.5 + 0.5, 1); }
)hlsl";

		SimpleGraphicsPSO mpso("TestMeshDirect");
		mpso.root_signature = Layouts::DefaultLayout;
		mpso.mesh           = { "shaders/mesh_shader.hlsl", "VS", HAL::ShaderOptions::None, {}, false };
		mpso.amplification  = { "shaders/mesh_shader.hlsl", "AS", HAL::ShaderOptions::None, {}, false };
		mpso.pixel          = { kNormalPS, "PS", HAL::ShaderOptions::None, {}, true };
		mpso.rtv_formats    = { HAL::Format::R8G8B8A8_UNORM };
		mpso.ds             = HAL::Format::D32_FLOAT;
		mpso.enable_depth   = true;
		mpso.cull           = HAL::CullMode::None;
		mpso.depth_func     = HAL::ComparisonFunc::GREATER;
		mpso.topology       = HAL::PrimitiveTopologyType::TRIANGLE;
		auto pso = mpso.create(device);

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
		gfx.set_pipeline(pso);
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);

		gfx.set(scene->compiledScene);
		{
			Slots::FrameInfo frameInfo;
			frameInfo.GetCamera() = cam.camera_cb.current;
			gfx.set(frameInfo);
		}

		for (auto& m : mesh_inst->rendering)
		{
			gfx.set(m.compiled_mesh_info);
			gfx.set(m.mesh_instance_info);
			gfx.dispatch_mesh(m.dispatch_mesh_arguments);
		}

		list->execute_and_wait();

		ASSERT_TEXTURE(color_tex.get(), "mesh_direct_normal");
	}

	// Geometry shader: VS emits one point at the origin; GS expands it into a
	// full-screen orange triangle.  Verifies the GS stage is invoked and produces
	// visible output — a regression target for geometry shader pipeline breakage.
	TEST(Core.HAL, RenderGeometryShader)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint W = 256, H = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		static constexpr const char* kGsHLSL = R"hlsl(
struct GSIn  { float4 pos : SV_Position; };
struct GSOut { float4 pos : SV_Position; float4 col : COLOR0; };

// VS: emit a single point — position is irrelevant, GS ignores it.
GSIn VS() { GSIn o; o.pos = float4(0,0,0,1); return o; }

// GS: one point in → one triangle out covering most of the screen.
[maxvertexcount(3)]
void GS(point GSIn input[1], inout TriangleStream<GSOut> stream)
{
    float4 orange = float4(1.0, 0.5, 0.0, 1.0);
    GSOut v;
    v.col = orange;
    v.pos = float4(-0.9, -0.9, 0, 1); stream.Append(v);
    v.pos = float4( 0.0,  0.9, 0, 1); stream.Append(v);
    v.pos = float4( 0.9, -0.9, 0, 1); stream.Append(v);
}

float4 PS(GSOut i) : SV_Target { return i.col; }
)hlsl";

		SimpleGraphicsPSO mpso("TestGeometryShader");
		mpso.root_signature = Layouts::NoneLayout;
		mpso.vertex        = { kGsHLSL, "VS", HAL::ShaderOptions::None, {}, true };
		mpso.geometry      = { kGsHLSL, "GS", HAL::ShaderOptions::None, {}, true };
		mpso.pixel         = { kGsHLSL, "PS", HAL::ShaderOptions::None, {}, true };
		mpso.rtv_formats   = { HAL::Format::R8G8B8A8_UNORM };
		mpso.enable_depth  = false;
		mpso.cull          = HAL::CullMode::None;
		mpso.topology      = HAL::PrimitiveTopologyType::POINT;
		auto pso = mpso.create(device);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"RenderGeometryShader");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
			vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline(pso);
		gfx.set_topology(HAL::PrimitiveTopologyType::POINT, HAL::PrimitiveTopologyFeed::LIST);
		gfx.draw(1);  // one point → GS expands to one triangle

		list->execute_and_wait();
		ASSERT_TEXTURE(tex.get(), "geometry_shader");
	}
}
