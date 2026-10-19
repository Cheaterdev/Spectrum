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

	struct BackdropBlurParams
	{
		float  radius     = 18.0f;                          // blur radius, px at UI scale 1
		float4 tint       = { 0.02f, 0.02f, 0.03f, 0.35f }; // sRGB rgb, alpha = strength
		float  saturation = 1.25f;
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

		// Draws what is behind `r` (everything already drawn into the target
		// this frame), blurred, tinted and desaturated, over `r`. `slot` is the
		// element's per-frame snapshot slot (user_interface::backdrop_counter):
		// elements recorded into different command lists must not share one.
		// Falls back to a flat translucent tint when the target can't be read
		// (tests, previews rendering the UI somewhere else).
		void draw_backdrop_blur(base::Context& c, uint32_t slot, rect r, const BackdropBlurParams& params);

		// Drops the snapshot textures; call before the device goes away.
		static void reset_backdrop();

		Renderer();

		// Without the Skin's area/virtual/container textures (and the asset system loading them): for hosts that
		// draw elements which bring their own textures. draw_area/_container/_virtual are not usable then.
		struct no_skin_t {};
		explicit Renderer(no_skin_t) {}

		void start();
		void set(base::Context& c);
	};
}

