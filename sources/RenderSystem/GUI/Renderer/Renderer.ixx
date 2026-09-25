export module GUI:Renderer;

import :Base;
import HAL;
export namespace GUI
{

	class NinePatch
	{
		using Vertex = Table::UI::vertex_input;

		std::vector<Vertex> vertexes;
		std::vector<HLSL::Texture2D<float4>> textures_handles;
		HAL::PipelineState::ptr current_state;
	public:
		static HAL::IndexBuffer index_buffer;
		int counter = 0;
		using ptr = s_ptr<NinePatch>;
		NinePatch();
		void draw(base::Context& c, HAL::PipelineState::ptr pipeline_state, rect r);
		void draw(base::Context& c, GUI::Texture& item, rect r);
		void draw(base::Context& c, GUI::Texture& item, rect r, HAL::PipelineState::ptr pipeline_state, bool solid = false);
		// Solid rect with a horizontal gradient, as one more instance of the
		// batch: selection, caret, gutter and scroll bar rects no longer cost
		// a flush and a draw call each.
		void draw_color(base::Context& c, float4 left, float4 right, rect r);
		void flush(base::Context& c);

		static void reset();
	};

	class Renderer
	{
		NinePatch nine_patch;

		Texture area_tex;
		Texture virtual_tex;
		Texture container_tex;

	public:
		using ptr = s_ptr<Renderer>;
		using wptr = w_ptr<Renderer>;

		void draw_area(base::ptr obj, base::Context& c);
		void draw_container(base::ptr obj, base::Context& c);
		void draw_virtual(base::ptr obj, base::Context& c);
		void draw_color(base::Context& c, float4 color, rect r);
		// Horizontal gradient: left color to right color
		void draw_color(base::Context& c, float4 left, float4 right, rect r);
		void flush(base::Context& c);

		void draw(base::Context& c, GUI::Texture& item, rect r);
		void draw(base::Context& c, HAL::PipelineState::ptr state, rect r);
		Renderer();

		void start();
		void set(base::Context& c);
	};
}

