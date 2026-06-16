export module Test.FrameGraph;

#define TEST_MODULE_ID FG

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import GUI;
import FrameGraph;

// PassDefault<T> specialisation declarations — bodies live in RenderSystem.
// Must be included AFTER the module imports so Passes::* and FrameGraph::*
// types are already in scope (mirrors how VulkanTest/main.cpp does it).
#include "RenderSystem/FrameGraph/autogen/pass_defaults.h"

export namespace Test
{
	// Exercises the complete UIPipeline path end-to-end:
	//   GUI::user_interface layout → UIPipeline::add_passes (Profiler + 16×UI_Render)
	//   → create_graph → Graph::setup/compile/render/commit → golden image comparison.
	//
	// UI: dark background fill, a 64px red top band, and a green fill area.
	// Rendered into a 512×256 B8G8R8A8_UNORM off-screen texture.
	TEST(Core.HAL, FrameGraph_UIPipeline)
	{
		THREAD_SCOPE(GUI);
		auto& device = RenderSystem::get().device();
		constexpr uint W = 512, H = 256;

		auto rt = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::B8G8R8A8_UNORM, {W, H}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		// ---- Build the UI hierarchy -------------------------------------------
		GUI::user_interface ui;
		ui.size = vec2((float)W, (float)H);

		// Full-screen dark-blue background (dock NONE, MATCH_PARENT)
		{
			auto bg = std::make_shared<GUI::Elements::colored_rect>();
			bg->color       = float4(0.10f, 0.15f, 0.25f, 1.0f);
			bg->width_size  = GUI::size_type::MATCH_PARENT;
			bg->height_size = GUI::size_type::MATCH_PARENT;
			ui.add_child(bg);
		}
		// Red top band, 64px tall
		{
			auto top = std::make_shared<GUI::Elements::colored_rect>();
			top->color       = float4(0.70f, 0.15f, 0.10f, 1.0f);
			top->docking     = GUI::dock::TOP;
			top->height_size = GUI::size_type::FIXED;
			top->width_size  = GUI::size_type::MATCH_PARENT;
			top->size        = vec2(0.0f, 64.0f);
			ui.add_child(top);
		}
		// Green fill area (remaining space after the top band)
		{
			auto fill = std::make_shared<GUI::Elements::colored_rect>();
			fill->color   = float4(0.10f, 0.55f, 0.25f, 1.0f);
			fill->docking = GUI::dock::FILL;
			ui.add_child(fill);
		}

		ui.process_ui(0.0f);

		// ---- Drive the FrameGraph --------------------------------------------
		FrameGraph::Graph graph;
		Pipelines::UIPipeline pipeline;

		graph.start_new_frame();
		graph.builder.pass_texture("swapchain", rt);

		pipeline.add_passes(graph);
		ui.create_graph(graph);   // populates UIContext::draw_infos

		graph.setup();
		graph.compile(0);
		graph.render();
		auto fence = graph.commit_command_lists();
		fence.wait();

		ASSERT_TEXTURE(rt.get(), "fg_uipipeline");

		graph.reset();
	}
}
