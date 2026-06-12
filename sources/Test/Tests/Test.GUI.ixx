export module Test.GUI;

#define TEST_MODULE_ID GUI

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import GUI;
import FrameGraph;

export namespace Test
{
	// Verifies that process_ui() propagates render_bounds to a fill-docked child.
	// Pure layout check — no GPU work required.
	TEST(Core.HAL, GUILayout_Fill)
	{
			THREAD_SCOPE(GUI);
		GUI::user_interface ui;
		ui.size = vec2(256.0f, 256.0f);

		auto cr = std::make_shared<GUI::Elements::colored_rect>();
		cr->color = float4(1.0f, 0.0f, 0.0f, 1.0f);
		cr->docking = GUI::dock::FILL;
		ui.add_child(cr);
		ui.process_ui(0.0f);

		rect b = cr->get_render_bounds();
		ASSERT_TRUE(b.x == 0.0f);
		ASSERT_TRUE(b.y == 0.0f);
		ASSERT_TRUE(b.w == 256.0f);
		ASSERT_TRUE(b.h == 256.0f);
	}

	// Verifies that dock::TOP elements consume space from the top and dock::FILL
	// elements get the remaining area — non-overlapping horizontal bands.
	TEST(Core.HAL, GUILayout_DockedBands)
	{
			THREAD_SCOPE(GUI);
		GUI::user_interface ui;
		ui.size = vec2(256.0f, 256.0f);

		auto top_rect = std::make_shared<GUI::Elements::colored_rect>();
		top_rect->color      = float4(1.0f, 0.0f, 0.0f, 1.0f);
		top_rect->docking    = GUI::dock::TOP;
		top_rect->size       = vec2(256.0f, 80.0f);
		top_rect->width_size = GUI::size_type::MATCH_PARENT;
		top_rect->height_size = GUI::size_type::FIXED;

		auto fill_rect = std::make_shared<GUI::Elements::colored_rect>();
		fill_rect->color   = float4(0.0f, 1.0f, 0.0f, 1.0f);
		fill_rect->docking = GUI::dock::FILL;

		ui.add_child(top_rect);
		ui.add_child(fill_rect);
		ui.process_ui(0.0f);

		rect tb = top_rect->get_render_bounds();
		rect fb = fill_rect->get_render_bounds();

		ASSERT_TRUE(tb.y == 0.0f);
		ASSERT_TRUE(tb.h == 80.0f);
		ASSERT_TRUE(fb.y == 80.0f);
		ASSERT_TRUE(fb.h == 176.0f);
	}

	// Tests GUI::Renderer::draw_color: renders a centered rect in pixel space and
	// verifies the result via golden image comparison.  Pixel (64,64)→(192,192) in
	// a 256×256 window maps to clip ±0.5 — the inner half of the screen.
	TEST(Core.HAL, GUIRenderer_DrawColor)
	{
			THREAD_SCOPE(GUI);
		auto& device = HAL::Device::get();
		constexpr uint WIDTH = 256, HEIGHT = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUIRenderer_DrawColor");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled_rt;
		compiled_rt.table_rtv = view.renderTarget;
		list->get_graphics().set_rtv(compiled_rt,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
			vec4(0.1f, 0.1f, 0.2f, 1.0f));

		sizer_long full_vp;
		full_vp.left   = 0;
		full_vp.top    = 0;
		full_vp.right  = (long)WIDTH;
		full_vp.bottom = (long)HEIGHT;

		GUI::Renderer renderer;
		GUIInfo c;
		c.renderer     = &renderer;
		c.command_list = list;
		c.window_size  = vec2((float)WIDTH, (float)HEIGHT);
		c.offset       = vec2(0.0f, 0.0f);
		c.ui_clipping  = full_vp;
		c.scissors     = full_vp;
		c.scale        = 1.0f;
		c.delta_time   = 0.0f;

		renderer.draw_color(c, float4(0.8f, 0.3f, 0.1f, 1.0f),
			rect{ vec2(64.0f, 64.0f), vec2(128.0f, 128.0f) });
		renderer.flush(c);

		list->execute_and_wait();
		ASSERT_TEXTURE(tex.get(), "gui_renderer_draw_color");
	}

