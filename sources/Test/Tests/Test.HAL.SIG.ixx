export module Test.HAL.SIG;

#define TEST_MODULE_ID HALSIG

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import RenderSystem;

export namespace Test
{
	// Verifies that Slots::Color (a plain float4 CBV) round-trips through the
	// DefaultLayout SIG pipeline: compile the slot via the graphics context, bind
	// it, draw a full-screen triangle that reads the CBV and writes the color to
	// the render target, and compare the result against the golden image.
	TEST(Core.HAL, SIGColor)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 64, HEIGHT = 64;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		// Full-screen triangle that reads the Color CBV and outputs it.
		// Uses the same indirect-CBV boilerplate as the autogen Color.h header so
		// the binding matches the Color slot. NOTE: the register/push index must
		// stay in sync with workdir/shaders/autogen/Color.h — the SIG layout can
		// shift slot indices when .sig files change. Color currently lives at
		// slot 8 → b8/space8 (D3D12) / _hal_push.s8 (SPIR-V).
		static constexpr const char* kSigColorHLSL = R"hlsl(
struct CB { uint offset; };
struct Color { float4 color; };
#ifdef __spirv__
#ifndef HAL_PUSH_DEFINED
#define HAL_PUSH_DEFINED
struct _HALPush { uint s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15; };
[[vk::push_constant]] ConstantBuffer<_HALPush> _hal_push;
#endif
struct _CB_Color { uint offset; };
static _CB_Color pass_Color = { _hal_push.s8 };
#else
ConstantBuffer<CB> pass_Color : register(b8, space8);
#endif
ConstantBuffer<Color> CreateColor() { return ResourceDescriptorHeap[pass_Color.offset]; }

float4 VS(uint vid : SV_VertexID) : SV_Position
{
    float2 uv = float2((vid & 1) * 2.0, (vid >> 1) * 2.0);
    return float4(uv * 2.0 - 1.0, 0.0, 1.0);
}
float4 PS() : SV_Target
{
    Color c = CreateColor();
    return c.color;
}
)hlsl";

		SimpleGraphicsPSO mpso("TestSIGColor");
		mpso.root_signature = Layouts::DefaultLayout;
		mpso.vertex        = { kSigColorHLSL, "VS", HAL::ShaderOptions::None, {}, true };
		mpso.pixel         = { kSigColorHLSL, "PS", HAL::ShaderOptions::None, {}, true };
		mpso.rtv_formats   = { HAL::Format::R8G8B8A8_UNORM };
		mpso.enable_depth  = false;
		mpso.cull          = HAL::CullMode::None;
		mpso.topology      = HAL::PrimitiveTopologyType::TRIANGLE;
		auto pso = mpso.create(device);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"SIGColor");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		Slots::Color color_slot;
		color_slot.GetColor() = float4(0.4f, 0.2f, 0.8f, 1.0f);

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline(pso);
		gfx.set(color_slot);
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(3);

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "sig_color");
	}

	// Verifies the SRV (Texture2D) path of the SIG system: upload a solid-color
	// source texture, bind it via Slots::CopyTexture, render a full-screen quad
	// that loads each texel by pixel position, and compare the copy output.
	TEST(Core.HAL, SIGCopyTexture)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 64, HEIGHT = 64;

		// Source texture: ShaderResource (default flags), filled with solid orange.
		auto src_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1),
			HAL::HeapType::DEFAULT);

		{
			constexpr uint ROW_STRIDE = WIDTH * 4;
			std::vector<uint8_t> pixels(WIDTH * HEIGHT * 4);
			for (size_t i = 0; i < pixels.size(); i += 4)
			{
				pixels[i + 0] = 255;
				pixels[i + 1] = 128;
				pixels[i + 2] = 0;
				pixels[i + 3] = 255;
			}
			auto upload = device.get_upload_list();
			upload->get_copy().update_texture(src_tex.get(),
				ivec3(0, 0, 0), ivec3(WIDTH, HEIGHT, 1), 0,
				reinterpret_cast<const char*>(pixels.data()), ROW_STRIDE);
			upload->execute_and_wait();
		}

		auto dst_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		// Full-screen triangle that reads each pixel from srcTex via Slots::CopyTexture.
		// The indirect-CBV boilerplate mirrors autogen/CopyTexture.h.
		static constexpr const char* kSigCopyHLSL = R"hlsl(
struct CB { uint offset; };
struct CopyTexture { uint srcTex; };
#ifdef __spirv__
#ifndef HAL_PUSH_DEFINED
#define HAL_PUSH_DEFINED
struct _HALPush { uint s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15; };
[[vk::push_constant]] ConstantBuffer<_HALPush> _hal_push;
#endif
struct _CB_CopyTexture { uint offset; };
static _CB_CopyTexture pass_CopyTexture = { _hal_push.s4 };
#else
ConstantBuffer<CB> pass_CopyTexture : register(b4, space4);
#endif
ConstantBuffer<CopyTexture> CreateCopyTexture() { return ResourceDescriptorHeap[pass_CopyTexture.offset]; }

float4 VS(uint vid : SV_VertexID) : SV_Position
{
    float2 uv = float2((vid & 1) * 2.0, (vid >> 1) * 2.0);
    return float4(uv * 2.0 - 1.0, 0.0, 1.0);
}
float4 PS(float4 pos : SV_Position) : SV_Target
{
    CopyTexture ct = CreateCopyTexture();
    Texture2D<float4> srcTex = ResourceDescriptorHeap[ct.srcTex];
    return srcTex.Load(int3((int)pos.x, (int)pos.y, 0));
}
)hlsl";

		SimpleGraphicsPSO mpso("TestSIGCopy");
		mpso.root_signature = Layouts::DefaultLayout;
		mpso.vertex        = { kSigCopyHLSL, "VS", HAL::ShaderOptions::None, {}, true };
		mpso.pixel         = { kSigCopyHLSL, "PS", HAL::ShaderOptions::None, {}, true };
		mpso.rtv_formats   = { HAL::Format::R8G8B8A8_UNORM };
		mpso.enable_depth  = false;
		mpso.cull          = HAL::CullMode::None;
		mpso.topology      = HAL::PrimitiveTopologyType::TRIANGLE;
		auto pso = mpso.create(device);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"SIGCopyTexture");

		HAL::Texture2DView src_view(src_tex, *list);
		HAL::Texture2DView dst_view(dst_tex, *list);
		HAL::CompiledRT compiled_rt;
		compiled_rt.table_rtv = dst_view.renderTarget;

		Slots::CopyTexture copy_slot;
		copy_slot.GetSrcTex() = src_view.texture2D;

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled_rt, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline(pso);
		gfx.set(copy_slot);
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(3);

		list->execute_and_wait();

		ASSERT_TEXTURE(dst_tex.get(), "sig_copy_texture");
	}
}
