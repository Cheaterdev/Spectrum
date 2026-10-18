export module Test.HAL.SIG;

#define TEST_MODULE_ID HALSIG

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import RenderSystem;

export namespace Test
{
	// Verifies that Slots::Editor::Color (a plain float4 CBV) round-trips through the
	// Frame::DefaultLayout SIG pipeline: compile the slot via the graphics context, bind
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

		// Full-screen triangle that reads the Color slot and outputs it (test/sig_color.hlsl through the
		// generated autogen/Color.h, PSOS::Tests::TestSIGColor): the binding follows the slot layout by
		// itself instead of a hand-copied register/push index.
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"SIGColor");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		Slots::Editor::Color color_slot;
		color_slot.GetColor() = float4(0.4f, 0.2f, 0.8f, 1.0f);

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline<PSOS::Tests::TestSIGColor>();
		gfx.set(color_slot);
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(3);

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "sig_color");
	}

	// Verifies the SRV (Texture2D) path of the SIG system: upload a solid-color
	// source texture, bind it via Slots::Utility::CopyTexture, render a full-screen quad
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

		// Full-screen triangle that reads each pixel from srcTex via Slots::Utility::CopyTexture
		// (test/sig_copy_texture.hlsl through the generated autogen/CopyTexture.h,
		// PSOS::Tests::TestSIGCopy).
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"SIGCopyTexture");

		HAL::Texture2DView src_view(src_tex, *list);
		HAL::Texture2DView dst_view(dst_tex, *list);
		HAL::CompiledRT compiled_rt;
		compiled_rt.table_rtv = dst_view.renderTarget;

		Slots::Utility::CopyTexture copy_slot;
		copy_slot.GetSrcTex() = src_view.texture2D;

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled_rt, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline<PSOS::Tests::TestSIGCopy>();
		gfx.set(copy_slot);
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
		gfx.draw(3);

		list->execute_and_wait();

		ASSERT_TEXTURE(dst_tex.get(), "sig_copy_texture");
	}
}