	// Tests colored_rect::draw through the full element dispatch path:
	// user_interface layout → colored_rect::draw → Renderer::draw_color → PSOS::SimpleRect.
	TEST(Core.HAL, GUIElement_ColoredRect)
	{
			THREAD_SCOPE(GUI);
		auto& device = HAL::Device::get();
		constexpr uint WIDTH = 256, HEIGHT = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		GUI::user_interface ui;
		ui.size = vec2((float)WIDTH, (float)HEIGHT);

		auto cr = std::make_shared<GUI::Elements::colored_rect>();
		cr->color   = float4(0.2f, 0.7f, 0.4f, 1.0f);
		cr->docking = GUI::dock::FILL;
		ui.add_child(cr);
		ui.process_ui(0.0f);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUIElement_ColoredRect");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled_rt;
		compiled_rt.table_rtv = view.renderTarget;
		list->get_graphics().set_rtv(compiled_rt,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
			vec4(0.1f, 0.1f, 0.2f, 1.0f));

		sizer_long full_vp;
		full_vp.left   = 0;
		full_vp.top    = 0;
		full_vp.right  = (long)WIDTH;
		full_vp.bottom = (long)HEIGHT;

		GUI::Renderer renderer;
		GUIInfo c;
		c.renderer     = &renderer;
		c.command_list = list;
		c.window_size  = vec2((float)WIDTH, (float)HEIGHT);
		c.offset       = vec2(0.0f, 0.0f);
		c.ui_clipping  = full_vp;
		c.scissors     = full_vp;
		c.scale        = 1.0f;
		c.delta_time   = 0.0f;

		cr->draw(c);
		renderer.flush(c);

		list->execute_and_wait();
		ASSERT_TEXTURE(tex.get(), "gui_element_colored_rect");
	}

	// Tests that three docked bands render as distinct horizontal stripes with
	// correct pixel boundaries, verifying both layout and rendering together.
	TEST(Core.HAL, GUIElement_ThreeBands)
	{
			THREAD_SCOPE(GUI);
		auto& device = HAL::Device::get();
		constexpr uint WIDTH = 256, HEIGHT = 240;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		GUI::user_interface ui;
		ui.size = vec2((float)WIDTH, (float)HEIGHT);

		struct Band { float4 color; float height; };
		constexpr Band bands[] = {
			{ float4(0.85f, 0.15f, 0.15f, 1.0f), 80.0f },
			{ float4(0.15f, 0.80f, 0.15f, 1.0f), 80.0f },
			{ float4(0.15f, 0.15f, 0.85f, 1.0f), 80.0f },
		};

		std::vector<std::shared_ptr<GUI::Elements::colored_rect>> rects;
		for (auto& b : bands)
		{
			auto r = std::make_shared<GUI::Elements::colored_rect>();
			r->color       = b.color;
			r->docking     = GUI::dock::TOP;
			r->size        = vec2((float)WIDTH, b.height);
			r->width_size  = GUI::size_type::MATCH_PARENT;
			r->height_size = GUI::size_type::FIXED;
			ui.add_child(r);
			rects.push_back(r);
		}
		ui.process_ui(0.0f);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUIElement_ThreeBands");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled_rt;
		compiled_rt.table_rtv = view.renderTarget;
		list->get_graphics().set_rtv(compiled_rt,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
			vec4(0.05f, 0.05f, 0.1f, 1.0f));

		sizer_long full_vp;
		full_vp.left   = 0;
		full_vp.top    = 0;
		full_vp.right  = (long)WIDTH;
		full_vp.bottom = (long)HEIGHT;

		GUI::Renderer renderer;
		GUIInfo c;
		c.renderer     = &renderer;
		c.command_list = list;
		c.window_size  = vec2((float)WIDTH, (float)HEIGHT);
		c.offset       = vec2(0.0f, 0.0f);
		c.ui_clipping  = full_vp;
		c.scissors     = full_vp;
		c.scale        = 1.0f;
		c.delta_time   = 0.0f;

		for (auto& r : rects)
			r->draw(c);
		renderer.flush(c);

		list->execute_and_wait();
		ASSERT_TEXTURE(tex.get(), "gui_element_three_bands");
	}

