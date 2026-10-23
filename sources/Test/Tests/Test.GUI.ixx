export module Test.GUI;

#define TEST_MODULE_ID GUI

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import RenderSystem;
import GUI;
import FrameGraph;
import TextEngine;

// -----------------------------------------------------------------------
// Clipping/culling helpers shared by the NinePatch clip tests.
//
// Source: 32x32 with 8px corners (RED corners, GREEN h-edges,
//         BLUE v-edges, WHITE center). Padding 8px on all sides.
// Patch:  160x80 placed at (48,24) inside a 256x128 RT.
//   Corner X boundaries in screen space: 48, 56, 200, 208
//   Corner Y boundaries in screen space: 24, 32,  96, 104
// -----------------------------------------------------------------------
namespace
{
	using Test::UI_RT_FORMAT;
	using Test::set_ui_target;

	// Records what the touch layer sends it, and whether any mouse handler ran (it must not).
	struct touch_probe : GUI::base
	{
		std::string actions;       // "DOWN UP", "DOWN CANCEL", ...
		int  moves = 0;
		int  long_presses = 0;
		int  mouse_calls = 0;
		bool accept = true;        // what on_touch_action returns for DOWN

		bool on_touch_action(mouse_action action, GUI::touch& t) override
		{
			actions += (actions.empty() ? "" : " ");
			actions += action == mouse_action::DOWN ? "DOWN" : action == mouse_action::UP ? "UP" : "CANCEL";
			return accept;
		}

		void on_touch_move(std::vector<GUI::touch*>& e) override
		{
			++moves;
			e[0]->catched = false;
		}

		bool on_long_pressed(const GUI::touch&) override
		{
			++long_presses;
			return true;
		}

		bool on_mouse_action(mouse_action, mouse_button, vec2) override { ++mouse_calls; return true; }
		bool on_mouse_move(vec2) override { ++mouse_calls; return true; }
	};

	GUI::touch_event make_touch(int flags, int id, float x, float y)
	{
		GUI::touch_event e;
		e.flags = flags;
		e.touches.push_back({ id, x, y });
		return e;
	}

