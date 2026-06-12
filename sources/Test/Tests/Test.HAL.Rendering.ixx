export module Test.HAL.Rendering;

#define TEST_MODULE_ID HALRendering

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;

export namespace Test
{
	TEST(Core.HAL, RenderTriangle)
	{
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
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
}