	// Renders a single label into a 256x64 texture via the full production path:
	// process_ui → create_graph (pre-renders glyphs) → draw_infos draw loop.
	// Isolates label cache creation and NinePatch rendering from all other widgets.
	TEST(Core.HAL, GUIElement_Label)
	{
		THREAD_SCOPE(GUI);
		auto& device = HAL::Device::get();
		constexpr uint W = 256, H = 64;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		GUI::user_interface ui;
		ui.size = vec2((float)W, (float)H);

		auto lbl = std::make_shared<GUI::Elements::label>();
		lbl->text    = "Hello Label";
		lbl->docking = GUI::dock::FILL;
		ui.add_child(lbl);

		ui.process_ui(0.0f);

		FrameGraph::Graph graph;
		ui.create_graph(graph);
		device.get_queue(HAL::CommandListType::DIRECT)->signal_and_wait();

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUIElement_Label");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled_rt;
		compiled_rt.table_rtv = view.renderTarget;
		list->get_graphics().set_rtv(compiled_rt,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
			vec4(0.1f, 0.1f, 0.2f, 1.0f));

		sizer_long full_vp;
		full_vp.left   = 0;
		full_vp.top    = 0;
		full_vp.right  = (long)W;
		full_vp.bottom = (long)H;
		list->get_graphics().set_scissors(full_vp);

		GUI::Renderer renderer;
		GUIInfo c;
		c.renderer     = &renderer;
		c.command_list = list;
		c.window_size  = vec2((float)W, (float)H);
		c.offset       = vec2(0.0f, 0.0f);
		c.ui_clipping  = full_vp;
		c.scissors     = full_vp;
		c.scale        = 1.0f;
		c.delta_time   = 0.0f;

		auto& ui_ctx = graph.get_context<GUI::UIContext>();
		for (auto& e : ui_ctx.draw_infos)
		{
			if (c.scissors != e.scissors)
			{
				renderer.flush(c);
				list->get_graphics().set_scissors(e.scissors);
			}
			c.scale       = e.scale;
			c.ui_clipping = e.clip;
			c.offset      = e.offset;
			c.scissors    = e.scissors;
			c.window_size = ui_ctx.scaled_size;

			if (e.before)
				e.elem->draw(c);
			else
				e.elem->draw_after(c);
		}
		renderer.flush(c);

		list->execute_and_wait();
		ASSERT_TEXTURE(tex.get(), "gui_element_label");
	}