	std::shared_ptr<HAL::TextureResource> make_ninepatch_src(HAL::Device& device)
	{
		constexpr uint TEX_W = 32, TEX_H = 32, CORNER = 8;
		auto src = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {TEX_W, TEX_H}, 1, 1),
			HAL::HeapType::DEFAULT);

		std::vector<uint8_t> px(TEX_W * TEX_H * 4);
		for (uint y = 0; y < TEX_H; ++y)
			for (uint x = 0; x < TEX_W; ++x)
			{
				bool cx = (x < CORNER || x >= TEX_W - CORNER);
				bool cy = (y < CORNER || y >= TEX_H - CORNER);
				uint8_t r, g, b;
				if      ( cx &&  cy) { r=255; g=0;   b=0;   }
				else if (!cx &&  cy) { r=0;   g=255; b=0;   }
				else if ( cx && !cy) { r=0;   g=0;   b=255; }
				else                 { r=255; g=255; b=255; }
				size_t i = ((size_t)y * TEX_W + x) * 4;
				px[i]=r; px[i+1]=g; px[i+2]=b; px[i+3]=255;
			}

		auto upload = device.get_upload_list();
		upload->get_copy().update_texture(src.get(), ivec3(0,0,0), ivec3(TEX_W,TEX_H,1), 0,
			reinterpret_cast<const char*>(px.data()), TEX_W * 4);
		upload->execute_and_wait();
		return src;
	}

	std::shared_ptr<HAL::TextureResource> run_clip_test(
		HAL::Device& device, sizer_long clip, LiteralWStr label)
	{
		constexpr uint RT_W = 256, RT_H = 128;
		auto src = make_ninepatch_src(device);

		auto rt = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {RT_W, RT_H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(label);

		HAL::Texture2DView src_view(src, *list);
		HAL::Texture2DView rt_view(rt, *list);

		set_ui_target(list, rt_view, vec4(0.05f, 0.05f, 0.1f, 1.0f));

		sizer_long full_vp{ 0, 0, (long)RT_W, (long)RT_H };
		list->get_graphics().set_scissors(full_vp);

		GUIInfo c;
		c.renderer     = nullptr;
		c.command_list = list;
		c.window_size  = vec2((float)RT_W, (float)RT_H);
		c.offset       = vec2(0.0f, 0.0f);
		c.ui_clipping  = clip;
		c.scissors     = full_vp;
		c.scale        = 1.0f;
		c.delta_time   = 0.0f;

		GUI::Texture item;
		item         = src_view;
		item.padding = sizer(8.0f, 8.0f, 8.0f, 8.0f);

		GUI::NinePatch nine_patch;
		nine_patch.draw(c, item, rect{ vec2(48.0f, 24.0f), vec2(160.0f, 80.0f) });
		nine_patch.flush(c);

		list->execute_and_wait();
		return rt;
	}

	// Red channel of a read-back R16G16B16A16_FLOAT target.
	struct fx_image
	{
		HAL::texture_data::ptr data;

		float at(uint x, uint y, uint channel = 0) const
		{
			const auto& mip = *data->array[0]->mips[0];
			const uint16_t* row = reinterpret_cast<const uint16_t*>(mip.data.data() + size_t(y) * mip.width_stride);
			const uint32_t h = row[x * 4 + channel];
			const uint32_t e = (h >> 10) & 0x1f, m = h & 0x3ff;
			return e == 0 ? std::ldexp(float(m), -24) : std::ldexp(float(m | 0x400), int(e) - 25);
		}
	};

	// A flat grey rect at (64,32) 128x64 on black, then draw_fx over it.
	fx_image render_fx(const GUI::FxParams& params)
	{
		constexpr uint W = 256, H = 128;
		auto& device = RenderSystem::get().device();

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {W, H}, 1, 1, HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"render_fx");

		HAL::Texture2DView view(tex, *list);
		set_ui_target(list, view, vec4(0.0f, 0.0f, 0.0f, 1.0f));

		sizer_long full_vp;
		full_vp.left   = 0;
		full_vp.top    = 0;
		full_vp.right  = (long)W;
		full_vp.bottom = (long)H;

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

		const rect shape{ vec2(64.0f, 32.0f), vec2(128.0f, 64.0f) };
		renderer.draw_color(c, float4(0.2f, 0.2f, 0.2f, 1.0f), shape);
		renderer.draw_fx(c, shape, params);
		renderer.flush(c);

		list->execute_and_wait();
		return { Test::readback_texture(tex.get()) };
	}

	// Lays out and draws a whole user_interface the way UI_Render does (create_graph,
	// the pre-draw pass, then the draw list in one command list) into a W x H
	// target cleared to black, and reads it back.
	// advance_dt >= 0 lays the NEXT frame out between building the draw list and
	// drawing it, which is what the app does when the GUI thread runs ahead of the
	// frame still being recorded: elements are drawn at newer bounds than the
	// clip rectangles captured in the draw list.
	fx_image render_ui(GUI::user_interface& ui, uint W, uint H, double* record_and_execute_ms = nullptr, double* record_ms = nullptr, float advance_dt = -1.0f)
	{
		auto& device = RenderSystem::get().device();

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {W, H}, 1, 1, HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		ui.process_ui(0.0f);

		FrameGraph::Graph graph;
		ui.create_graph(graph);
		device.get_queue(HAL::CommandListType::DIRECT)->signal_and_wait();

		auto& predraw = graph.get_context<GUI::UIContext>();
		if (!predraw.pre_draw_infos.empty())
		{
			auto pre_draw_list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
			pre_draw_list->begin(L"pre_draw");
			GUI::user_interface::run_pre_draw(predraw.pre_draw_infos, pre_draw_list);
			pre_draw_list->end();
			pre_draw_list->execute_and_wait();
		}

		const auto timer_start = std::chrono::steady_clock::now();

		if (advance_dt >= 0.0f)
			ui.process_ui(advance_dt);

		auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"render_ui");

		HAL::Texture2DView view(tex, *list);
		set_ui_target(list, view, vec4(0.0f, 0.0f, 0.0f, 1.0f));

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
		c.target       = view;
		c.ui_scale     = 1.0f;
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
			c.scale         = e.scale;
			c.ui_clipping   = e.clip;
			c.offset        = e.offset;
			c.scissors      = e.scissors;
			c.window_size   = ui_ctx.scaled_size;
			c.layer         = e.layer;
			c.layer_slot    = e.layer_slot;
			c.layer_opacity = e.layer_opacity;

			GUI::draw_bounds_scope bounds_scope(e.elem, e.bounds);

			if (e.before)
				e.elem->draw_before(c);
			else
				e.elem->draw_end(c);
		}
		renderer.flush(c);

		if (record_ms)
			*record_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - timer_start).count();

		list->execute_and_wait();

		if (record_and_execute_ms)
			*record_and_execute_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - timer_start).count();

		return { Test::readback_texture(tex.get()) };
	}
}

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

	// Touch is its own input path: a finger pressed and lifted inside an element gives it DOWN then UP,
	// and no mouse handler runs. Pure CPU, like the layout tests.
	TEST(Core.HAL, GUITouch_TapInside)
	{
		THREAD_SCOPE(GUI);
		// Owned by a shared_ptr, as in the app: the touch path walks up to parents via shared_from_this.
		auto ui_owner = std::make_shared<GUI::user_interface>();
		GUI::user_interface& ui = *ui_owner;
		ui.size = vec2(256.0f, 256.0f);

		auto probe = std::make_shared<touch_probe>();
		probe->docking = GUI::dock::FILL;
		ui.add_child(probe);
		ui.process_ui(0.0f);

		ASSERT_TRUE(ui.on_touch_event(make_touch(GUI::touch_event::DOWN, 7, 100.0f, 100.0f)));
		ASSERT_TRUE(ui.on_touch_event(make_touch(GUI::touch_event::UP, 7, 100.0f, 100.0f)));

		ASSERT_TRUE(probe->actions == "DOWN UP");
		ASSERT_TRUE(probe->mouse_calls == 0);
	}

	// A finger that slides off the element before lifting cancels the press instead of activating it,
	// and the element still sees the move.
	TEST(Core.HAL, GUITouch_SlideOffCancels)
	{
		THREAD_SCOPE(GUI);
		auto ui_owner = std::make_shared<GUI::user_interface>();
		GUI::user_interface& ui = *ui_owner;
		ui.size = vec2(256.0f, 256.0f);

		auto probe = std::make_shared<touch_probe>();
		probe->docking = GUI::dock::FILL;
		ui.add_child(probe);
		ui.process_ui(0.0f);

		ui.on_touch_event(make_touch(GUI::touch_event::DOWN, 3, 100.0f, 100.0f));
		ui.on_touch_event(make_touch(GUI::touch_event::MOVE, 3, 300.0f, 300.0f));
		ui.on_touch_event(make_touch(GUI::touch_event::UP, 3, 300.0f, 300.0f));

		ASSERT_TRUE(probe->actions == "DOWN CANCEL");
		ASSERT_TRUE(probe->moves == 1);
		ASSERT_TRUE(probe->mouse_calls == 0);
	}

	// A finger held for the long-press delay offers a long press once, from process_ui.
	TEST(Core.HAL, GUITouch_LongPress)
	{
		THREAD_SCOPE(GUI);
		auto ui_owner = std::make_shared<GUI::user_interface>();
		GUI::user_interface& ui = *ui_owner;
		ui.size = vec2(256.0f, 256.0f);

		auto probe = std::make_shared<touch_probe>();
		probe->docking = GUI::dock::FILL;
		ui.add_child(probe);
		ui.process_ui(0.0f);

		ui.on_touch_event(make_touch(GUI::touch_event::DOWN, 1, 50.0f, 50.0f));
		ui.process_ui(0.0f);
		ASSERT_TRUE(probe->long_presses == 0);   // not held long enough yet

		std::this_thread::sleep_for(std::chrono::milliseconds(600));
		ui.process_ui(0.0f);
		ui.process_ui(0.0f);
		ASSERT_TRUE(probe->long_presses == 1);   // once, not on every update

		ui.on_touch_event(make_touch(GUI::touch_event::UP, 1, 50.0f, 50.0f));
	}

	// Front-most element is offered the touch first; one that takes it ends the search, one that
	// declines passes it on to the element behind.
	TEST(Core.HAL, GUITouch_FrontMostFirst)
	{
		THREAD_SCOPE(GUI);
		auto ui_owner = std::make_shared<GUI::user_interface>();
		GUI::user_interface& ui = *ui_owner;
		ui.size = vec2(256.0f, 256.0f);

		auto back = std::make_shared<touch_probe>();
		auto front = std::make_shared<touch_probe>();
		back->docking = front->docking = GUI::dock::FILL;
		ui.add_child(back);
		ui.add_child(front);
		ui.process_ui(0.0f);

		ui.on_touch_event(make_touch(GUI::touch_event::DOWN, 1, 10.0f, 10.0f));
		ASSERT_TRUE(front->actions == "DOWN");
		ASSERT_TRUE(back->actions.empty());
		ui.on_touch_event(make_touch(GUI::touch_event::UP, 1, 10.0f, 10.0f));

		front->actions.clear();
		front->accept = false;   // declines: the element behind gets its chance
		ui.on_touch_event(make_touch(GUI::touch_event::DOWN, 2, 10.0f, 10.0f));
		ASSERT_TRUE(front->actions == "DOWN");
		ASSERT_TRUE(back->actions == "DOWN");
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
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 256, HEIGHT = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUIRenderer_DrawColor");

		HAL::Texture2DView view(tex, *list);
		set_ui_target(list, view, vec4(0.1f, 0.1f, 0.2f, 1.0f));

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
	// user_interface layout → colored_rect::draw → Renderer::draw_color → the NinePatch batch.
	TEST(Core.HAL, GUIElement_ColoredRect)
	{
			THREAD_SCOPE(GUI);
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 256, HEIGHT = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {WIDTH, HEIGHT}, 1, 1,
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
		set_ui_target(list, view, vec4(0.1f, 0.1f, 0.2f, 1.0f));

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
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 256, HEIGHT = 240;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {WIDTH, HEIGHT}, 1, 1,
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
		set_ui_target(list, view, vec4(0.05f, 0.05f, 0.1f, 1.0f));

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

	// Single label "Hello Label" docked FILL in a 256x64 render target.
	// Exercises the full label path: recalculate → pre_draw (glyph raster) →
	// draw via UIContext.draw_infos loop.
	TEST(Core.HAL, GUIElement_Label)
	{
		THREAD_SCOPE(GUI);

		constexpr uint W = 256, H = 64;
		auto& device = RenderSystem::get().device();

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {W, H}, 1, 1,
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

		// pre_draw (glyph raster) now happens in the UI_PreDraw FrameGraph pass's
		// render(), which this test never invokes — run it directly instead.
		auto& ui_ctx_predraw = graph.get_context<GUI::UIContext>();
		if (!ui_ctx_predraw.pre_draw_infos.empty())
		{
			auto& pre_draw_queue = device.get_queue(HAL::CommandListType::DIRECT);
			auto pre_draw_list = pre_draw_queue->get_free_list();
			pre_draw_list->begin(L"pre_draw");
			GUI::user_interface::run_pre_draw(ui_ctx_predraw.pre_draw_infos, pre_draw_list);
			pre_draw_list->end();
			pre_draw_list->execute_and_wait();
		}

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"GUIElement_Label");

		HAL::Texture2DView view(tex, *list);
		set_ui_target(list, view, vec4(0.05f, 0.05f, 0.1f, 1.0f));

		sizer_long full_vp{ 0, 0, (long)W, (long)H };
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
			c.layer         = e.layer;
			c.layer_slot    = e.layer_slot;
			c.layer_opacity = e.layer_opacity;

			GUI::draw_bounds_scope bounds_scope(e.elem, e.bounds);

			if (e.before)
				e.elem->draw_before(c);
			else
				e.elem->draw_end(c);
		}
		renderer.flush(c);

		list->execute_and_wait();

		ASSERT_TEXTURE_EX(tex.get(), "gui_element_label", .tolerance = 24);   // glyph anti-aliasing differs a little per GPU
	}

	// Renders a realistic multi-widget UI into a reference-size texture.
	// Exercises: colored_rect background, label, button row, check_box_text
	// panel, status_bar.  Uses create_graph to pre-render label glyphs via
	// the production path, then iterates UIContext.draw_infos exactly as
	// PassDefault<UI_Render>::render does.
	TEST(Core.HAL, GUIFullScreen)
	{
		THREAD_SCOPE(GUI);

		constexpr uint W = kReferenceWidth, H = kReferenceHeight;
		auto& device = RenderSystem::get().device();

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {W, H}, 1, 1,
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

		// Populate UIContext.draw_infos. Pre-draw (label glyph cache updates
		// etc.) now happens in the UI_PreDraw FrameGraph pass's render(), which
		// this test never invokes (it drives the GPU render manually instead of
		// via Graph::setup()/render()) — so run it directly here, matching what
		// PassDefault<Passes::UI::UI_PreDraw>::render() does.
		FrameGraph::Graph graph;
		ui.create_graph(graph);
		device.get_queue(HAL::CommandListType::DIRECT)->signal_and_wait();

		auto& ui_ctx_predraw = graph.get_context<GUI::UIContext>();
		if (!ui_ctx_predraw.pre_draw_infos.empty())
		{
			auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
			auto pre_draw_list = queue->get_free_list();
			pre_draw_list->begin(L"pre_draw");
			GUI::user_interface::run_pre_draw(ui_ctx_predraw.pre_draw_infos, pre_draw_list);
			pre_draw_list->end();
			pre_draw_list->execute_and_wait();
		}

		// GPU render pass
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUIFullScreen");

		HAL::Texture2DView view(tex, *list);
		set_ui_target(list, view, vec4(0.05f, 0.05f, 0.1f, 1.0f));

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
			c.layer         = e.layer;
			c.layer_slot    = e.layer_slot;
			c.layer_opacity = e.layer_opacity;

			GUI::draw_bounds_scope bounds_scope(e.elem, e.bounds);

			if (e.before)
				e.elem->draw_before(c);
			else
				e.elem->draw_end(c);
		}
		renderer.flush(c);

		list->execute_and_wait();
		ASSERT_TEXTURE_EX(tex.get(), "gui_full_screen", .tolerance = 24);   // glyph edges and 1px tab border rows differ a little per GPU
	}

	// Renders a 32x32 nine-patch source texture into a 256x128 target at three
	// different rect sizes to verify that corners stay fixed (8px) while the
	// edges and center stretch correctly.
	// Source color layout: RED 8x8 corners, GREEN top/bottom edges,
	// BLUE left/right edges, WHITE center (R8G8B8A8_UNORM).
	TEST(Core.HAL, GUINinePatch_Stretch)
	{
		THREAD_SCOPE(GUI);
		auto& device = RenderSystem::get().device();
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
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {RT_W, RT_H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUINinePatch_Stretch");

		HAL::Texture2DView src_view(src_tex, *list);
		HAL::Texture2DView rt_view(rt, *list);

		set_ui_target(list, rt_view, vec4(0.05f, 0.05f, 0.1f, 1.0f));

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

	// Each test clips the 160x80 patch at (48,24) from a different side,
	// landing inside the 8px corner region to exercise the corner clipping path.

	// Clip cuts 8px into the left corner (corner ends at x=56, clip starts at x=52).
	TEST(Core.HAL, GUINinePatch_ClipLeft)
	{
		THREAD_SCOPE(GUI);
		sizer_long clip{ 52, 0, 256, 128 };
		auto rt = run_clip_test(RenderSystem::get().device(), clip, L"GUINinePatch_ClipLeft"_cs);
		ASSERT_TEXTURE(rt.get(), "gui_nine_patch_clip_left");
	}

	// Clip cuts 8px into the right corner (corner starts at x=200, clip ends at x=204).
	TEST(Core.HAL, GUINinePatch_ClipRight)
	{
		THREAD_SCOPE(GUI);
		sizer_long clip{ 0, 0, 204, 128 };
		auto rt = run_clip_test(RenderSystem::get().device(), clip, L"GUINinePatch_ClipRight"_cs);
		ASSERT_TEXTURE(rt.get(), "gui_nine_patch_clip_right");
	}

	// Clip cuts 8px into the top corner (corner ends at y=32, clip starts at y=28).
	TEST(Core.HAL, GUINinePatch_ClipTop)
	{
		THREAD_SCOPE(GUI);
		sizer_long clip{ 0, 28, 256, 128 };
		auto rt = run_clip_test(RenderSystem::get().device(), clip, L"GUINinePatch_ClipTop"_cs);
		ASSERT_TEXTURE(rt.get(), "gui_nine_patch_clip_top");
	}

	// Clip cuts 8px into the bottom corner (corner starts at y=96, clip ends at y=100).
	TEST(Core.HAL, GUINinePatch_ClipBottom)
	{
		THREAD_SCOPE(GUI);
		sizer_long clip{ 0, 0, 256, 100 };
		auto rt = run_clip_test(RenderSystem::get().device(), clip, L"GUINinePatch_ClipBottom"_cs);
		ASSERT_TEXTURE(rt.get(), "gui_nine_patch_clip_bottom");
	}

	// All four sides clipped simultaneously — only the white center is visible.
	TEST(Core.HAL, GUINinePatch_ClipCenter)
	{
		THREAD_SCOPE(GUI);
		sizer_long clip{ 60, 36, 196, 92 };
		auto rt = run_clip_test(RenderSystem::get().device(), clip, L"GUINinePatch_ClipCenter"_cs);
		ASSERT_TEXTURE(rt.get(), "gui_nine_patch_clip_center");
	}

	// Text::Engine directly on a command list — no label element, no FrameGraph.
	// Exercises the text path in isolation: layout → glyph raster → atlas
	// upload → glyph quad draw, raw output into a plain UNORM target.
	TEST(Core.HAL, TextEngineDirect_Draw)
	{
		THREAD_SCOPE(GUI);

		constexpr uint W = 256, H = 64;
		auto& device = RenderSystem::get().device();

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto  list  = queue->get_free_list();
		list->begin(L"TextEngineDirect_Draw");

		Text::Layout text;
		Text::Engine::get().build("Hello Direct", { 20.0f, Text::Weight::Light }, text);
		Text::Engine::get().upload(list);

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;
		list->get_graphics().set_rtv(compiled,
			HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0,
			vec4(0.05f, 0.05f, 0.1f, 1.0f));

		Text::Engine::get().draw(list, text, vec2(8.0f, 10.0f), float4(1.0f, 1.0f, 1.0f, 1.0f),
			sizer{ 0, 0, float(W), float(H) }, vec2(float(W), float(H)), true);

		list->execute_and_wait();
		ASSERT_TEXTURE_EX(tex.get(), "text_engine_direct_draw", .tolerance = 24);
	}

	// Backdrop blur: a hard red|blue seam is drawn first, then a blurred panel
	// over part of it. Inside the panel the seam must be smeared (blue leaks
	// into the red side); outside it the seam must stay sharp. Checked on read-
	// back pixel values, not a golden: the point is that the snapshot, the blur
	// chain and the composite really ran in order on one command list.
	TEST(Core.HAL, GUIRenderer_BackdropBlur)
	{
		THREAD_SCOPE(GUI);
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 256, HEIGHT = 128;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUIRenderer_BackdropBlur");

		HAL::Texture2DView view(tex, *list);
		set_ui_target(list, view, vec4(0.0f, 0.0f, 0.0f, 1.0f));

		sizer_long full_vp;
		full_vp.left   = 0;
		full_vp.top    = 0;
		full_vp.right  = (long)WIDTH;
		full_vp.bottom = (long)HEIGHT;

		GUI::Renderer renderer;
		GUIInfo c;
		c.renderer     = &renderer;
		c.command_list = list;
		c.target       = view;
		c.ui_scale     = 1.0f;
		c.window_size  = vec2((float)WIDTH, (float)HEIGHT);
		c.offset       = vec2(0.0f, 0.0f);
		c.ui_clipping  = full_vp;
		c.scissors     = full_vp;
		c.scale        = 1.0f;
		c.delta_time   = 0.0f;

		renderer.draw_color(c, float4(1.0f, 0.0f, 0.0f, 1.0f), rect{ vec2(0.0f, 0.0f),   vec2(128.0f, 128.0f) });
		renderer.draw_color(c, float4(0.0f, 0.0f, 1.0f, 1.0f), rect{ vec2(128.0f, 0.0f), vec2(128.0f, 128.0f) });

		GUI::BackdropBlurParams params;
		params.radius = 16.0f;
		params.tint = float4(0.0f, 0.0f, 0.0f, 0.0f);
		params.saturation = 1.0f;
		renderer.draw_backdrop_blur(c, 0, rect{ vec2(64.0f, 32.0f), vec2(128.0f, 64.0f) }, params);
		renderer.flush(c);

		list->execute_and_wait();

		auto data = readback_texture(tex.get());
		ASSERT_TRUE(data && !data->array.empty() && !data->array[0]->mips.empty());
		const auto& mip = *data->array[0]->mips[0];

		auto half_to_float = [](uint16_t h)
		{
			const uint32_t e = (h >> 10) & 0x1f, m = h & 0x3ff;
			float v = e == 0 ? std::ldexp(float(m), -24)
			                 : std::ldexp(float(m | 0x400), int(e) - 25);
			return (h & 0x8000) ? -v : v;
		};

		auto channel = [&](uint x, uint y, uint ch)
		{
			const uint16_t* row = reinterpret_cast<const uint16_t*>(mip.data.data() + size_t(y) * mip.width_stride);
			return half_to_float(row[x * 4 + ch]);
		};

		const uint y = 64;

		// Outside the panel: the seam is untouched.
		ASSERT_TRUE(channel(40, y, 0) > 0.9f);
		ASSERT_TRUE(channel(40, y, 2) < 0.05f);
		ASSERT_TRUE(channel(220, y, 2) > 0.9f);
		ASSERT_TRUE(channel(220, y, 0) < 0.05f);

		// Inside the panel, 8 px left of the seam: blue has leaked across, red has dimmed.
		ASSERT_TRUE(channel(120, y, 2) > 0.08f);
		ASSERT_TRUE(channel(120, y, 0) < 0.92f);

		// Same distance on the other side, mirrored.
		ASSERT_TRUE(channel(136, y, 0) > 0.08f);
		ASSERT_TRUE(channel(136, y, 2) < 0.92f);

		// Far from the seam inside the panel the blur must not wash the colours out.
		ASSERT_TRUE(channel(70, y, 0) > 0.8f);
		ASSERT_TRUE(channel(186, y, 2) > 0.8f);
	}

	// Small radii take the direct path (the composite filters the snapshot itself,
	// no blur chain): at the default 3-4 px the seam is smeared over a few pixels
	// only, and nothing further away is touched.
	TEST(Core.HAL, GUIRenderer_BackdropBlurDirect)
	{
		THREAD_SCOPE(GUI);
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH = 256, HEIGHT = 128;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"GUIRenderer_BackdropBlurDirect");

		HAL::Texture2DView view(tex, *list);
		set_ui_target(list, view, vec4(0.0f, 0.0f, 0.0f, 1.0f));

		sizer_long full_vp;
		full_vp.left   = 0;
		full_vp.top    = 0;
		full_vp.right  = (long)WIDTH;
		full_vp.bottom = (long)HEIGHT;

		GUI::Renderer renderer;
		GUIInfo c;
		c.renderer     = &renderer;
		c.command_list = list;
		c.target       = view;
		c.ui_scale     = 1.0f;
		c.window_size  = vec2((float)WIDTH, (float)HEIGHT);
		c.offset       = vec2(0.0f, 0.0f);
		c.ui_clipping  = full_vp;
		c.scissors     = full_vp;
		c.scale        = 1.0f;
		c.delta_time   = 0.0f;

		renderer.draw_color(c, float4(1.0f, 0.0f, 0.0f, 1.0f), rect{ vec2(0.0f, 0.0f),   vec2(128.0f, 128.0f) });
		renderer.draw_color(c, float4(0.0f, 0.0f, 1.0f, 1.0f), rect{ vec2(128.0f, 0.0f), vec2(128.0f, 128.0f) });

		GUI::BackdropBlurParams params;
		params.radius = 4.0f;
		params.tint = float4(0.0f, 0.0f, 0.0f, 0.0f);
		params.saturation = 1.0f;
		renderer.draw_backdrop_blur(c, 0, rect{ vec2(64.0f, 32.0f), vec2(128.0f, 64.0f) }, params);
		renderer.flush(c);

		list->execute_and_wait();

		const auto img = fx_image{ readback_texture(tex.get()) };
		const uint y = 64;

		ASSERT_TRUE(img.at(127, y, 2) > 0.05f);     // blue has leaked 1 px across the seam
		ASSERT_TRUE(img.at(127, y, 2) < 0.5f);      // but it is a smear, not a swap
		ASSERT_TRUE(img.at(128, y, 0) > 0.05f);     // and the same the other way
		ASSERT_TRUE(img.at(112, y, 2) < 0.02f);     // 16 px from the seam: untouched
		ASSERT_TRUE(img.at(112, y, 0) > 0.95f);
		ASSERT_TRUE(img.at(144, y, 2) > 0.95f);
		ASSERT_TRUE(img.at(40, y, 0) > 0.95f);      // outside the panel: untouched
	}

	// The real menu element through the production draw path (create_graph →
	// draw_infos → draw, the loop UI_Render runs): a popup menu straddling a
	// hard red|blue seam must show it blurred. Its item rects are translucent
	// dark plates, so without the backdrop pass the blue channel just left of
	// the seam is exactly 0.
	TEST(Core.HAL, GUIMenuList_BackdropBlur)
	{
		THREAD_SCOPE(GUI);
		auto& device = RenderSystem::get().device();
		constexpr uint W = 512, H = 256;
		constexpr float seam = 256.0f;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		GUI::user_interface ui;
		ui.size = vec2((float)W, (float)H);
		ui.backdrop_radius = 18.0f;   // the assertions below look 8 px either side of the seam
		ui.shadows_enabled = false;   // the popup's shadow would darken the "outside stays hard" pixel

		{
			auto left = std::make_shared<GUI::Elements::colored_rect>();
			left->color       = float4(1.0f, 0.0f, 0.0f, 1.0f);
			left->docking     = GUI::dock::LEFT;
			left->width_size  = GUI::size_type::FIXED;
			left->size        = vec2(seam, 0.0f);
			ui.add_child(left);

			auto right = std::make_shared<GUI::Elements::colored_rect>();
			right->color   = float4(0.0f, 0.0f, 1.0f, 1.0f);
			right->docking = GUI::dock::FILL;
			ui.add_child(right);
		}

		auto menu = std::make_shared<GUI::Elements::menu_list>();
		menu->add_item("Open");
		menu->add_item("Save");
		menu->add_item("Close");
		ui.add_child(menu);
		ui.process_ui(0.0f);

		// Centre the popup on the seam; its size is only known after a layout pass.
		const rect first = menu->get_render_bounds();
		menu->pos = vec2(seam - first.w / 2.0f, 60.0f);
		ui.process_ui(0.0f);
		const rect mb = menu->get_render_bounds();
		ASSERT_TRUE(mb.x < seam - 16.0f && mb.x + mb.w > seam + 16.0f);

		FrameGraph::Graph graph;
		ui.create_graph(graph);
		device.get_queue(HAL::CommandListType::DIRECT)->signal_and_wait();

		auto& predraw = graph.get_context<GUI::UIContext>();
		if (!predraw.pre_draw_infos.empty())
		{
			auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
			auto pre_draw_list = queue->get_free_list();
			pre_draw_list->begin(L"pre_draw");
			GUI::user_interface::run_pre_draw(predraw.pre_draw_infos, pre_draw_list);
			pre_draw_list->end();
			pre_draw_list->execute_and_wait();
		}

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUIMenuList_BackdropBlur");

		HAL::Texture2DView view(tex, *list);
		set_ui_target(list, view, vec4(0.0f, 0.0f, 0.0f, 1.0f));

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
		c.target       = view;
		c.ui_scale     = 1.0f;
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
			c.layer         = e.layer;
			c.layer_slot    = e.layer_slot;
			c.layer_opacity = e.layer_opacity;

			GUI::draw_bounds_scope bounds_scope(e.elem, e.bounds);

			if (e.before)
				e.elem->draw_before(c);
			else
				e.elem->draw_end(c);
		}
		renderer.flush(c);

		list->execute_and_wait();

		auto data = readback_texture(tex.get());
		ASSERT_TRUE(data && !data->array.empty() && !data->array[0]->mips.empty());
		const auto& mip = *data->array[0]->mips[0];

		auto half_to_float = [](uint16_t h)
		{
			const uint32_t e = (h >> 10) & 0x1f, m = h & 0x3ff;
			float v = e == 0 ? std::ldexp(float(m), -24)
			                 : std::ldexp(float(m | 0x400), int(e) - 25);
			return (h & 0x8000) ? -v : v;
		};

		auto channel = [&](uint x, uint y, uint ch)
		{
			const uint16_t* row = reinterpret_cast<const uint16_t*>(mip.data.data() + size_t(y) * mip.width_stride);
			return half_to_float(row[x * 4 + ch]);
		};

		// A row inside the first item's top padding: dark plate over the backdrop, no glyphs.
		const uint y = (uint)(mb.y + 2.0f);

		// Outside the menu the seam is hard.
		const uint outside = (uint)(mb.x - 6.0f);
		ASSERT_TRUE(channel(outside, y, 0) > 0.9f);
		ASSERT_TRUE(channel(outside, y, 2) < 0.02f);

		// Inside, either side of the seam: the other colour has leaked across the
		// 0.41-transmittance plate (the plate is alpha 150/255 dark over the blur).
		ASSERT_TRUE(channel((uint)seam - 8, y, 2) > 0.03f);
		ASSERT_TRUE(channel((uint)seam + 8, y, 0) > 0.03f);

		// And it is a smooth transition, not a step: monotonic red falloff across the seam.
		float prev = channel((uint)seam - 16, y, 0);
		for (uint x = (uint)seam - 15; x <= (uint)seam + 16; x += 4)
		{
			const float cur = channel(x, y, 0);
			ASSERT_TRUE(cur <= prev + 0.01f);
			prev = cur;
		}
	}

	// A floating window goes through the same generic path (a BackdropBlur effect
	// + draw_before). Its skin body is 80% black, so only ~20% of the blurred
	// backdrop shows through: without the blur the blue channel just left of the
	// seam is exactly 0.
	TEST(Core.HAL, GUIWindow_BackdropBlur)
	{
		THREAD_SCOPE(GUI);
		auto& device = RenderSystem::get().device();
		constexpr uint W = 512, H = 256;
		constexpr float seam = 256.0f;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		GUI::user_interface ui;
		ui.size = vec2((float)W, (float)H);
		ui.backdrop_radius = 12.0f;   // wide enough to show well above the skin's 20% transmittance
		ui.window_animations = false; // the window is measured fully opaque, not mid-fade

		{
			auto left = std::make_shared<GUI::Elements::colored_rect>();
			left->color       = float4(1.0f, 0.0f, 0.0f, 1.0f);
			left->docking     = GUI::dock::LEFT;
			left->width_size  = GUI::size_type::FIXED;
			left->size        = vec2(seam, 0.0f);
			ui.add_child(left);

			auto right = std::make_shared<GUI::Elements::colored_rect>();
			right->color   = float4(0.0f, 0.0f, 1.0f, 1.0f);
			right->docking = GUI::dock::FILL;
			ui.add_child(right);
		}

		auto win = std::make_shared<GUI::Elements::window>();
		win->width_size  = GUI::size_type::FIXED;
		win->height_size = GUI::size_type::FIXED;
		win->size        = vec2(240.0f, 140.0f);
		win->pos         = vec2(seam - 120.0f, 50.0f);
		ui.add_child(win);
		ui.process_ui(0.0f);

		const rect wb = win->get_render_bounds();
		ASSERT_TRUE(wb.x < seam - 40.0f && wb.x + wb.w > seam + 40.0f);

		FrameGraph::Graph graph;
		ui.create_graph(graph);
		device.get_queue(HAL::CommandListType::DIRECT)->signal_and_wait();

		auto& predraw = graph.get_context<GUI::UIContext>();
		if (!predraw.pre_draw_infos.empty())
		{
			auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
			auto pre_draw_list = queue->get_free_list();
			pre_draw_list->begin(L"pre_draw");
			GUI::user_interface::run_pre_draw(predraw.pre_draw_infos, pre_draw_list);
			pre_draw_list->end();
			pre_draw_list->execute_and_wait();
		}

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"GUIWindow_BackdropBlur");

		HAL::Texture2DView view(tex, *list);
		set_ui_target(list, view, vec4(0.0f, 0.0f, 0.0f, 1.0f));

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
		c.target       = view;
		c.ui_scale     = 1.0f;
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
			c.layer         = e.layer;
			c.layer_slot    = e.layer_slot;
			c.layer_opacity = e.layer_opacity;

			GUI::draw_bounds_scope bounds_scope(e.elem, e.bounds);

			if (e.before)
				e.elem->draw_before(c);
			else
				e.elem->draw_end(c);
		}
		renderer.flush(c);

		list->execute_and_wait();

		auto data = readback_texture(tex.get());
		ASSERT_TRUE(data && !data->array.empty() && !data->array[0]->mips.empty());
		const auto& mip = *data->array[0]->mips[0];

		auto half_to_float = [](uint16_t h)
		{
			const uint32_t e = (h >> 10) & 0x1f, m = h & 0x3ff;
			float v = e == 0 ? std::ldexp(float(m), -24)
			                 : std::ldexp(float(m | 0x400), int(e) - 25);
			return (h & 0x8000) ? -v : v;
		};

		auto channel = [&](uint x, uint y, uint ch)
		{
			const uint16_t* row = reinterpret_cast<const uint16_t*>(mip.data.data() + size_t(y) * mip.width_stride);
			return half_to_float(row[x * 4 + ch]);
		};

		// A row in the window body, below the title bar.
		const uint y = (uint)(wb.y + 90.0f);

		// Far outside the window the seam is untouched.
		ASSERT_TRUE(channel(20, y, 0) > 0.9f);
		ASSERT_TRUE(channel(20, y, 2) < 0.02f);

		// Inside the body the other colour has leaked across (through the skin's ~20% transmittance).
		ASSERT_TRUE(channel((uint)seam - 6, y, 2) > 0.015f);
		ASSERT_TRUE(channel((uint)seam + 6, y, 0) > 0.015f);

		// A step would be exactly 0 either side of it; blur makes the transition monotonic.
		float prev = channel((uint)seam - 24, y, 0);
		for (uint x = (uint)seam - 20; x <= (uint)seam + 24; x += 4)
		{
			const float cur = channel(x, y, 0);
			ASSERT_TRUE(cur <= prev + 0.005f);
			prev = cur;
		}
	}

	// Reveal highlight: a soft spot at the cursor, nothing far from it or
	// outside the shape. (Base grey is ~0.033 linear.)
	TEST(Core.HAL, GUIFx_Reveal)
	{
		THREAD_SCOPE(GUI);

		GUI::FxParams p;
		p.cursor = float2(96.0f, 64.0f);
		p.reveal_strength = 0.6f;
		p.reveal_size = 14.0f;
		const auto img = render_fx(p);

		ASSERT_TRUE(img.at(96, 64) > 0.4f);
		ASSERT_TRUE(img.at(96, 64) > img.at(112, 64));
		ASSERT_TRUE(img.at(112, 64) > img.at(130, 64));
		ASSERT_TRUE(img.at(170, 64) < 0.06f);    // far from the cursor: just the base grey
		ASSERT_TRUE(img.at(20, 64) < 0.01f);     // outside the shape: untouched black
		ASSERT_TRUE(img.at(96, 20) < 0.01f);
	}

	// Edge light: the rim lights up near the cursor and stays dark far from it;
	// the inside of the shape is not lit.
	TEST(Core.HAL, GUIFx_EdgeLight)
	{
		THREAD_SCOPE(GUI);

		GUI::FxParams p;
		p.cursor = float2(64.0f, 64.0f);   // on the left edge
		p.border_strength = 1.0f;
		p.border_size = 40.0f;
		p.border_width = 2.0f;
		const auto img = render_fx(p);

		ASSERT_TRUE(img.at(64, 64) > 0.5f);      // rim at the cursor
		ASSERT_TRUE(img.at(128, 64) < 0.06f);    // centre: not lit
		ASSERT_TRUE(img.at(191, 64) < 0.08f);    // rim on the far side: out of the cursor's reach
		ASSERT_TRUE(img.at(64, 64) > img.at(64, 90));   // falls off along the rim
	}

	// A floating window's procedural shadow: it falls outside the window (a soft
	// darkening that fades with distance) and leaves the far background alone.
	TEST(Core.HAL, GUIWindow_Shadow)
	{
		THREAD_SCOPE(GUI);
		auto& device = RenderSystem::get().device();
		constexpr uint W = 512, H = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(UI_RT_FORMAT, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		GUI::user_interface ui;
		ui.size = vec2((float)W, (float)H);
		ui.window_animations = false;

		auto background = std::make_shared<GUI::Elements::colored_rect>();
		background->color   = float4(0.5f, 0.5f, 0.5f, 1.0f);
		background->docking = GUI::dock::FILL;
		ui.add_child(background);

		auto win = std::make_shared<GUI::Elements::window>();
		win->width_size  = GUI::size_type::FIXED;
		win->height_size = GUI::size_type::FIXED;
		win->size        = vec2(200.0f, 110.0f);
		win->pos         = vec2(160.0f, 70.0f);
		ui.add_child(win);
		ui.process_ui(0.0f);

		const rect wb = win->get_render_bounds();

		FrameGraph::Graph graph;
		ui.create_graph(graph);
		device.get_queue(HAL::CommandListType::DIRECT)->signal_and_wait();

		auto& predraw = graph.get_context<GUI::UIContext>();
		if (!predraw.pre_draw_infos.empty())
		{
			auto pre_draw_list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
			pre_draw_list->begin(L"pre_draw");
			GUI::user_interface::run_pre_draw(predraw.pre_draw_infos, pre_draw_list);
			pre_draw_list->end();
			pre_draw_list->execute_and_wait();
		}

		auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"GUIWindow_Shadow");

		HAL::Texture2DView view(tex, *list);
		set_ui_target(list, view, vec4(0.0f, 0.0f, 0.0f, 1.0f));

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
		c.target       = view;
		c.ui_scale     = 1.0f;
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
			c.layer         = e.layer;
			c.layer_slot    = e.layer_slot;
			c.layer_opacity = e.layer_opacity;

			GUI::draw_bounds_scope bounds_scope(e.elem, e.bounds);

			if (e.before)
				e.elem->draw_before(c);
			else
				e.elem->draw_end(c);
		}
		renderer.flush(c);

		list->execute_and_wait();

		const auto img = fx_image{ readback_texture(tex.get()) };
		const uint mid_y = (uint)(wb.y + wb.h * 0.5f);

		const float far_value = img.at(20, 20);
		ASSERT_TRUE(far_value > 0.15f);                                    // grey background, ~0.21 linear

		const float near_value = img.at((uint)(wb.x - 8.0f), mid_y);      // 8 px outside the left edge
		ASSERT_TRUE(near_value < far_value * 0.96f);                       // shadowed
		ASSERT_TRUE(near_value > far_value * 0.6f);                        // but soft, not black

		const float nearer = img.at((uint)(wb.x - 2.0f), mid_y);
		ASSERT_TRUE(nearer < near_value);                                  // darker closer to the window

		const float below = img.at((uint)(wb.x + wb.w * 0.5f), (uint)(wb.y + wb.h + 6.0f));
		ASSERT_TRUE(below < far_value * 0.96f);                            // and below it (offset pushes it down)
	}

	// The hover state behind the effects: with the cursor over an element (or its
	// child) fx_hover eases to 1 within a few frames and fx_shine starts; moving
	// away eases it back to 0. No GPU needed.
	TEST(Core.HAL, GUIFx_HoverState)
	{
		THREAD_SCOPE(GUI);

		// Shared-owned: hit testing calls get_ptr() on the root, which a stack
		// user_interface (as the other tests use) cannot answer.
		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(400.0f, 200.0f);

		auto card = std::make_shared<GUI::Elements::colored_rect>();
		card->docking     = GUI::dock::NONE;
		card->width_size  = GUI::size_type::FIXED;
		card->height_size = GUI::size_type::FIXED;
		card->size        = vec2(200.0f, 100.0f);
		card->pos         = vec2(20.0f, 20.0f);
		auto& light = card->add_effect<GUI::LightOverlay>();
		light.reveal = true;
		light.border = true;
		light.shine  = true;

		auto child = std::make_shared<GUI::base>();   // what the cursor really hits
		child->docking = GUI::dock::FILL;
		card->add_child(child);
		ui.add_child(card);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};

		frame(0.03f);
		ASSERT_TRUE(light.hover == 0.0f);
		ASSERT_TRUE(light.shine_t < 0.0f);

		const rect b = card->get_render_bounds();
		ui.mouse_move_event_internal(vec2(b.x + b.w * 0.5f, b.y + b.h * 0.5f));

		frame(0.03f);
		ASSERT_TRUE(light.hover > 0.2f);      // started easing in
		ASSERT_TRUE(light.shine_t >= 0.0f);     // the sweep began

		for (int i = 0; i < 12; i++)
			frame(0.03f);
		ASSERT_TRUE(light.hover > 0.95f);     // settled

		ui.mouse_move_event_internal(vec2(390.0f, 190.0f));   // off the card
		for (int i = 0; i < 14; i++)
			frame(0.03f);
		ASSERT_TRUE(light.hover < 0.05f);
	}

	// Shine: a band at the sweep phase, moving across the shape; off at phase < 0.
	TEST(Core.HAL, GUIFx_Shine)
	{
		THREAD_SCOPE(GUI);

		GUI::FxParams p;
		p.shine_strength = 0.8f;
		p.shine_width = 0.18f;

		p.shine_phase = 0.5f;   // band centred
		const auto mid = render_fx(p);
		ASSERT_TRUE(mid.at(128, 64) > 0.5f);
		ASSERT_TRUE(mid.at(70, 64) < 0.06f);
		ASSERT_TRUE(mid.at(186, 64) < 0.06f);

		p.shine_phase = 0.1f;   // band near the start
		const auto early = render_fx(p);
		ASSERT_TRUE(early.at(70, 64) > 0.2f);
		ASSERT_TRUE(early.at(128, 64) < 0.06f);

		p.shine_phase = -1.0f;  // idle
		const auto off = render_fx(p);
		ASSERT_TRUE(off.at(128, 64) < 0.06f);
		ASSERT_TRUE(off.at(70, 64) < 0.06f);
	}

	// Press ripple: a ring expanding from the press point; the centre is only
	// lightly filled and the far side has not been reached yet.
	TEST(Core.HAL, GUIFx_Ripple)
	{
		THREAD_SCOPE(GUI);

		GUI::FxParams p;
		p.ripple_origin = float2(128.0f, 64.0f);
		p.ripple_strength = 0.8f;

		p.ripple_phase = 0.4f;   // front about 29 px out
		const auto img = render_fx(p);
		ASSERT_TRUE(img.at(156, 64) > 0.35f);               // on the front
		ASSERT_TRUE(img.at(156, 64) > img.at(128, 64));     // brighter than the centre
		ASSERT_TRUE(img.at(185, 64) < 0.12f);               // beyond it

		p.ripple_phase = -1.0f;
		const auto off = render_fx(p);
		ASSERT_TRUE(off.at(156, 64) < 0.06f);
	}

	// A tinted light (for light surfaces): the reveal spot takes the light's colour.
	TEST(Core.HAL, GUIFx_LightColor)
	{
		THREAD_SCOPE(GUI);

		GUI::FxParams p;
		p.cursor = float2(128.0f, 64.0f);
		p.reveal_strength = 0.6f;
		p.reveal_size = 20.0f;
		p.light_color = float3(0.0f, 0.0f, 1.0f);
		const auto img = render_fx(p);

		ASSERT_TRUE(img.at(128, 64, 2) > 0.35f);     // blue at the cursor
		ASSERT_TRUE(img.at(128, 64, 0) < 0.1f);      // and only blue: no white in it
		ASSERT_TRUE(img.at(128, 64, 1) < 0.1f);
	}

	// Focus ring: accent blue just inside the edge, nothing in the middle.
	TEST(Core.HAL, GUIFx_FocusRing)
	{
		THREAD_SCOPE(GUI);

		GUI::FxParams p;
		p.focus_strength = 1.0f;
		p.focus_width = 2.0f;
		const auto img = render_fx(p);

		ASSERT_TRUE(img.at(64, 64, 2) > 0.3f);                         // blue at the edge
		ASSERT_TRUE(img.at(64, 64, 2) > 3.0f * img.at(64, 64, 0));     // and it is blue, not white
		ASSERT_TRUE(img.at(128, 64, 2) < 0.06f);                       // centre untouched
	}

	// Inner shadow: darker at the edges than in the middle, and deeper along the top.
	TEST(Core.HAL, GUIFx_InnerShadow)
	{
		THREAD_SCOPE(GUI);

		GUI::FxParams p;
		p.inner_strength = 0.6f;
		p.inner_size = 10.0f;
		p.inner_offset = 2.0f;
		const auto img = render_fx(p);

		ASSERT_TRUE(img.at(128, 64) > 0.028f);                  // middle keeps the base grey (~0.033)
		ASSERT_TRUE(img.at(64, 64) < 0.02f);                    // left edge darkened
		ASSERT_TRUE(img.at(128, 34) < img.at(128, 93));         // top darker than bottom
	}

	// The state behind the press ripple and the focus ring: a press starts the
	// ripple at the cursor and it runs out on its own; focus eases the ring in
	// and out. No GPU needed.
	TEST(Core.HAL, GUIFx_PressAndFocusState)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(400.0f, 200.0f);

		auto card = std::make_shared<GUI::Elements::colored_rect>();
		card->docking     = GUI::dock::NONE;
		card->width_size  = GUI::size_type::FIXED;
		card->height_size = GUI::size_type::FIXED;
		card->size        = vec2(200.0f, 100.0f);
		card->pos         = vec2(20.0f, 20.0f);
		auto& light = card->add_effect<GUI::LightOverlay>();
		light.ripple = true;
		light.focus  = true;
		ui.add_child(card);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};

		frame(0.03f);
		ASSERT_TRUE(light.ripple_t < 0.0f);
		ASSERT_TRUE(light.focus_t == 0.0f);

		const rect b = card->get_render_bounds();
		const vec2 press_at(b.x + b.w * 0.25f, b.y + b.h * 0.5f);
		ui.mouse_move_event_internal(press_at);
		ui.mouse_action_event_internal(mouse_action::DOWN, mouse_button::LEFT, press_at);

		frame(0.03f);
		ASSERT_TRUE(light.ripple_t >= 0.0f);                                   // started
		ASSERT_TRUE(std::abs(light.ripple_origin.x - press_at.x) < 1.0f);    // at the cursor
		ASSERT_TRUE(std::abs(light.ripple_origin.y - press_at.y) < 1.0f);

		for (int i = 0; i < 25; i++)
			frame(0.03f);
		ASSERT_TRUE(light.ripple_t < 0.0f);                                    // ran out (0.55 s)

		card->focus();
		for (int i = 0; i < 14; i++)
			frame(0.03f);
		ASSERT_TRUE(light.focus_t > 0.95f);

		card->unfocus();
		for (int i = 0; i < 14; i++)
			frame(0.03f);
		ASSERT_TRUE(light.focus_t < 0.05f);
	}

	// Group opacity: a group at 0.5 over a red background is exactly
	// 0.5 * group + 0.5 * background, and where its children overlap only the
	// top child counts. Per-element alpha would let the lower child (green)
	// show through the upper one (blue) there.
	TEST(Core.HAL, GUILayer_GroupFade)
	{
		THREAD_SCOPE(GUI);
		constexpr uint W = 256, H = 128;

		GUI::user_interface ui;
		ui.size = vec2((float)W, (float)H);

		auto background = std::make_shared<GUI::Elements::colored_rect>();
		background->color   = float4(1.0f, 0.0f, 0.0f, 1.0f);
		background->docking = GUI::dock::FILL;
		ui.add_child(background);

		auto group = std::make_shared<GUI::base>();   // draws nothing itself
		group->docking     = GUI::dock::NONE;
		group->width_size  = GUI::size_type::FIXED;
		group->height_size = GUI::size_type::FIXED;
		group->size        = vec2(160.0f, 80.0f);
		group->pos         = vec2(48.0f, 24.0f);
		group->layer_opacity = 0.5f;
		ui.add_child(group);

		auto add_blob = [&](float4 color, vec2 pos)
		{
			auto blob = std::make_shared<GUI::Elements::colored_rect>();
			blob->color       = color;
			blob->docking     = GUI::dock::NONE;
			blob->width_size  = GUI::size_type::FIXED;
			blob->height_size = GUI::size_type::FIXED;
			blob->size        = vec2(80.0f, 50.0f);
			blob->pos         = pos;
			group->add_child(blob);
		};
		add_blob(float4(0.0f, 1.0f, 0.0f, 1.0f), vec2(10.0f, 10.0f));    // green, lower
		add_blob(float4(0.0f, 0.0f, 1.0f, 1.0f), vec2(50.0f, 20.0f));    // blue, on top of it

		const auto img = render_ui(ui, W, H);

		const rect gb = group->get_render_bounds();
		auto px = [&](float x, float y, uint ch) { return img.at((uint)(gb.x + x), (uint)(gb.y + y), ch); };

		// Outside the group: untouched red.
		ASSERT_TRUE(img.at(10, 10, 0) > 0.98f);
		ASSERT_TRUE(img.at(10, 10, 1) < 0.02f);

		// Green only (x 10..50 is clear of the blue one): half green, half the red behind it.
		ASSERT_TRUE(std::abs(px(20, 20, 1) - 0.5f) < 0.03f);
		ASSERT_TRUE(std::abs(px(20, 20, 0) - 0.5f) < 0.03f);

		// Overlap (blue over green): half blue, half red -- and no green at all.
		ASSERT_TRUE(std::abs(px(70, 40, 2) - 0.5f) < 0.03f);
		ASSERT_TRUE(std::abs(px(70, 40, 0) - 0.5f) < 0.03f);
		ASSERT_TRUE(px(70, 40, 1) < 0.03f);

		// Fully opaque or fully hidden are the two ends of the same blend.
		group->layer_opacity = 1.0f;
		const auto opaque = render_ui(ui, W, H);
		ASSERT_TRUE(opaque.at((uint)(gb.x + 20), (uint)(gb.y + 20), 1) > 0.98f);

		group->layer_opacity = 0.0f;
		const auto hidden = render_ui(ui, W, H);
		ASSERT_TRUE(hidden.at((uint)(gb.x + 20), (uint)(gb.y + 20), 0) > 0.98f);
		ASSERT_TRUE(hidden.at((uint)(gb.x + 20), (uint)(gb.y + 20), 1) < 0.02f);
	}

	// A layer copies the target when it opens and draws it back when it closes, so
	// both must land in one command list: the chunk ranges handed to the UI_Render
	// instances never start or end inside a layer.
	TEST(Core.HAL, GUILayer_ChunksKeepLayersWhole)
	{
		THREAD_SCOPE(GUI);

		GUI::user_interface ui;
		ui.size = vec2(400.0f, 400.0f);

		auto add_rects = [&](GUI::base::ptr parent, int count)
		{
			for (int i = 0; i < count; i++)
			{
				auto r = std::make_shared<GUI::Elements::colored_rect>();
				r->docking = GUI::dock::TOP;
				r->height_size = GUI::size_type::FIXED;
				r->size = vec2(0.0f, 1.0f);
				parent->add_child(r);
			}
		};

		auto before = std::make_shared<GUI::base>();
		before->docking = GUI::dock::TOP;
		before->height_size = GUI::size_type::FIXED;
		before->size = vec2(0.0f, 100.0f);
		add_rects(before, 40);
		ui.add_child(before);

		// ~300 draw entries inside one layer, far past the 64-per-instance minimum.
		auto group = std::make_shared<GUI::base>();
		group->docking = GUI::dock::TOP;
		group->height_size = GUI::size_type::FIXED;
		group->size = vec2(0.0f, 100.0f);
		group->layer_opacity = 0.5f;
		add_rects(group, 150);
		ui.add_child(group);

		auto after = std::make_shared<GUI::base>();
		after->docking = GUI::dock::TOP;
		after->height_size = GUI::size_type::FIXED;
		after->size = vec2(0.0f, 100.0f);
		add_rects(after, 150);
		ui.add_child(after);

		ui.process_ui(0.0f);
		FrameGraph::Graph graph;
		ui.create_graph(graph);

		const auto& ctx = graph.get_context<GUI::UIContext>();
		ASSERT_TRUE(ctx.draw_infos.size() > 400);
		ASSERT_TRUE(ctx.chunks.size() >= 2);   // it does get split somewhere

		// The ranges tile the list, and at every boundary no layer is open.
		uint32_t expected_start = 0;
		bool layer_seen = false;
		for (const auto& [start, end] : ctx.chunks)
		{
			ASSERT_TRUE(start == expected_start);
			ASSERT_TRUE(end > start);

			int depth = 0;
			for (uint32_t i = start; i < end; i++)
				if (ctx.draw_infos[i].layer)
				{
					layer_seen = true;
					depth += ctx.draw_infos[i].before ? 1 : -1;
					ASSERT_TRUE(depth >= 0);
				}

			ASSERT_TRUE(depth == 0);   // opened and closed inside this chunk
			expected_start = end;
		}

		ASSERT_TRUE(expected_start == ctx.draw_infos.size());
		ASSERT_TRUE(layer_seen);
	}

	// The fade state behind group opacity: fade_to eases layer_opacity to its
	// target, the element is a layer only while it is below 1, and it stops
	// being one when the fade ends. No GPU needed.
	TEST(Core.HAL, GUILayer_FadeState)
	{
		THREAD_SCOPE(GUI);

		GUI::user_interface ui;
		ui.size = vec2(200.0f, 100.0f);

		auto card = std::make_shared<GUI::Elements::colored_rect>();
		card->docking = GUI::dock::FILL;
		ui.add_child(card);

		auto layered = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);

			for (const auto& e : graph.get_context<GUI::UIContext>().draw_infos)
				if (e.elem == card.get() && e.before)
					return e.layer;
			return false;
		};

		ASSERT_TRUE(!layered(0.1f));                 // opacity 1: no layer

		card->layer_opacity = 0.0f;
		card->fade_to(1.0f, 0.3f);
		ASSERT_TRUE(layered(0.1f));                  // fading: a layer
		ASSERT_TRUE(card->layer_opacity > 0.0f && card->layer_opacity < 1.0f);

		float last = card->layer_opacity;
		layered(0.1f);
		ASSERT_TRUE(card->layer_opacity > last);     // still rising
		for (int i = 0; i < 4; i++)
			layered(0.1f);
		ASSERT_TRUE(card->layer_opacity == 1.0f);    // arrived
		ASSERT_TRUE(!layered(0.1f));                 // and no longer a layer

		ui.fade_enabled = false;
		card->layer_opacity = 0.3f;
		ASSERT_TRUE(!layered(0.1f));                 // the setting turns layers off
	}

	// The effect stack: add_effect appends once and returns the existing one the second
	// time, find_effect finds by type, and an element without effects has none.
	TEST(Core.HAL, GUIEffects_StackAddFind)
	{
		THREAD_SCOPE(GUI);

		auto plain = std::make_shared<GUI::Elements::colored_rect>();
		ASSERT_TRUE(plain->effects.empty());
		ASSERT_TRUE(plain->find_effect<GUI::Shadow>() == nullptr);

		auto& blur = plain->add_effect<GUI::BackdropBlur>();
		auto& shadow = plain->add_effect<GUI::Shadow>();
		ASSERT_TRUE(&plain->add_effect<GUI::BackdropBlur>() == &blur);   // not added twice
		ASSERT_TRUE(plain->effects.size() == 2);
		ASSERT_TRUE(plain->effects[0].get() == &blur);                   // stack order is add order
		ASSERT_TRUE(plain->effects[1].get() == &shadow);
		ASSERT_TRUE(plain->find_effect<GUI::Shadow>() == &shadow);
		ASSERT_TRUE(plain->find_effect<GUI::Glow>() == nullptr);

		// The presets: a window is glass with a shadow and an inner shadow; a button has light only.
		GUI::Elements::window win;
		ASSERT_TRUE(win.find_effect<GUI::BackdropBlur>() && win.find_effect<GUI::Shadow>() && win.find_effect<GUI::LightOverlay>());
		auto button = std::make_shared<GUI::Elements::button>();
		ASSERT_TRUE(button->find_effect<GUI::LightOverlay>() != nullptr);
		ASSERT_TRUE(button->find_effect<GUI::BackdropBlur>() == nullptr);
	}

	// Single selection in a list: one bar slides to the newly selected row (eased, so
	// it is in between mid-way) and ends exactly on it; it appears in place the first time.
	TEST(Core.HAL, GUIListBox_SelectionBarSlides)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(300.0f, 300.0f);

		auto list = std::make_shared<GUI::Elements::list_box>();
		list->docking = GUI::dock::NONE;
		list->width_size = GUI::size_type::FIXED;
		list->height_size = GUI::size_type::FIXED;
		list->size = vec2(200.0f, 200.0f);
		std::vector<GUI::Elements::list_element::ptr> items;
		for (const char* name : { "One", "Two", "Three", "Four" })
			items.push_back(list->add_item(name));
		ui.add_child(list);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};

		frame(0.016f);
		frame(0.016f);
		const float first_y = list->highlight().y;
		ASSERT_TRUE(list->highlight().alpha > 0.0f);                       // the first item starts selected
		ASSERT_TRUE(std::abs(first_y - (items[0]->get_render_bounds().y - list->contents->get_render_bounds().y)) < 1.0f);

		list->on_select(items[3].get());
		const float target = items[3]->get_render_bounds().y - list->contents->get_render_bounds().y;
		ASSERT_TRUE(target > first_y + 10.0f);

		frame(0.016f);
		ASSERT_TRUE(list->highlight().y > first_y + 0.5f && list->highlight().y < target - 0.5f);   // on its way
		for (int i = 0; i < 40; i++)
			frame(0.016f);
		ASSERT_TRUE(std::abs(list->highlight().y - target) < 0.5f);       // arrived

		ui.window_animations = false;
		list->on_select(items[1].get());
		frame(0.016f);
		const float second = items[1]->get_render_bounds().y - list->contents->get_render_bounds().y;
		ASSERT_TRUE(std::abs(list->highlight().y - second) < 0.5f);       // off: snaps
	}

	// The row under the cursor gets a fainter bar that slides between rows and fades
	// when the cursor leaves the list.
	TEST(Core.HAL, GUIListBox_HoverBarFollowsCursor)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();   // hit testing needs a shared-owned root
		auto& ui = *ui_owner;
		ui.size = vec2(300.0f, 300.0f);

		auto list = std::make_shared<GUI::Elements::list_box>();
		list->docking = GUI::dock::NONE;
		list->width_size = GUI::size_type::FIXED;
		list->height_size = GUI::size_type::FIXED;
		list->size = vec2(200.0f, 200.0f);
		std::vector<GUI::Elements::list_element::ptr> items;
		for (const char* name : { "One", "Two", "Three", "Four" })
			items.push_back(list->add_item(name));
		ui.add_child(list);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto rel = [&](int i) { return items[i]->get_render_bounds().y - list->contents->get_render_bounds().y; };
		auto center = [&](int i) { const rect b = items[i]->get_render_bounds(); return vec2(b.x + b.w * 0.5f, b.y + b.h * 0.5f); };

		frame(0.016f);
		frame(0.016f);
		ASSERT_TRUE(list->hover_highlight().alpha == 0.0f);                // nothing under the cursor

		ui.mouse_move_event_internal(center(1));
		for (int i = 0; i < 40; i++)
			frame(0.016f);
		ASSERT_TRUE(list->hover_highlight().alpha > 0.99f);
		ASSERT_TRUE(std::abs(list->hover_highlight().y - rel(1)) < 0.5f);

		ui.mouse_move_event_internal(center(3));
		frame(0.016f);
		ASSERT_TRUE(list->hover_highlight().y > rel(1) + 0.5f && list->hover_highlight().y < rel(3) - 0.5f);   // sliding
		for (int i = 0; i < 40; i++)
			frame(0.016f);
		ASSERT_TRUE(std::abs(list->hover_highlight().y - rel(3)) < 0.5f);

		ui.mouse_move_event_internal(vec2(290.0f, 290.0f));                // off the list
		for (int i = 0; i < 60; i++)
			frame(0.016f);
		ASSERT_TRUE(list->hover_highlight().alpha == 0.0f);
	}

	// The tree's expand arrow turns between right and down instead of swapping.
	TEST(Core.HAL, GUITree_ChevronTurns)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(100.0f, 100.0f);

		auto icon = std::make_shared<GUI::Elements::toogle_icon>();
		ui.add_child(icon);
		ui.process_ui(0.016f);
		ASSERT_TRUE(icon->turn_amount() == 0.0f);                          // collapsed: pointing right

		icon->set_expanded(true);
		ui.process_ui(0.05f);
		ASSERT_TRUE(icon->turn_amount() > 0.0f && icon->turn_amount() < 1.0f);   // turning
		for (int i = 0; i < 10; i++)
			ui.process_ui(0.05f);
		ASSERT_TRUE(icon->turn_amount() == 1.0f);                          // pointing down

		icon->toogle(true);                                                // the old inverted call: true = collapsed
		for (int i = 0; i < 10; i++)
			ui.process_ui(0.05f);
		ASSERT_TRUE(icon->turn_amount() == 0.0f);

		ui.window_animations = false;
		icon->set_expanded(true);
		ASSERT_TRUE(icon->turn_amount() == 1.0f);                          // off: jumps
	}

	// Ribbon tabs: the first tab shows, selecting another swaps the strips with a fade,
	// a hidden (contextual) tab hands the selection on, and minimising collapses the
	// strip to the tab bar and back. The touch set makes everything finger-sized.
	TEST(Core.HAL, GUIRibbon_TabsMinimizeAndTouch)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(800.0f, 400.0f);

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto home = rb->add_tab("Home");
		auto view = rb->add_tab("View");
		auto object = rb->add_tab("Object", true);
		home->add_group("File")->add_large("A", "New");
		view->add_group("Show")->add_large("B", "Grid");
		object->add_group("Transform")->add_large("C", "Move");
		ui.add_child(rb);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto settle = [&] { for (int i = 0; i < 30; i++) frame(0.03f); };

		settle();
		ASSERT_TRUE(rb->selected() == 0);
		ASSERT_TRUE(home->visible.get() && !view->visible.get() && !object->visible.get());
		ASSERT_TRUE(std::abs(rb->get_render_bounds().h - (28.0f + 96.0f)) < 2.0f);   // tab bar + strip

		rb->select(1);
		frame(0.03f);
		ASSERT_TRUE(view->visible.get() && !home->visible.get());
		ASSERT_TRUE(view->layer_opacity < 1.0f);                                    // fading in
		settle();
		ASSERT_TRUE(view->layer_opacity == 1.0f);
		ASSERT_TRUE(view->margin->left == 0.0f);                                     // slid into place

		rb->set_tab_visible(1, false);                                               // the selected tab goes away
		ASSERT_TRUE(rb->selected() == 0 && home->visible.get() && !view->visible.get());
		rb->set_tab_visible(2, false);
		rb->select(2);                                                               // a hidden tab can't be selected
		ASSERT_TRUE(rb->selected() == 0);
		rb->set_tab_visible(2, true);
		rb->select(2);
		ASSERT_TRUE(rb->selected() == 2);
		rb->select(0);

		rb->set_minimized(true);
		frame(0.03f);
		ASSERT_TRUE(rb->get_render_bounds().h > 28.0f + 1.0f);                       // collapsing, not cut
		settle();
		ASSERT_TRUE(std::abs(rb->get_render_bounds().h - 28.0f) < 2.0f);              // just the bar
		rb->set_minimized(false);
		settle();
		ASSERT_TRUE(std::abs(rb->get_render_bounds().h - (28.0f + 96.0f)) < 2.0f);

		rb->set_touch_mode(true);
		settle();
		ASSERT_TRUE(std::abs(rb->get_render_bounds().h - (40.0f + 136.0f)) < 2.0f);
		rb->set_touch_mode(false);
		settle();
		ASSERT_TRUE(std::abs(rb->get_render_bounds().h - (28.0f + 96.0f)) < 2.0f);
	}

	// Buttons: a large one fills the group's height, small ones stack three high then
	// start a new column of equal-width buttons; is_enabled gates clicks, is_checked is
	// asked every frame, and a menu button opens its menu under it.
	TEST(Core.HAL, GUIRibbon_ButtonsStateAndLayout)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(800.0f, 400.0f);

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto home = rb->add_tab("Home");
		auto group = home->add_group("Clipboard");

		int clicks = 0;
		bool enabled = false;
		bool checked = false;
		auto large = group->add_large("A", "Paste", [&] { clicks++; });
		large->is_enabled = [&] { return enabled; };
		large->is_checked = [&] { return checked; };
		auto s1 = group->add_small("B", "Cut");
		auto s2 = group->add_small("C", "Copy file");
		auto s3 = group->add_small("D", "Dup");
		auto s4 = group->add_small("E", "Next column");
		auto menu = std::make_shared<GUI::Elements::menu_list>();
		menu->add_item("Mesh");
		auto dropdown = group->add_large("F", "Add", {}, menu);
		ui.add_child(rb);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		for (int i = 0; i < 10; i++)
			frame(0.03f);

		const rect L = large->get_render_bounds();
		const rect b1 = s1->get_render_bounds(), b2 = s2->get_render_bounds(), b3 = s3->get_render_bounds(), b4 = s4->get_render_bounds();
		ASSERT_TRUE(std::abs(L.h - 72.0f) < 1.0f);
		ASSERT_TRUE(std::abs(b1.h - 24.0f) < 1.0f);
		ASSERT_TRUE(std::abs(b1.x - b2.x) < 1.0f && std::abs(b2.x - b3.x) < 1.0f);        // one column
		ASSERT_TRUE(b2.y > b1.y + 20.0f && b3.y > b2.y + 20.0f);                          // stacked
		ASSERT_TRUE(std::abs(b1.w - b2.w) < 1.0f && std::abs(b2.w - b3.w) < 1.0f);        // equal widths
		ASSERT_TRUE(b1.w > 40.0f);
		ASSERT_TRUE(b4.x > b1.x + b1.w - 1.0f);                                           // the fourth starts a new column
		ASSERT_TRUE(L.x < b1.x);                                                          // buttons keep their order
		ASSERT_TRUE(dropdown->get_render_bounds().x > b4.x);

		// Disabled: a click does nothing. Enabled: it calls back.
		large->on_mouse_action(mouse_action::UP, mouse_button::LEFT, vec2(L.x + 2.0f, L.y + 2.0f));
		ASSERT_TRUE(clicks == 0);
		enabled = true;
		frame(0.03f);
		large->on_mouse_action(mouse_action::UP, mouse_button::LEFT, vec2(L.x + 2.0f, L.y + 2.0f));
		ASSERT_TRUE(clicks == 1);

		// The state callbacks are read per frame, not cached.
		checked = true;
		frame(0.03f);
		checked = false;
		frame(0.03f);

		// A menu button opens its menu under itself, and a second click closes it.
		ASSERT_TRUE(!menu->is_self_open());
		dropdown->activate();
		ASSERT_TRUE(menu->is_self_open());
		const rect D = dropdown->get_render_bounds();
		ASSERT_TRUE(std::abs(menu->pos->x - D.x) < 1.0f && std::abs(menu->pos->y - (D.y + D.h)) < 1.0f);
		ASSERT_TRUE(std::abs(menu->size->x - D.w) < 1.0f);                                 // as wide as the button
		dropdown->activate();
		for (int i = 0; i < 12; i++)
			frame(0.03f);
		ASSERT_TRUE(!menu->is_self_open());
	}

	// Compact forms: when the ribbon gets narrower its groups shrink through small
	// buttons and icons to a single popup button, right to left, and come back when
	// there is room again.
	TEST(Core.HAL, GUIRibbon_CompactForms)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(1400.0f, 400.0f);

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto home = rb->add_tab("Home");
		for (const char* title : { "Clipboard", "Font", "Paragraph", "Styles" })
		{
			auto g = home->add_group(title);
			g->add_large("A", "Paste it");
			g->add_large("B", "Cut it");
			g->add_small("C", "Copy file");
			g->add_small("D", "Format");
			g->add_small("E", "Select");
		}
		ui.add_child(rb);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto settle = [&] { for (int i = 0; i < 6; i++) frame(0.03f); };
		auto available = [&] { return rb->get_render_bounds().w - 8.0f; };
		auto level = [&](int i) { return home->group(i)->get_level(); };

		settle();
		for (int i = 0; i < 4; i++)
			ASSERT_TRUE(level(i) == 0);                                              // roomy: as authored
		const float full = home->measure_total();
		ASSERT_TRUE(full < available());

		// Narrower than the full width: the right-most groups give up room first.
		ui.size = vec2(full * 0.8f, 400.0f);
		settle();
		bool any = false;
		for (int i = 0; i < 4; i++)
			any = any || level(i) > 0;
		ASSERT_TRUE(any);
		for (int i = 0; i + 1 < 4; i++)
			ASSERT_TRUE(level(i) <= level(i + 1));                                    // never a left group further down than one on its right
		ASSERT_TRUE(home->measure_total() <= available() + 0.5f);                     // and it fits

		// Very narrow: every group is a popup button holding its buttons.
		ui.size = vec2(200.0f, 400.0f);
		settle();
		for (int i = 0; i < 4; i++)
		{
			ASSERT_TRUE(level(i) == GUI::Elements::ribbon_group::max_level);
			ASSERT_TRUE(home->group(i)->get_popup() != nullptr);
			ASSERT_TRUE(home->group(i)->get_popup()->contents->get_child_count() == 1);   // the group's body moved in
		}

		// Room again: back to the authored form, the bodies back in their groups.
		ui.size = vec2(1400.0f, 400.0f);
		settle();
		for (int i = 0; i < 4; i++)
		{
			ASSERT_TRUE(level(i) == 0);
			ASSERT_TRUE(home->group(i)->get_popup()->contents->get_child_count() == 0);
		}
		ASSERT_TRUE(std::abs(home->measure_total() - full) < 1.0f);
	}

	// A drop-down or collapsed-group popup closes on a press anywhere else: on another
	// control, on empty ribbon, on empty window. Real mouse events through the UI.
	TEST(Core.HAL, GUIRibbon_PopupClosesOnOutsideClick)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(1000.0f, 500.0f);

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto home = rb->add_tab("Home");
		auto group = home->add_group("Create");
		auto menu = std::make_shared<GUI::Elements::menu_list>();
		menu->add_item("Mesh");
		auto dropdown = group->add_large("A", "Add", {}, menu);
		ui.add_child(rb);

		auto below = std::make_shared<GUI::Elements::colored_rect>();
		below->color = float4(0.2f, 0.2f, 0.2f, 1.0f);
		below->docking = GUI::dock::FILL;
		below->clickable = true;
		ui.add_child(below);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto settle = [&] { for (int i = 0; i < 15; i++) frame(0.03f); };
		auto click_at = [&](vec2 p)
		{
			ui.mouse_move_event_internal(p);
			frame(0.016f);
			ui.mouse_action_event_internal(mouse_action::DOWN, mouse_button::LEFT, p);
			frame(0.016f);
			ui.mouse_action_event_internal(mouse_action::UP, mouse_button::LEFT, p);
			settle();
		};

		settle();
		const rect R = rb->get_render_bounds();
		const rect B = below->get_render_bounds();
		ASSERT_TRUE(B.h > 50.0f);

		// On another control.
		dropdown->activate();
		settle();
		ASSERT_TRUE(menu->is_self_open());
		click_at(vec2(B.x + B.w * 0.5f, B.y + B.h * 0.5f));
		ASSERT_TRUE(!menu->is_self_open());

		// On an empty part of the ribbon (far right of the tab bar, and of the strip).
		dropdown->activate();
		settle();
		ASSERT_TRUE(menu->is_self_open());
		click_at(vec2(R.x + R.w - 40.0f, R.y + 10.0f));
		ASSERT_TRUE(!menu->is_self_open());

		dropdown->activate();
		settle();
		ASSERT_TRUE(menu->is_self_open());
		click_at(vec2(R.x + R.w - 40.0f, R.y + 28.0f + 40.0f));
		ASSERT_TRUE(!menu->is_self_open());

		// A press on the button itself closes it, and the release does not bring it back.
		dropdown->activate();
		settle();
		ASSERT_TRUE(menu->is_self_open());
		const rect D = dropdown->get_render_bounds();
		click_at(vec2(D.x + D.w * 0.5f, D.y + D.h * 0.5f));
		ASSERT_TRUE(!menu->is_self_open());
	}

	// A minimised ribbon shows only the tab bar; clicking a tab opens its strip as a
	// popup that closes again on a click anywhere else, like a menu. The arrow button
	// brings the strip back for good.
	TEST(Core.HAL, GUIRibbon_MinimizedStripIsAPopup)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(1000.0f, 500.0f);

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto home = rb->add_tab("Home");
		home->add_group("File")->add_large("A", "New");
		auto view = rb->add_tab("View");
		view->add_group("Show")->add_large("B", "Grid");
		ui.add_child(rb);

		auto below = std::make_shared<GUI::Elements::colored_rect>();
		below->color = float4(0.2f, 0.2f, 0.2f, 1.0f);
		below->docking = GUI::dock::FILL;
		below->clickable = true;
		ui.add_child(below);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto settle = [&] { for (int i = 0; i < 20; i++) frame(0.03f); };
		auto click_at = [&](vec2 p)
		{
			ui.mouse_move_event_internal(p);
			frame(0.016f);
			ui.mouse_action_event_internal(mouse_action::DOWN, mouse_button::LEFT, p);
			frame(0.016f);
			ui.mouse_action_event_internal(mouse_action::UP, mouse_button::LEFT, p);
			settle();
		};

		settle();
		rb->set_minimized(true);
		settle();
		ASSERT_TRUE(std::abs(rb->get_render_bounds().h - 28.0f) < 2.0f);             // only the bar
		ASSERT_TRUE(rb->get_strip_popup() != nullptr);
		ASSERT_TRUE(!rb->get_strip_popup()->is_self_open());
		ASSERT_TRUE(!rb->tab_looks_selected(0));                                        // no tab on show, so none looks selected

		click_at(vec2(30.0f, 14.0f));                                                  // the first tab
		ASSERT_TRUE(rb->get_strip_popup()->is_self_open());
		ASSERT_TRUE(rb->tab_looks_selected(0));                                         // its strip is open
		ASSERT_TRUE(std::abs(rb->get_render_bounds().h - 28.0f) < 2.0f);             // nothing pushed down

		const rect B = below->get_render_bounds();
		click_at(vec2(B.x + B.w * 0.5f, B.y + B.h * 0.5f));                           // elsewhere
		ASSERT_TRUE(!rb->get_strip_popup()->is_self_open());
		ASSERT_TRUE(!rb->tab_looks_selected(0));                                        // and it can highlight on hover again

		click_at(vec2(30.0f, 14.0f));                                                  // and again
		ASSERT_TRUE(rb->get_strip_popup()->is_self_open());

		// The arrow button un-minimises for good: the strip is back in the ribbon.
		rb->set_minimized(false);
		settle();
		ASSERT_TRUE(!rb->get_strip_popup()->is_self_open());
		ASSERT_TRUE(std::abs(rb->get_render_bounds().h - (28.0f + 96.0f)) < 2.0f);
	}

	// A double click on a tab title does what the arrow button does, in both directions:
	// minimised ribbon back to full, full ribbon to minimised.
	TEST(Core.HAL, GUIRibbon_DoubleClickTabTogglesMinimize)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(1000.0f, 500.0f);

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto home = rb->add_tab("Home");
		home->add_group("File")->add_large("A", "New");
		rb->add_tab("View")->add_group("Show")->add_large("B", "Grid");
		ui.add_child(rb);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto settle = [&] { for (int i = 0; i < 25; i++) frame(0.03f); };
		auto press = [&](vec2 p)
		{
			ui.mouse_action_event_internal(mouse_action::DOWN, mouse_button::LEFT, p);
			frame(0.016f);
			ui.mouse_action_event_internal(mouse_action::UP, mouse_button::LEFT, p);
			frame(0.016f);
		};
		auto double_click = [&](vec2 p)
		{
			ui.mouse_move_event_internal(p);
			frame(0.016f);
			press(p);
			press(p);
			settle();
		};

		settle();
		ASSERT_TRUE(!rb->is_minimized());

		const vec2 tab(30.0f, 14.0f);
		double_click(tab);
		ASSERT_TRUE(rb->is_minimized());
		ASSERT_TRUE(std::abs(rb->get_render_bounds().h - 28.0f) < 2.0f);

		double_click(tab);
		ASSERT_TRUE(!rb->is_minimized());
		ASSERT_TRUE(std::abs(rb->get_render_bounds().h - (28.0f + 96.0f)) < 2.0f);

		// Two slow clicks are two single clicks: nothing toggles.
		ui.mouse_move_event_internal(tab);
		frame(0.016f);
		press(tab);
		settle();
		press(tab);
		settle();
		ASSERT_TRUE(!rb->is_minimized());
	}

	// A window that draws its own title bar asks the UI what each point is (WM_NCHITTEST):
	// the ribbon's tab bar is the caption, its window buttons report as minimise / maximise /
	// close, and tab titles and the strip stay ordinary UI. A click on a window button runs
	// the UI's window command handler.
	TEST(Core.HAL, GUIRibbon_WindowControlsAndHitTest)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		constexpr float W = 1000.0f;
		ui.size = vec2(W, 500.0f);

		std::vector<window_command> commands;
		ui.window_command_handler = [&](window_command c) { commands.push_back(c); };
		ui.window_maximized = [] { return false; };

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		rb->set_window_icon("app");
		rb->set_window_controls(true);
		rb->set_window_title("Spectrum");
		auto home = rb->add_tab("Home");
		home->add_group("File")->add_large("A", "New");
		ui.add_child(rb);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		for (int i = 0; i < 10; i++)
			frame(0.03f);

		// Buttons are 46 px wide at the default metrics; close is the outermost.
		ASSERT_TRUE(ui.hit_test_window(vec2(W - 23.0f, 14.0f)) == window_hit::close);
		ASSERT_TRUE(ui.hit_test_window(vec2(W - 46.0f - 23.0f, 14.0f)) == window_hit::maximize);
		ASSERT_TRUE(ui.hit_test_window(vec2(W - 92.0f - 23.0f, 14.0f)) == window_hit::minimize);

		ASSERT_TRUE(ui.hit_test_window(vec2(W * 0.5f, 14.0f)) == window_hit::caption);   // empty tab bar: drag the window
		ASSERT_TRUE(ui.hit_test_window(vec2(18.0f, 14.0f)) == window_hit::caption);      // the app icon is part of the title bar
		ASSERT_TRUE(ui.hit_test_window(vec2(70.0f, 14.0f)) == window_hit::client);       // a tab title is a click
		ASSERT_TRUE(ui.hit_test_window(vec2(W - 138.0f - 14.0f, 14.0f)) == window_hit::client);   // the collapse button too
		ASSERT_TRUE(ui.hit_test_window(vec2(300.0f, 28.0f + 40.0f)) == window_hit::client);       // the strip
		ASSERT_TRUE(ui.hit_test_window(vec2(300.0f, 400.0f)) == window_hit::client);              // below the ribbon

		// Clicking a window button runs the command; the same pointer events the window forwards.
		const vec2 close_at(W - 23.0f, 14.0f);
		ui.mouse_move_event_internal(close_at);
		frame(0.016f);
		ui.mouse_action_event_internal(mouse_action::DOWN, mouse_button::LEFT, close_at);
		frame(0.016f);
		ui.mouse_action_event_internal(mouse_action::UP, mouse_button::LEFT, close_at);
		frame(0.016f);
		ASSERT_TRUE(commands.size() == 1 && commands[0] == window_command::close);

		// Without window controls the tab bar is plain UI again.
		rb->set_window_controls(false);
		for (int i = 0; i < 5; i++)
			frame(0.03f);
		ASSERT_TRUE(ui.hit_test_window(vec2(W * 0.5f, 14.0f)) == window_hit::client);
	}

	// Group priority: when the ribbon runs out of width, the groups with the lowest priority
	// shrink first, so a high-priority group keeps its full form longer.
	TEST(Core.HAL, GUIRibbon_GroupPriorityDecidesWhoShrinksFirst)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(1400.0f, 400.0f);

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto home = rb->add_tab("Home");
		for (const char* title : { "Clipboard", "Font", "Styles" })
		{
			auto g = home->add_group(title);
			g->add_large("A", "Paste it");
			g->add_large("B", "Cut it");
			g->add_small("C", "Copy file");
			g->add_small("D", "Format");
			g->add_small("E", "Select");
		}
		home->group(2)->set_priority(5);                                  // the right-most one matters most
		ui.add_child(rb);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto settle = [&] { for (int i = 0; i < 6; i++) frame(0.03f); };
		auto level = [&](int i) { return home->group(i)->get_level(); };

		settle();
		const float full = home->measure_total();

		ui.size = vec2(full * 0.86f, 400.0f);
		settle();
		ASSERT_TRUE(level(0) + level(1) > 0);                             // somebody shrank
		ASSERT_TRUE(level(2) <= level(0) && level(2) <= level(1));        // but not the one that matters most
	}

	// A split button has two parts: the main part calls on_click, the arrow part opens the menu.
	TEST(Core.HAL, GUIRibbon_SplitButtonHalves)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(800.0f, 400.0f);

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto group = rb->add_tab("Home")->add_group("Clipboard");

		int clicks = 0;
		auto menu = std::make_shared<GUI::Elements::menu_list>();
		menu->add_item("Paste special");
		auto large = group->add_large("A", "Paste", [&] { clicks++; }, menu, true);
		ui.add_child(rb);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto press = [&](GUI::Elements::ribbon_button& b, vec2 p)
		{
			b.on_mouse_action(mouse_action::DOWN, mouse_button::LEFT, p);
			b.on_mouse_action(mouse_action::UP, mouse_button::LEFT, p);
		};
		for (int i = 0; i < 10; i++)
			frame(0.03f);

		const rect L = large->get_render_bounds();
		ASSERT_TRUE(large->is_split());

		press(*large, vec2(L.x + L.w * 0.5f, L.y + 6.0f));                 // the icon part
		ASSERT_TRUE(clicks == 1);
		ASSERT_TRUE(!menu->is_self_open());

		press(*large, vec2(L.x + L.w * 0.5f, L.y + L.h - 4.0f));           // the arrow part
		ASSERT_TRUE(clicks == 1);                                           // not the main action
		ASSERT_TRUE(menu->is_self_open());

		menu->self_close();
		for (int i = 0; i < 12; i++)
			frame(0.03f);
		ASSERT_TRUE(!menu->is_self_open());

		// A small one: the strip at the right edge is the arrow.
		auto small = group->add_small("B", "Paste text", [&] { clicks += 10; }, menu, true);
		for (int i = 0; i < 6; i++)
			frame(0.03f);
		const rect S = small->get_render_bounds();

		press(*small, vec2(S.x + 8.0f, S.y + S.h * 0.5f));
		ASSERT_TRUE(clicks == 11);
		press(*small, vec2(S.x + S.w - 5.0f, S.y + S.h * 0.5f));
		ASSERT_TRUE(clicks == 11);
		ASSERT_TRUE(menu->is_self_open());
	}

	// When even the smallest forms don't fit, the strip scrolls: the wheel moves it, the
	// range follows the width, and it is back to zero when there is room.
	TEST(Core.HAL, GUIRibbon_OverflowScrolls)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(1400.0f, 400.0f);

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto home = rb->add_tab("Home");
		for (const char* title : { "Clipboard", "Font", "Paragraph", "Styles", "Editing" })
		{
			auto g = home->add_group(title);
			g->add_large("A", "Paste it");
			g->add_small("C", "Copy file");
			g->add_small("D", "Format");
		}
		ui.add_child(rb);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto settle = [&] { for (int i = 0; i < 6; i++) frame(0.03f); };

		settle();
		ASSERT_TRUE(home->get_max_scroll() == 0.0f);

		ui.size = vec2(150.0f, 400.0f);
		settle();
		ASSERT_TRUE(home->get_max_scroll() > 0.0f);
		ASSERT_TRUE(home->get_scroll() == 0.0f);

		home->on_wheel(mouse_wheel::VERTICAL, -1.0f, vec2(10.0f, 60.0f));      // wheel down scrolls towards the end
		ASSERT_TRUE(home->get_scroll() > 0.0f);
		for (int i = 0; i < 40; i++)
			home->on_wheel(mouse_wheel::VERTICAL, -1.0f, vec2(10.0f, 60.0f));
		ASSERT_TRUE(home->get_scroll() == home->get_max_scroll());              // clamped at the end
		home->on_wheel(mouse_wheel::VERTICAL, 1.0f, vec2(10.0f, 60.0f));
		ASSERT_TRUE(home->get_scroll() < home->get_max_scroll());

		ui.size = vec2(1400.0f, 400.0f);
		settle();
		ASSERT_TRUE(home->get_max_scroll() == 0.0f);
		ASSERT_TRUE(home->get_scroll() == 0.0f);
	}

	// Every label gets a soft shadow by default (UI Effects > Text shadow): white text on a mid
	// grey background leaves darker-than-background pixels only when the shadow is on; a label
	// can opt out, and one with its own shadow colour keeps that.
	TEST(Core.HAL, GUILabel_DefaultTextShadow)
	{
		THREAD_SCOPE(GUI);
		constexpr uint W = 200, H = 80;

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2((float)W, (float)H);

		auto background = std::make_shared<GUI::Elements::colored_rect>();
		background->color   = float4(0.5f, 0.5f, 0.5f, 1.0f);
		background->docking = GUI::dock::FILL;
		ui.add_child(background);

		auto label = std::make_shared<GUI::Elements::label>();
		label->font_size = 32;
		label->text = "Shadow";
		label->docking = GUI::dock::NONE;
		label->pos = vec2(20.0f, 20.0f);
		ui.add_child(label);

		// Pixels clearly darker than the background, over the label's box and a margin below/right of it.
		auto dark_pixels = [&]
		{
			const auto img = render_ui(ui, W, H);
			int count = 0;
			for (uint y = 15; y < 70; y++)
				for (uint x = 15; x < 190; x++)
					if (img.at(x, y, 0) < 0.42f)
						count++;
			return count;
		};

		ui.text_shadow_enabled = false;
		ASSERT_TRUE(dark_pixels() == 0);                                     // white text only

		ui.text_shadow_enabled = true;
		ASSERT_TRUE(dark_pixels() > 20);                                     // the shadow shows

		label->default_shadow = false;                                       // opted out
		ASSERT_TRUE(dark_pixels() == 0);

		label->default_shadow = true;
		const int soft_count = dark_pixels();
		ui.text_shadow_softness = 0.0f;                                      // crisp: more pixels at full strength, fewer spread
		ASSERT_TRUE(dark_pixels() > 20);
		ui.text_shadow_softness = 3.0f;                                      // very soft: reaches further out
		ASSERT_TRUE(dark_pixels() != soft_count);
		ui.text_shadow_softness = 1.0f;

		ui.text_shadow_strength = 0.0f;                                      // the setting can fade it away
		ASSERT_TRUE(dark_pixels() == 0);
	}

	// Shortcut texts: modifiers and a key joined by '+', any case, written back in one form.
	TEST(Core.HAL, GUICommands_KeyChordParsing)
	{
		auto ctrl_s = GUI::key_chord::parse("ctrl + s");
		ASSERT_TRUE(ctrl_s.has_value());
		ASSERT_TRUE(ctrl_s->ctrl && !ctrl_s->shift && !ctrl_s->alt && ctrl_s->key == 'S');
		ASSERT_TRUE(ctrl_s->to_string() == "Ctrl+S");

		auto all = GUI::key_chord::parse("Alt+Shift+Ctrl+F5");
		ASSERT_TRUE(all.has_value() && all->ctrl && all->shift && all->alt && all->key == 0x74);
		ASSERT_TRUE(all->to_string() == "Ctrl+Shift+Alt+F5");

		ASSERT_TRUE(GUI::key_chord::parse("Del")->key == 0x2E);
		ASSERT_TRUE(GUI::key_chord::parse("Esc")->key == 0x1B);
		ASSERT_TRUE(GUI::key_chord::parse("Ctrl+Left")->key == 0x25);
		ASSERT_TRUE(GUI::key_chord::parse("7")->key == '7');
		ASSERT_TRUE(GUI::key_chord::parse("Ctrl+Del")->to_string() == "Ctrl+Delete");

		// Text that is not a shortcut.
		ASSERT_TRUE(!GUI::key_chord::parse("").has_value());
		ASSERT_TRUE(!GUI::key_chord::parse("Ctrl+").has_value());          // no key
		ASSERT_TRUE(!GUI::key_chord::parse("Ctrl").has_value());
		ASSERT_TRUE(!GUI::key_chord::parse("Ctrl+A+B").has_value());       // two keys
		ASSERT_TRUE(!GUI::key_chord::parse("Ctrl+Banana").has_value());
		ASSERT_TRUE(!GUI::key_chord::parse("F13").has_value());
	}

	// The registry: one command per id (adding it again returns the same one), lookup, running
	// with the enabled state respected, and the titles falling back sensibly.
	TEST(Core.HAL, GUICommands_RegistryBasics)
	{
		GUI::command_registry commands;
		int saves = 0;
		bool allowed = false;

		commands.add("file.save").title("Save").shortcut("Ctrl+S").enabled([&] { return allowed; }).run([&] { saves++; });
		ASSERT_TRUE(&commands.add("file.save") == commands.find("file.save").get());   // the same command
		ASSERT_TRUE(commands.all().size() == 1);

		ASSERT_TRUE(commands.find("nothing") == nullptr);
		ASSERT_TRUE(!commands.run("nothing"));

		ASSERT_TRUE(!commands.run("file.save") && saves == 0);              // disabled: not run
		allowed = true;
		ASSERT_TRUE(commands.run("file.save") && saves == 1);

		auto save = commands.find("file.save");
		ASSERT_TRUE(save->get_tooltip() == "Save (Ctrl+S)");

		auto& bare = commands.add("view.grid");                             // only an id so far
		ASSERT_TRUE(bare.get_title() == "view.grid");
		ASSERT_TRUE(bare.get_short_title() == "view.grid");
		ASSERT_TRUE(!bare.is_toggle() && bare.is_enabled() && !bare.execute());   // nothing to run

		commands.add("view.grid").title("Grid").short_title("G").checked([] { return true; });
		ASSERT_TRUE(bare.get_short_title() == "G" && bare.is_toggle() && bare.is_checked());

		ASSERT_TRUE(commands.find_shortcut(*GUI::key_chord::parse("Ctrl+S")) == save);
		ASSERT_TRUE(commands.find_shortcut(*GUI::key_chord::parse("Ctrl+Shift+S")) == nullptr);
	}

	// Shortcuts: a key hook runs the command for a chord, but not a disabled one, not a plain key
	// while a text field has the focus, and not while the ribbon's Alt mode holds the keyboard.
	TEST(Core.HAL, GUICommands_ShortcutDispatch)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(800.0f, 400.0f);

		GUI::command_registry commands;
		int saves = 0, grids = 0;
		bool allowed = true;
		commands.add("file.save").shortcut("Ctrl+S").enabled([&] { return allowed; }).run([&] { saves++; });
		commands.add("view.grid").shortcut("G").run([&] { grids++; });
		commands.install_shortcuts(ui);

		auto field = std::make_shared<GUI::Elements::edit_text>();
		ui.add_child(field);

		auto press = [&](long code, bool ctrl)
		{
			key_mods mods;
			mods.ctrl = ctrl;
			ui.key_action_event_internal(key_action::DOWN, code, mods);
			ui.key_action_event_internal(key_action::UP, code, mods);
		};

		press('S', true);
		ASSERT_TRUE(saves == 1);
		press('S', false);                                                   // no Ctrl: another chord
		ASSERT_TRUE(saves == 1);

		allowed = false;
		press('S', true);                                                    // disabled: not run
		ASSERT_TRUE(saves == 1);
		allowed = true;

		press('G', false);                                                   // a plain key, nothing focused
		ASSERT_TRUE(grids == 1);

		field->focus();
		press('G', false);                                                   // typing in a field: left alone
		ASSERT_TRUE(grids == 1);
		press('S', true);                                                    // with Ctrl it still runs
		ASSERT_TRUE(saves == 2);
		field->unfocus();

		ui.keys_captured = true;                                             // the ribbon's Alt mode
		press('S', true);
		ASSERT_TRUE(saves == 2);
	}

	// The adapters: a ribbon button takes everything from the command (and follows its state),
	// a menu item runs it.
	TEST(Core.HAL, GUICommands_RibbonAndMenuAdapters)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(800.0f, 400.0f);

		GUI::command_registry commands;
		int saves = 0;
		bool allowed = false, lit = false;
		commands.add("file.save").title("Save").short_title("Sv").icon("S").shortcut("Ctrl+S")
			.enabled([&] { return allowed; }).run([&] { saves++; });
		commands.add("view.grid").title("Grid").checked([&] { return lit; }).run([&] { lit = !lit; });

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto group = rb->add_tab("Home")->add_group("File");
		auto save = group->add_command(commands, "file.save");
		auto small_save = group->add_command(commands.find("file.save"), false);
		auto grid = group->add_command(commands, "view.grid");
		ASSERT_TRUE(group->add_command(commands, "nothing") == nullptr);
		ui.add_child(rb);

		ASSERT_TRUE(save->get_text() == "Save" && small_save->get_text() == "Sv");   // title, short title
		ASSERT_TRUE(save->get_icon() == "S");
		ASSERT_TRUE(save->tooltip == "Save (Ctrl+S)");

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		for (int i = 0; i < 6; i++)
			frame(0.03f);

		const rect S = save->get_render_bounds();
		save->on_mouse_action(mouse_action::UP, mouse_button::LEFT, vec2(S.x + 2.0f, S.y + 2.0f));
		ASSERT_TRUE(saves == 0);                                             // disabled: the button follows the command
		allowed = true;
		frame(0.03f);
		save->on_mouse_action(mouse_action::UP, mouse_button::LEFT, vec2(S.x + 2.0f, S.y + 2.0f));
		ASSERT_TRUE(saves == 1);

		const rect G = grid->get_render_bounds();
		grid->on_mouse_action(mouse_action::UP, mouse_button::LEFT, vec2(G.x + 2.0f, G.y + 2.0f));
		ASSERT_TRUE(lit);                                                    // a toggle command toggles

		// A menu item.
		GUI::Elements::menu_list menu;
		auto item = menu.add_command(commands.find("file.save"));
		ASSERT_TRUE(item->text == "Save");
		item->on_click(item);
		ASSERT_TRUE(saves == 2);
		allowed = false;
		item->on_click(item);
		ASSERT_TRUE(saves == 2);                                             // disabled
		ASSERT_TRUE(menu.add_command(nullptr) == nullptr);
	}

	// A finger drags an overflowing strip sideways; a finger that stays put is still a tap.
	TEST(Core.HAL, GUIRibbon_TouchDragScrollsOverflow)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(1400.0f, 400.0f);

		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto home = rb->add_tab("Home");
		for (const char* title : { "Clipboard", "Font", "Paragraph", "Styles", "Editing" })
		{
			auto g = home->add_group(title);
			g->add_large("A", "Paste it");
			g->add_small("C", "Copy file");
			g->add_small("D", "Format");
		}
		ui.add_child(rb);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto settle = [&] { for (int i = 0; i < 6; i++) frame(0.03f); };

		ui.size = vec2(150.0f, 400.0f);
		settle();
		ASSERT_TRUE(home->get_max_scroll() > 0.0f);

		const float y = 28.0f + 40.0f;
		ui.on_touch_event(make_touch(GUI::touch_event::DOWN, 1, 120.0f, y));
		ui.on_touch_event(make_touch(GUI::touch_event::MOVE, 1, 118.0f, y));
		ASSERT_TRUE(home->get_scroll() == 0.0f);                                // inside the slop: a tap, no scrolling

		ui.on_touch_event(make_touch(GUI::touch_event::MOVE, 1, 90.0f, y));    // past it: the drag begins
		ui.on_touch_event(make_touch(GUI::touch_event::MOVE, 1, 40.0f, y));
		ASSERT_TRUE(home->get_scroll() > 0.0f);                                 // left drag shows what is to the right
		const float after_left = home->get_scroll();

		ui.on_touch_event(make_touch(GUI::touch_event::MOVE, 1, 100.0f, y));   // and back
		ASSERT_TRUE(home->get_scroll() < after_left);
		ui.on_touch_event(make_touch(GUI::touch_event::UP, 1, 100.0f, y));
	}

	// Keyboard access: Alt on its own shows letters over the tabs, a letter opens that tab
	// and shows letters over its buttons, a letter there runs the button.
	TEST(Core.HAL, GUIRibbon_KeyTips)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(1000.0f, 500.0f);

		int views = 0;
		auto rb = std::make_shared<GUI::Elements::ribbon>();
		auto home = rb->add_tab("Home");
		int deletes = 0;
		auto file_group = home->add_group("File");
		file_group->add_large("A", "Paste");
		file_group->add_large("B", "Delete", [&] { deletes++; });
		auto view = rb->add_tab("View");
		view->add_group("Show")->add_large("B", "Grid", [&] { views++; });
		ui.add_child(rb);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto key = [&](long code, key_action action)
		{
			ui.key_action_event_internal(action, code, key_mods());
			frame(0.016f);
		};
		auto tap = [&](long code) { key(code, key_action::DOWN); key(code, key_action::UP); };
		constexpr long alt = 0x12, escape = 0x1B;

		for (int i = 0; i < 6; i++)
			frame(0.03f);
		ASSERT_TRUE(rb->get_key_level() == 0);

		tap(alt);
		ASSERT_TRUE(rb->get_key_level() == 1);
		ASSERT_TRUE(ui.keys_captured);                                        // typed characters go nowhere meanwhile

		tap('V');                                                              // View
		ASSERT_TRUE(rb->selected() == 1);
		ASSERT_TRUE(rb->get_key_level() == 2);

		const auto targets = view->key_targets();
		ASSERT_TRUE(targets.size() == 1);
		ASSERT_TRUE(targets[0]->get_key_tip().size() == 1);
		tap(targets[0]->get_key_tip()[0]);                                     // the button's letter runs it
		ASSERT_TRUE(views == 1);
		ASSERT_TRUE(rb->get_key_level() == 0);
		ASSERT_TRUE(!ui.keys_captured);

		// Alt with another key is not a tap.
		key(alt, key_action::DOWN);
		tap('H');
		key(alt, key_action::UP);
		ASSERT_TRUE(rb->get_key_level() == 0);

		// Escape goes back a step, then out.
		tap(alt);
		tap('H');
		ASSERT_TRUE(rb->selected() == 0 && rb->get_key_level() == 2);
		tap(escape);
		ASSERT_TRUE(rb->get_key_level() == 1);
		tap(escape);
		ASSERT_TRUE(rb->get_key_level() == 0);

		// Arrow keys change tab.
		tap(alt);
		tap(0x27);                                                             // Right
		ASSERT_TRUE(rb->selected() == 1);
		tap(0x25);                                                             // Left
		ASSERT_TRUE(rb->selected() == 0);
		tap(escape);

		// In the strip the arrows move a focus ring over the buttons; Enter runs the focused one.
		tap(alt);
		tap('H');
		const auto home_targets = home->key_targets();
		ASSERT_TRUE(home_targets.size() == 2);
		ASSERT_TRUE(home_targets[0]->has_key_focus() && !home_targets[1]->has_key_focus());
		tap(0x27);                                                             // Right
		ASSERT_TRUE(!home_targets[0]->has_key_focus() && home_targets[1]->has_key_focus());
		tap(0x27);                                                             // already on the last one
		ASSERT_TRUE(home_targets[1]->has_key_focus());
		tap(0x25);                                                             // Left
		ASSERT_TRUE(home_targets[0]->has_key_focus());
		tap(0x28);                                                             // Down: the next one
		tap(0x0D);                                                             // Enter
		ASSERT_TRUE(deletes == 1);
		ASSERT_TRUE(rb->get_key_level() == 0);
		ASSERT_TRUE(!home_targets[1]->has_key_focus());                        // the ring goes with the mode

		// A mouse press leaves it too.
		tap(alt);
		ASSERT_TRUE(rb->get_key_level() == 1);
		ui.mouse_action_event_internal(mouse_action::DOWN, mouse_button::LEFT, vec2(500.0f, 300.0f));
		frame(0.016f);
		ASSERT_TRUE(rb->get_key_level() == 0);
		ASSERT_TRUE(!ui.keys_captured);
	}

	class gui_test_node : public ::tree<gui_test_node, my_unique_vector<std::shared_ptr<gui_test_node>>>
	{
		std::wstring name;
	public:
		using ptr = std::shared_ptr<gui_test_node>;
		gui_test_node(std::wstring n) : name(std::move(n)) {}
		std::wstring get_name() const override { return name; }
	};

	// Collapsing a tree branch slides its children shut instead of cutting them: the
	// tree's content height is in between mid-way, ends at the collapsed height, and
	// expanding goes back to the full height.
	TEST(Core.HAL, GUITree_BranchSlidesOpenAndShut)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(300.0f, 400.0f);

		auto root = std::make_shared<gui_test_node>(L"root");
		auto branch = std::make_shared<gui_test_node>(L"branch");
		root->add_child(branch);
		for (const wchar_t* n : { L"a", L"b", L"c" })
			branch->add_child(std::make_shared<gui_test_node>(n));

		using tree_type = GUI::Elements::tree<gui_test_node>;
		using element_type = GUI::Elements::tree_element<gui_test_node, GUI::Elements::tree_creator<gui_test_node>>;
		auto tree = std::make_shared<tree_type>();
		tree->docking = GUI::dock::NONE;
		tree->width_size = GUI::size_type::FIXED;
		tree->height_size = GUI::size_type::FIXED;
		tree->size = vec2(250.0f, 350.0f);
		ui.add_child(tree);
		tree->init(root.get());

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};
		auto content_height = [&] { return tree->contents->get_render_bounds().h; };

		for (int i = 0; i < 5; i++)
			frame(0.016f);
		const float full = content_height();
		ASSERT_TRUE(full > 50.0f);

		element_type* branch_element = nullptr;
		{
			FrameGraph::Graph graph;
			ui.create_graph(graph);
			for (const auto& e : graph.get_context<GUI::UIContext>().draw_infos)
				if (auto* t = dynamic_cast<element_type*>(e.elem))
					if (t->node == branch.get())
						branch_element = t;
		}
		ASSERT_TRUE(branch_element != nullptr);

		branch_element->toogle();                      // collapse
		frame(0.05f);
		frame(0.05f);
		ASSERT_TRUE(content_height() < full - 1.0f);   // closing
		ASSERT_TRUE(content_height() > full - 3.0f * 20.0f + 1.0f);   // but not shut yet
		for (int i = 0; i < 20; i++)
			frame(0.05f);
		const float collapsed = content_height();
		ASSERT_TRUE(collapsed < full - 20.0f);

		branch_element->toogle();                      // expand again
		// While the children are being measured nothing below may move: the content
		// must not grow past the collapsed height until the slide actually starts.
		for (int i = 0; i < 2; i++)
		{
			frame(0.001f);
			ASSERT_TRUE(content_height() <= collapsed + 0.5f);
		}
		frame(0.016f);
		frame(0.05f);
		ASSERT_TRUE(content_height() > collapsed - 0.5f && content_height() < full - 0.5f);   // opening
		for (int i = 0; i < 20; i++)
			frame(0.05f);
		ASSERT_TRUE(std::abs(content_height() - full) < 1.0f);        // fully open again
	}

	// A window fades in when added, and closing it fades it out and removes it only
	// when the fade ends (it stays in the tree, drawn, until then). Closing twice is
	// harmless, and cancel_close() brings it back. No GPU needed.
	TEST(Core.HAL, GUIWindow_FadesInAndClosesDelayed)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(400.0f, 300.0f);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};

		auto win = std::make_shared<GUI::Elements::window>();
		win->width_size  = GUI::size_type::FIXED;
		win->height_size = GUI::size_type::FIXED;
		win->size        = vec2(200.0f, 120.0f);
		ui.add_child(win);

		ASSERT_TRUE(win->layer_opacity == 0.0f);          // starts invisible
		frame(0.03f);
		ASSERT_TRUE(win->layer_opacity > 0.0f && win->layer_opacity < 1.0f);
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		ASSERT_TRUE(win->layer_opacity == 1.0f);          // arrived

		win->close();
		win->close();                                     // second call: nothing new
		ASSERT_TRUE(win->is_closing());
		frame(0.03f);
		ASSERT_TRUE(win->get_parent() != nullptr);        // still there mid-fade
		ASSERT_TRUE(win->layer_opacity < 1.0f);

		// Changed mind mid-fade.
		win->cancel_close();
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		ASSERT_TRUE(win->get_parent() != nullptr);
		ASSERT_TRUE(win->layer_opacity == 1.0f);

		win->close();
		for (int i = 0; i < 12; i++)
			frame(0.03f);
		ASSERT_TRUE(win->get_parent() == nullptr);        // gone once the fade ended
		ASSERT_TRUE(!win->is_closing());

		// Without the setting the window just goes.
		ui.window_animations = false;
		auto win2 = std::make_shared<GUI::Elements::window>();
		ui.add_child(win2);
		ASSERT_TRUE(win2->layer_opacity == 1.0f);
		win2->close();
		ASSERT_TRUE(win2->get_parent() == nullptr);
	}

	// Dragging a window's title bar lifts it (bigger shadow); letting go eases back.
	TEST(Core.HAL, GUIWindow_DragLiftsAndReleases)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(400.0f, 300.0f);
		ui.window_animations = false;

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};

		auto win = std::make_shared<GUI::Elements::window>();
		win->width_size  = GUI::size_type::FIXED;
		win->height_size = GUI::size_type::FIXED;
		win->size        = vec2(200.0f, 120.0f);
		win->pos         = vec2(50.0f, 50.0f);
		ui.add_child(win);
		auto* found = win->find_effect<GUI::Shadow>();
		ASSERT_TRUE(found != nullptr);
		auto& shadow = *found;
		frame(0.03f);
		ASSERT_TRUE(shadow.lift == 0.0f);

		shadow.lift_target = 1.0f;                           // what the dragger sets on mouse down
		frame(0.03f);
		ASSERT_TRUE(shadow.lift > 0.0f && shadow.lift < 1.0f); // eased, not a jump
		for (int i = 0; i < 6; i++)
			frame(0.03f);
		ASSERT_TRUE(shadow.lift == 1.0f);

		shadow.lift_target = 0.0f;
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		ASSERT_TRUE(shadow.lift == 0.0f);
	}

	// The progress fill eases toward a new value instead of jumping; the first frame snaps.
	TEST(Core.HAL, GUIProgressBar_EasesToValue)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(300.0f, 100.0f);

		auto bar = std::make_shared<GUI::Elements::progress_bar>();
		bar->value = 0.2f;
		ui.add_child(bar);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};

		frame(0.016f);
		ASSERT_TRUE(std::abs(bar->displayed() - 0.2f) < 0.001f);   // first frame snapped

		bar->value = 0.9f;
		frame(0.016f);
		ASSERT_TRUE(bar->displayed() > 0.2f && bar->displayed() < 0.5f);   // moving, not there yet
		for (int i = 0; i < 30; i++)
			frame(0.016f);
		ASSERT_TRUE(std::abs(bar->displayed() - 0.9f) < 0.001f);   // arrived

		bar->animated = false;
		bar->value = 0.3f;
		frame(0.016f);
		ASSERT_TRUE(std::abs(bar->displayed() - 0.3f) < 0.001f);   // off: snaps
	}

	// A popup that is closed fades out first and leaves the tree only when the
	// fade ends; reopening it mid-fade brings the same popup back instead of
	// adding it twice. No GPU needed.
	TEST(Core.HAL, GUILayer_PopupFadesOutOnClose)
	{
		THREAD_SCOPE(GUI);

		// Shared-owned: closing and hit testing call get_ptr() on the root.
		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(400.0f, 200.0f);

		auto strip = std::make_shared<GUI::Elements::menu_strip>();
		auto file = strip->add_item("File");
		auto popup = file->get_menu();
		popup->add_item("New");
		ui.add_child(strip);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};

		frame(0.03f);
		file->open_menu();
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		ASSERT_TRUE(popup->get_parent() != nullptr);
		ASSERT_TRUE(popup->layer_opacity == 1.0f);          // faded in

		file->close_menu();
		frame(0.03f);
		ASSERT_TRUE(popup->get_parent() != nullptr);        // still there, fading
		ASSERT_TRUE(popup->layer_opacity < 1.0f && popup->layer_opacity > 0.0f);

		for (int i = 0; i < 8; i++)
			frame(0.03f);
		ASSERT_TRUE(popup->get_parent() == nullptr);        // gone once the fade ended
		ASSERT_TRUE(popup->layer_opacity == 1.0f);          // ready for the next open

		// Reopen while it is fading out: the same popup comes back, once.
		file->open_menu();
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		file->close_menu();
		frame(0.03f);
		file->open_menu();
		for (int i = 0; i < 12; i++)
			frame(0.03f);
		ASSERT_TRUE(popup->get_parent() != nullptr);
		ASSERT_TRUE(popup->layer_opacity == 1.0f);

		// Drawn once, not twice: it was never added a second time.
		ui.process_ui(0.03f);
		FrameGraph::Graph graph;
		ui.create_graph(graph);
		int copies = 0;
		for (const auto& e : graph.get_context<GUI::UIContext>().draw_infos)
			if (e.elem == popup.get() && e.before)
				copies++;
		ASSERT_TRUE(copies == 1);

		// With fades off a close is immediate.
		ui.fade_enabled = false;
		file->close_menu();
		frame(0.03f);
		ASSERT_TRUE(popup->get_parent() == nullptr);
	}

	// A toast over a hard red|blue seam: its translucent background shows the
	// backdrop blurred (blue has crossed the seam under it, which a plain
	// translucent plate would not do), and its opacity fades the whole toast as one
	// group (blur, strip and text), 0 = nothing, 1 = fully there.
	TEST(Core.HAL, GUIToast_FrostedAndGroupFade)
	{
		THREAD_SCOPE(GUI);
		constexpr uint W = 512, H = 128;
		constexpr float seam = 256.0f;

		GUI::user_interface ui;
		ui.size = vec2((float)W, (float)H);

		auto add_half = [&](float4 color, float x, float width)
		{
			auto half = std::make_shared<GUI::Elements::colored_rect>();
			half->color       = color;
			half->docking     = GUI::dock::NONE;
			half->width_size  = GUI::size_type::FIXED;
			half->height_size = GUI::size_type::FIXED;
			half->size        = vec2(width, (float)H);
			half->pos         = vec2(x, 0.0f);
			ui.add_child(half);
		};
		add_half(float4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f, seam);
		add_half(float4(0.0f, 0.0f, 1.0f, 1.0f), seam, W - seam);

		auto toast = std::make_shared<GUI::Elements::toast_widget>("Saved", GUI::Elements::toast_kind::info, 4.0f);
		ui.add_child(toast);

		toast->set_opacity(1.0f);
		const auto full = render_ui(ui, W, H);

		const rect tb = toast->get_render_bounds();
		ASSERT_TRUE(tb.x < seam - 40.0f && tb.x + tb.w > seam + 40.0f);        // it spans the seam
		const uint y = (uint)(tb.y + 3.0f);                                    // above the text

		ASSERT_TRUE(full.at((uint)seam - 1, y, 2) > 0.05f);                    // blue under it, blurred across the seam
		ASSERT_TRUE(full.at((uint)seam - 1, y, 2) < 0.4f);                     // as a smear, not a swap
		ASSERT_TRUE(full.at((uint)seam - 17, y, 2) < 0.01f);                   // 16 px away the blur is gone
		ASSERT_TRUE(full.at(100, y, 0) > 0.2f && full.at(100, y, 0) < 0.6f);   // red darkened by the plate

		toast->set_opacity(0.5f);
		const auto half = render_ui(ui, W, H);
		toast->set_opacity(0.0f);
		const auto none = render_ui(ui, W, H);

		const float r_full = full.at(100, y, 0), r_half = half.at(100, y, 0), r_none = none.at(100, y, 0);
		ASSERT_TRUE(r_none > 0.98f);                  // opacity 0: only the background is left
		ASSERT_TRUE(r_none > r_half && r_half > r_full);
		ASSERT_TRUE(std::abs(r_half - 0.5f * (r_full + r_none)) < 0.03f);   // exactly the midpoint
		ASSERT_TRUE(none.at((uint)seam - 1, y, 2) < 0.01f);                 // and no blur either
	}

	// While a toast slides, everything that makes it up has to move with it: the
	// blur (drawn from the toast's rectangle) and the plate, strip and text (its
	// children). With the children left behind, the plate stays at the resting
	// place while the blur moves off: left of the slid toast there is a darkened
	// plate where only the plain background belongs.
	TEST(Core.HAL, GUIToast_SlideMovesChildrenAndBlurTogether)
	{
		THREAD_SCOPE(GUI);
		constexpr uint W = 512, H = 128;

		GUI::user_interface ui;
		ui.size = vec2((float)W, (float)H);

		auto background = std::make_shared<GUI::Elements::colored_rect>();
		background->color       = float4(1.0f, 0.0f, 0.0f, 1.0f);
		background->docking     = GUI::dock::NONE;
		background->width_size  = GUI::size_type::FIXED;
		background->height_size = GUI::size_type::FIXED;
		background->size        = vec2((float)W, (float)H);
		ui.add_child(background);

		auto toast = std::make_shared<GUI::Elements::toast_widget>("Saved", GUI::Elements::toast_kind::info, 4.0f);
		ui.add_child(toast);
		toast->set_opacity(1.0f);

		// Rendered once at rest first, as a real toast is: the children have been placed.
		render_ui(ui, W, H);

		toast->set_offset(80.0f);
		const auto img = render_ui(ui, W, H);

		const rect tb = toast->get_render_bounds();
		ASSERT_TRUE(tb.x > 70.0f);                                   // it did move
		const uint y = (uint)(tb.y + 3.0f);

		ASSERT_TRUE(img.at((uint)(tb.x - 10.0f), y, 0) > 0.98f);     // left of it: plain background, no plate
		ASSERT_TRUE(img.at((uint)(tb.x + 10.0f), y, 0) < 0.6f);      // inside it: the dark plate
		ASSERT_TRUE(img.at(20, y, 0) > 0.98f);                       // and nothing left behind at the old place

		// The accent strip is a child: it must have moved with the toast, not stayed behind.
		ASSERT_TRUE(img.at((uint)(tb.x + 1.0f), y, 2) > 0.7f);       // strip blue at the toast's left edge
		ASSERT_TRUE(img.at((uint)(tb.x + 1.0f), y, 0) < 0.4f);
	}

	// The same through the real flow: toast_manager puts the toast in its stack and
	// slides it from think(). Mid-slide the blurred area must be exactly the
	// toast's plate: on every side, one pixel inside is plate-over-blur and one
	// pixel outside is the plain background.
	TEST(Core.HAL, GUIToast_ManagerSlideBlurMatchesPlate)
	{
		THREAD_SCOPE(GUI);
		constexpr uint W = 640, H = 200;
		constexpr float seam = 320.0f;

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2((float)W, (float)H);

		auto add_half = [&](float4 color, float x, float width)
		{
			auto half = std::make_shared<GUI::Elements::colored_rect>();
			half->color       = color;
			half->docking     = GUI::dock::NONE;
			half->width_size  = GUI::size_type::FIXED;
			half->height_size = GUI::size_type::FIXED;
			half->size        = vec2(width, (float)H);
			half->pos         = vec2(x, 0.0f);
			ui.add_child(half);
		};
		add_half(float4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f, seam);
		add_half(float4(0.0f, 0.0f, 1.0f, 1.0f), seam, W - seam);

		ui.add_child(std::make_shared<GUI::Elements::toast_manager>());
		GUI::Elements::toast_manager::show("Mid-slide", GUI::Elements::toast_kind::info, 4.0f);

		ui.process_ui(0.05f);
		for (int i = 0; i < 2; i++)
			ui.process_ui(0.1f);        // ~0.25 s in: sliding and fading in

		// Find the toast, then draw the frame exactly as it is (render_ui runs one more
		// zero-length process_ui, so nothing moves under it).
		GUI::Elements::toast_widget* toast = nullptr;
		{
			FrameGraph::Graph graph;
			ui.create_graph(graph);
			for (const auto& e : graph.get_context<GUI::UIContext>().draw_infos)
				if (auto* t = dynamic_cast<GUI::Elements::toast_widget*>(e.elem))
					toast = t;
		}
		ASSERT_TRUE(toast != nullptr);
		ASSERT_TRUE(toast->margin->left > 1.0f);                  // really mid-slide
		toast->layer_opacity = 1.0f;                              // read the colours, not the fade

		const auto img = render_ui(ui, W, H);
		const rect tb = toast->get_render_bounds();

		const uint mid_y = (uint)(tb.y + 3.0f);                   // above the text
		const uint left = (uint)tb.x, right = (uint)(tb.x + tb.w) - 1;
		const uint top = (uint)tb.y, bottom = (uint)(tb.y + tb.h) - 1;
		const uint mid_x = (uint)(tb.x + 20.0f);

		// Red background (left of the seam): plain = 1.0, plate over it = darkened.
		ASSERT_TRUE(img.at(left + 1, mid_y, 0) < 0.8f);           // inside, left edge: plate
		ASSERT_TRUE(img.at(left - 2, mid_y, 0) > 0.98f);          // outside, left edge: plain
		ASSERT_TRUE(img.at(mid_x, top + 1, 0) < 0.8f);            // inside, top edge: plate
		ASSERT_TRUE(img.at(mid_x, top - 2, 0) > 0.98f);           // outside, top edge: plain
		ASSERT_TRUE(img.at(mid_x, bottom - 1, 0) < 0.8f);         // inside, bottom edge: plate
		ASSERT_TRUE(img.at(mid_x, bottom + 2, 0) > 0.98f);        // outside, bottom edge: plain
		ASSERT_TRUE(img.at(right - 1, mid_y, 2) > 0.1f);          // inside, right edge: plate over blue
		ASSERT_TRUE(img.at(std::min(right + 2, W - 1), mid_y, 2) > 0.98f);   // outside (or past the window): plain blue
	}

	// The app's frame overlap: while a frame is drawn, the GUI thread has already
	// laid the next one out. A sliding toast is then drawn at bounds one step ahead
	// of the clip rectangles captured with the draw list. Whatever that does, the
	// blurred area and the plate must still agree on where the toast is.
	TEST(Core.HAL, GUIToast_SlideWithTheNextFrameAlreadyLaidOut)
	{
		THREAD_SCOPE(GUI);
		constexpr uint W = 640, H = 200;

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2((float)W, (float)H);

		auto background = std::make_shared<GUI::Elements::colored_rect>();
		background->color       = float4(1.0f, 0.0f, 0.0f, 1.0f);
		background->docking     = GUI::dock::NONE;
		background->width_size  = GUI::size_type::FIXED;
		background->height_size = GUI::size_type::FIXED;
		background->size        = vec2((float)W, (float)H);
		ui.add_child(background);

		ui.add_child(std::make_shared<GUI::Elements::toast_manager>());
		GUI::Elements::toast_manager::show("Mid-slide", GUI::Elements::toast_kind::info, 4.0f);

		ui.process_ui(0.05f);
		for (int i = 0; i < 2; i++)
			ui.process_ui(0.1f);                       // sliding

		GUI::Elements::toast_widget* toast = nullptr;
		{
			FrameGraph::Graph graph;
			ui.create_graph(graph);
			for (const auto& e : graph.get_context<GUI::UIContext>().draw_infos)
				if (auto* t = dynamic_cast<GUI::Elements::toast_widget*>(e.elem))
					toast = t;
		}
		ASSERT_TRUE(toast != nullptr);

		// Draw it with the next step (0.1 s further along) already laid out.
		toast->layer_opacity = 1.0f;
		const rect collected = toast->get_render_bounds();   // what the draw list is built from
		const auto img = render_ui(ui, W, H, nullptr, nullptr, 0.1f);
		ASSERT_TRUE(std::abs(toast->get_render_bounds().x - collected.x) > 1.0f);   // really one step ahead

		const uint mid_y = (uint)(collected.y + 3.0f);
		const uint left = (uint)collected.x, right = (uint)(collected.x + collected.w) - 1;

		// Where the toast was collected, the plate darkens the blurred red everywhere;
		// blur drawn at the newer bounds would leave bright strips there or outside.
		for (uint x = left + 1; x < std::min(right, W - 1); x += 8)
			ASSERT_TRUE(img.at(x, mid_y, 0) < 0.8f);
		ASSERT_TRUE(img.at(left - 2, mid_y, 0) > 0.98f);
	}

	// The slide: an offset moves the toast sideways without changing its width.
	TEST(Core.HAL, GUIToast_SlideMovesWithoutResizing)
	{
		THREAD_SCOPE(GUI);

		GUI::user_interface ui;
		ui.size = vec2(400.0f, 100.0f);

		auto toast = std::make_shared<GUI::Elements::toast_widget>("Saved", GUI::Elements::toast_kind::info, 4.0f);
		ui.add_child(toast);

		ui.process_ui(0.0f);
		const rect rest = toast->get_render_bounds();

		toast->set_offset(36.0f);
		ui.process_ui(0.0f);
		const rect slid = toast->get_render_bounds();
		ASSERT_TRUE(std::abs((slid.x - rest.x) - 36.0f) < 1.0f);
		ASSERT_TRUE(std::abs(slid.w - rest.w) < 1.0f);

		toast->set_offset(0.0f);
		ui.process_ui(0.0f);
		ASSERT_TRUE(std::abs(toast->get_render_bounds().x - rest.x) < 1.0f);
	}

	// The manager's lifecycle: a toast arrives sliding and fading in, settles,
	// slides and fades out near the end of its life, then is gone.
	TEST(Core.HAL, GUIToast_ManagerAnimates)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(600.0f, 300.0f);
		ui.add_child(std::make_shared<GUI::Elements::toast_manager>());

		auto find_toast = [&]() -> GUI::Elements::toast_widget*
		{
			FrameGraph::Graph graph;
			ui.create_graph(graph);
			for (const auto& e : graph.get_context<GUI::UIContext>().draw_infos)
				if (auto* t = dynamic_cast<GUI::Elements::toast_widget*>(e.elem))
					return t;
			return nullptr;
		};

		GUI::Elements::toast_manager::show("Hello", GUI::Elements::toast_kind::info, 1.0f);

		ui.process_ui(0.05f);
		auto* toast = find_toast();
		ASSERT_TRUE(toast != nullptr);
		ASSERT_TRUE(toast->layer_opacity < 0.5f);          // arriving: faint,
		ASSERT_TRUE(toast->margin->left > 20.0f);          // and still off to the right

		for (int i = 0; i < 4; i++)                         // ~0.45 s in: settled (fades are 0.4 s, life 1 s)
			ui.process_ui(0.1f);
		toast = find_toast();
		ASSERT_TRUE(toast != nullptr);
		ASSERT_TRUE(toast->layer_opacity == 1.0f);
		ASSERT_TRUE(std::abs(float(toast->margin->left)) < 0.5f);

		for (int i = 0; i < 3; i++)                         // ~0.75 s in: leaving
			ui.process_ui(0.1f);
		toast = find_toast();
		ASSERT_TRUE(toast != nullptr);
		ASSERT_TRUE(toast->layer_opacity < 1.0f);
		ASSERT_TRUE(toast->margin->left > 1.0f);

		for (int i = 0; i < 6; i++)                         // past its life
			ui.process_ui(0.1f);
		ASSERT_TRUE(find_toast() == nullptr);
	}

	// Context menus open through menu_list::self_open and close through
	// close_menus: both fade now, like submenus do.
	TEST(Core.HAL, GUILayer_ContextMenuFades)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(400.0f, 200.0f);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};

		auto menu = std::make_shared<GUI::Elements::menu_list>();
		menu->add_item("Cut");
		menu->add_item("Copy");

		frame(0.03f);
		menu->self_open(&ui);
		frame(0.03f);
		ASSERT_TRUE(menu->layer_opacity > 0.0f && menu->layer_opacity < 1.0f);   // fading in
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		ASSERT_TRUE(menu->get_parent() != nullptr);
		ASSERT_TRUE(menu->layer_opacity == 1.0f);

		ui.close_menus();   // a press elsewhere
		frame(0.03f);
		ASSERT_TRUE(menu->get_parent() != nullptr);                               // still fading out
		ASSERT_TRUE(menu->layer_opacity < 1.0f);
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		ASSERT_TRUE(menu->get_parent() == nullptr);

		// Opened again right after a close: the same popup comes back.
		menu->self_open(&ui);
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		ui.close_menus();
		frame(0.03f);
		menu->self_open(&ui);
		for (int i = 0; i < 12; i++)
			frame(0.03f);
		ASSERT_TRUE(menu->get_parent() != nullptr);
		ASSERT_TRUE(menu->layer_opacity == 1.0f);
	}

	// A combo box dropdown is a menu_list too: it fades in on the click that opens
	// it and fades out when closed, and the click that closes-and-reopens it (the
	// press closes, the release opens) keeps one dropdown, not two.
	TEST(Core.HAL, GUILayer_ComboDropdownFades)
	{
		THREAD_SCOPE(GUI);

		auto ui_owner = std::make_shared<GUI::user_interface>();
		auto& ui = *ui_owner;
		ui.size = vec2(400.0f, 200.0f);

		auto frame = [&](float dt)
		{
			ui.process_ui(dt);
			FrameGraph::Graph graph;
			ui.create_graph(graph);
		};

		auto combo = std::make_shared<GUI::Elements::combo_box>();
		combo->add_item("Alpha");
		combo->add_item("Beta");
		combo->docking = GUI::dock::NONE;
		combo->width_size = GUI::size_type::FIXED;
		combo->size = vec2(160.0f, 25.0f);
		combo->pos = vec2(20.0f, 20.0f);
		ui.add_child(combo);
		frame(0.03f);
		ASSERT_TRUE(!combo->is_menu_open());

		combo->on_mouse_action(mouse_action::UP, mouse_button::LEFT, vec2(10.0f, 10.0f));   // the click
		frame(0.03f);
		ASSERT_TRUE(combo->is_menu_open());
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		ASSERT_TRUE(combo->is_menu_open());

		combo->close_menus();
		frame(0.03f);
		ASSERT_TRUE(combo->is_menu_open());           // fading out, not gone yet
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		ASSERT_TRUE(!combo->is_menu_open());          // gone once the fade ended

		// Press closes, release opens: the dropdown that was closing comes back.
		combo->on_mouse_action(mouse_action::UP, mouse_button::LEFT, vec2(10.0f, 10.0f));
		for (int i = 0; i < 8; i++)
			frame(0.03f);
		combo->close_menus();
		frame(0.03f);
		combo->on_mouse_action(mouse_action::UP, mouse_button::LEFT, vec2(10.0f, 10.0f));
		for (int i = 0; i < 12; i++)
			frame(0.03f);
		ASSERT_TRUE(combo->is_menu_open());
	}
}
