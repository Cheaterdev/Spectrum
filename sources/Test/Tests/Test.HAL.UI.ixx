export module Test.HAL.UI;

#define TEST_MODULE_ID HALUI

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import RenderSystem;

export namespace Test
{
	// Render a single centered rectangle via the production SimpleRect PSO and
	// Slots::ColorRect.  Positions are clip-space float2 pairs packed into
	// float4[2]: pos[0] = (v0.x, v0.y, v1.x, v1.y), pos[1] = (v2.x, v2.y, v3.x, v3.y).
	// Draw(4) with TRIANGLE_STRIP forms two triangles that cover the quad.
	TEST(Core.HAL, UIRect_Solid)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 256, HEIGHT = 256;

		// SimpleRect PSO requires B8G8R8A8_UNORM
		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"UIRect_Solid");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		// A centered 50%-of-screen green rectangle (clip-space ±0.5)
		Slots::ColorRect rect_slot;
		auto* p = rect_slot.GetPos();
		p[0] = float4(-0.5f,  0.5f,  0.5f,  0.5f); // v0=TL, v1=TR
		p[1] = float4(-0.5f, -0.5f,  0.5f, -0.5f); // v2=BL, v3=BR
		rect_slot.GetColor()[0] = float4(0.2f, 0.8f, 0.2f, 1.0f);
 		rect_slot.GetColor()[1] = float4(0.2f, 0.8f, 0.2f, 1.0f);
		rect_slot.GetColor()[2] = float4(0.2f, 0.8f, 0.2f, 1.0f);
		rect_slot.GetColor()[3] = float4(0.2f, 0.8f, 0.2f, 1.0f);

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline<PSOS::SimpleRect>();
		gfx.set(rect_slot);
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
		gfx.draw(4);

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "ui_rect_solid");
	}


	// Render two overlapping rectangles to verify alpha blending.
	// Pass 1: full-screen opaque blue (0,0,1,1) — establishes the base color.
	// Pass 2: right-half semi-transparent red (1,0,0,0.6) — blends over it.
	// Expected result: left half = pure blue, right half = (0.6, 0, 0.4, 1.0).
	TEST(Core.HAL, UIRect_AlphaBlend)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 256, HEIGHT = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"UIRect_AlphaBlend");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline<PSOS::SimpleRect>();
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

		// Pass 1: opaque blue full-screen quad
		{
			Slots::ColorRect blue;
			auto* p = blue.GetPos();
			p[0] = float4(-1.0f,  1.0f,  1.0f,  1.0f);
			p[1] = float4(-1.0f, -1.0f,  1.0f, -1.0f);
			blue.GetColor()[0] = float4(0.0f, 0.0f, 1.0f, 1.0f);
			blue.GetColor()[1] = float4(0.0f, 0.0f, 1.0f, 1.0f);
			blue.GetColor()[2] = float4(0.0f, 0.0f, 1.0f, 1.0f);
			blue.GetColor()[3] = float4(0.0f, 0.0f, 1.0f, 1.0f);
			gfx.set(blue);
			gfx.draw(4);
		}

		// Pass 2: semi-transparent red over the right half
		{
			Slots::ColorRect red;
			auto* p = red.GetPos();
			p[0] = float4(0.0f,  1.0f,  1.0f,  1.0f);
			p[1] = float4(0.0f, -1.0f,  1.0f, -1.0f);
			red.GetColor()[0] = float4(1.0f, 0.0f, 0.0f, 0.6f);
			red.GetColor()[1] = float4(1.0f, 0.0f, 0.0f, 0.6f);
			red.GetColor()[2] = float4(1.0f, 0.0f, 0.0f, 0.6f);
			red.GetColor()[3] = float4(1.0f, 0.0f, 0.0f, 0.6f);
			gfx.set(red);
			gfx.draw(4);
		}

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "ui_rect_alpha_blend");
	}

	// Render three non-overlapping rectangles in different positions to verify
	// that clip-space coordinates map to the correct screen regions.
	TEST(Core.HAL, UIRect_Layout)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 256, HEIGHT = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"UIRect_Layout");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		auto& gfx = list->get_graphics();
		gfx.set_rtv(compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0.05f, 0.05f, 0.1f, 1.0f));
		gfx.set_pipeline<PSOS::SimpleRect>();
		gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);

		// Left-third red
		{
			Slots::ColorRect r;
			auto* p = r.GetPos();
			p[0] = float4(-1.0f,  0.8f, -0.35f,  0.8f);
			p[1] = float4(-1.0f, -0.8f, -0.35f, -0.8f);
			r.GetColor()[0] = float4(0.9f, 0.2f, 0.2f, 1.0f);		 
			r.GetColor()[1] = float4(0.9f, 0.2f, 0.2f, 1.0f);
			r.GetColor()[2] = float4(0.9f, 0.2f, 0.2f, 1.0f);
			r.GetColor()[3] = float4(0.9f, 0.2f, 0.2f, 1.0f);
			gfx.set(r); gfx.draw(4);
		}

		// Center green
		{
			Slots::ColorRect r;
			auto* p = r.GetPos();
			p[0] = float4(-0.3f,  0.8f,  0.3f,  0.8f);
			p[1] = float4(-0.3f, -0.8f,  0.3f, -0.8f);
			r.GetColor()[0] = float4(0.2f, 0.9f, 0.2f, 1.0f);
			r.GetColor()[1] = float4(0.2f, 0.9f, 0.2f, 1.0f);
			r.GetColor()[2] = float4(0.2f, 0.9f, 0.2f, 1.0f);
			r.GetColor()[3] = float4(0.2f, 0.9f, 0.2f, 1.0f);
			gfx.set(r); gfx.draw(4);
		}

		// Right-third blue
		{
			Slots::ColorRect r;
			auto* p = r.GetPos();
			p[0] = float4(0.35f,  0.8f,  1.0f,  0.8f);
			p[1] = float4(0.35f, -0.8f,  1.0f, -0.8f);
			r.GetColor()[0] = float4(0.2f, 0.2f, 0.9f, 1.0f);
			r.GetColor()[1] = float4(0.2f, 0.2f, 0.9f, 1.0f);
			r.GetColor()[2] = float4(0.2f, 0.2f, 0.9f, 1.0f);
			r.GetColor()[3] = float4(0.2f, 0.2f, 0.9f, 1.0f);
			gfx.set(r); gfx.draw(4);
		}

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "ui_rect_layout");
	}
}