	// Renders a realistic multi-widget UI into an 800x600 texture.
	// Exercises: colored_rect background, label, button row, check_box_text
	// panel, status_bar.  Uses create_graph to pre-render label glyphs via
	// the production path, then iterates UIContext.draw_infos exactly as
	// PassDefault<UI_Render>::render does.
	TEST(Core.HAL, GUIFullScreen)
	{
		THREAD_SCOPE(GUI);

		constexpr uint W = 800, H = 600;
		auto& device = HAL::Device::get();

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		GUI::user_interface ui;
		ui.size = vec2((float)W, (float)H);

		// Full-screen background (dock::NONE + MATCH_PARENT — layer 0)
		{
			auto bg = std::make_shared<GUI::Elements::colored_rect>();
			bg->color       = float4(0.11f, 0.13f, 0.19f, 1.0f);
			bg->width_size  = GUI::size_type::MATCH_PARENT;
			bg->height_size = GUI::size_type::MATCH_PARENT;
			ui.add_child(bg);
		}

		// Title bar (dock TOP, 36px)
		{
			auto panel = std::make_shared<GUI::base>();
			panel->docking     = GUI::dock::TOP;
			panel->height_size = GUI::size_type::FIXED;
			panel->size        = vec2(0.0f, 36.0f);

			auto lbl = std::make_shared<GUI::Elements::label>();
			lbl->text    = "Spectrum UI Test";
			lbl->docking = GUI::dock::FILL;
			panel->add_child(lbl);
			ui.add_child(panel);
		}

		// Button row (dock TOP, 40px)
		{
			auto row = std::make_shared<GUI::base>();
			row->docking     = GUI::dock::TOP;
			row->height_size = GUI::size_type::FIXED;
			row->size        = vec2(0.0f, 40.0f);

			for (const char* name : {"File", "Edit", "View"})
			{
				auto btn = std::make_shared<GUI::Elements::button>();
				btn->get_label()->text = name;
				btn->docking           = GUI::dock::LEFT;
				btn->width_size        = GUI::size_type::FIXED;
				btn->size              = vec2(80.0f, 0.0f);
				row->add_child(btn);
			}
			ui.add_child(row);
		}

		// Status bar (dock BOTTOM)
		{
			auto bar = std::make_shared<GUI::Elements::status_bar>();
			auto lbl = std::make_shared<GUI::Elements::label>();
			lbl->text = "Ready";
			bar->add_child(lbl);
			ui.add_child(bar);
		}

		// Left options panel (dock LEFT, 180px)
		{
			auto panel = std::make_shared<GUI::base>();
			panel->docking    = GUI::dock::LEFT;
			panel->width_size = GUI::size_type::FIXED;
			panel->size       = vec2(180.0f, 0.0f);

			auto hdr = std::make_shared<GUI::Elements::label>();
			hdr->text        = "Options";
			hdr->docking     = GUI::dock::TOP;
			hdr->height_size = GUI::size_type::FIXED;
			hdr->size        = vec2(0.0f, 28.0f);
			panel->add_child(hdr);

			for (const char* opt : {"Shadows", "Reflections", "Voxel GI"})
			{
				auto cbt = std::make_shared<GUI::Elements::check_box_text>();
				cbt->get_label()->text = opt;
				cbt->docking           = GUI::dock::TOP;
				cbt->height_size       = GUI::size_type::FIXED;
				cbt->size              = vec2(0.0f, 28.0f);
				panel->add_child(cbt);
			}
			ui.add_child(panel);
		}

		// Fill area (dock FILL) — blue-tinted content area
		{
			auto fill = std::make_shared<GUI::Elements::colored_rect>();
			fill->color   = float4(0.12f, 0.20f, 0.36f, 1.0f);
			fill->docking = GUI::dock::FILL;
			ui.add_child(fill);
		}

		// Layout pass
		ui.process_ui(0.0f);

		// Populate UIContext.draw_infos + async pre-render of label glyphs
		FrameGraph::Graph graph;
		ui.create_graph(graph);
		device.get_queue(HAL::CommandListType::DIRECT)->signal_and_wait();

		// GPU render pass
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUIFullScreen");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled_rt;
		compiled_rt.table_rtv = view.renderTarget;
		list->get_graphics().set_rtv(compiled_rt,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
			vec4(0.05f, 0.05f, 0.1f, 1.0f));

		sizer_long full_vp;
		full_vp.left   = 0;
		full_vp.top    = 0;
		full_vp.right  = (long)W;
		full_vp.bottom = (long)H;
		list->get_graphics().set_scissors(full_vp);

		GUI::Renderer renderer;
		GUIInfo c;
		c.renderer     = &renderer;
		c.command_list = list;
		c.window_size  = vec2((float)W, (float)H);
		c.offset       = vec2(0.0f, 0.0f);
		c.ui_clipping  = full_vp;
		c.scissors     = full_vp;
		c.scale        = 1.0f;
		c.delta_time   = 0.0f;

		auto& ui_ctx = graph.get_context<GUI::UIContext>();
		for (auto& e : ui_ctx.draw_infos)
		{
			if (c.scissors != e.scissors)
			{
				renderer.flush(c);
				list->get_graphics().set_scissors(e.scissors);
			}
			c.scale       = e.scale;
			c.ui_clipping = e.clip;
			c.offset      = e.offset;
			c.scissors    = e.scissors;
			c.window_size = ui_ctx.scaled_size;

			if (e.before)
				e.elem->draw(c);
			else
				e.elem->draw_after(c);
		}
		renderer.flush(c);

		list->execute_and_wait();
		ASSERT_TEXTURE(tex.get(), "gui_full_screen");
	}

