export module Test.HAL.Rendering;

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
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0, 0, 0, 1));

		gfx.set_pipeline(pso);
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(3);

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "triangle");
	}
}