	// Renders a 32x32 nine-patch source texture into a 256x128 target at three
	// different rect sizes to verify that corners stay fixed (8px) while the
	// edges and center stretch correctly.
	// Source color layout: RED 8x8 corners, GREEN top/bottom edges,
	// BLUE left/right edges, WHITE center (R8G8B8A8_UNORM).
	TEST(Core.HAL, GUINinePatch_Stretch)
	{
		THREAD_SCOPE(GUI);
		auto& device = HAL::Device::get();
		constexpr uint RT_W = 256, RT_H = 128;
		constexpr uint TEX_W = 32, TEX_H = 32;
		constexpr uint CORNER = 8;

		// Build 32x32 source texture: RED corners, GREEN top/bottom edges,
		// BLUE left/right edges, WHITE center.
		auto src_tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {TEX_W, TEX_H}, 1, 1),
			HAL::HeapType::DEFAULT);

		constexpr uint ROW_STRIDE = TEX_W * 4;
		std::vector<uint8_t> pixels(TEX_W * TEX_H * 4);
		for (uint y = 0; y < TEX_H; ++y)
		{
			for (uint x = 0; x < TEX_W; ++x)
			{
				bool cx = (x < CORNER || x >= TEX_W - CORNER);
				bool cy = (y < CORNER || y >= TEX_H - CORNER);
				uint8_t r, g, b;
				if (cx && cy)       { r=255; g=0;   b=0;   } // RED corners
				else if (!cx && cy) { r=0;   g=255; b=0;   } // GREEN top/bottom edges
				else if (cx && !cy) { r=0;   g=0;   b=255; } // BLUE left/right edges
				else                { r=255; g=255; b=255; } // WHITE center
				size_t idx = ((size_t)y * TEX_W + x) * 4;
				pixels[idx+0]=r; pixels[idx+1]=g; pixels[idx+2]=b; pixels[idx+3]=255;
			}
		}
		{
			auto upload = device.get_upload_list();
			upload->get_copy().update_texture(src_tex.get(), ivec3(0,0,0), ivec3(TEX_W,TEX_H,1), 0,
				reinterpret_cast<const char*>(pixels.data()), ROW_STRIDE);
			upload->execute_and_wait();
		}

		auto rt = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {RT_W, RT_H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUINinePatch_Stretch");

		HAL::Texture2DView src_view(src_tex, *list);
		HAL::Texture2DView rt_view(rt, *list);

		HAL::CompiledRT compiled_rt;
		compiled_rt.table_rtv = rt_view.renderTarget;
		list->get_graphics().set_rtv(compiled_rt,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
			vec4(0.05f, 0.05f, 0.1f, 1.0f));

		sizer_long full_vp;
		full_vp.left   = 0;
		full_vp.top    = 0;
		full_vp.right  = (long)RT_W;
		full_vp.bottom = (long)RT_H;
		list->get_graphics().set_scissors(full_vp);

		GUIInfo c;
		c.renderer     = nullptr;
		c.command_list = list;
		c.window_size  = vec2((float)RT_W, (float)RT_H);
		c.offset       = vec2(0.0f, 0.0f);
		c.ui_clipping  = full_vp;
		c.scissors     = full_vp;
		c.scale        = 1.0f;
		c.delta_time   = 0.0f;

		GUI::Texture item;
		item          = src_view;
		item.padding  = sizer(8.0f, 8.0f, 8.0f, 8.0f);

		GUI::NinePatch nine_patch;
		// Small (40x40): corners should occupy most of the patch
		nine_patch.draw(c, item, rect{ vec2(4.0f, 4.0f), vec2(40.0f, 40.0f) });
		// Medium (120x60): balanced stretch
		nine_patch.draw(c, item, rect{ vec2(52.0f, 4.0f), vec2(120.0f, 60.0f) });
		// Wide (240x48): heavy horizontal stretch
		nine_patch.draw(c, item, rect{ vec2(4.0f, 72.0f), vec2(240.0f, 48.0f) });
		nine_patch.flush(c);

		list->execute_and_wait();
		ASSERT_TEXTURE(rt.get(), "gui_nine_patch_stretch");
	}
}
