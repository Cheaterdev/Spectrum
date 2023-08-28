export module GUI:Renderer;

import :Base;
import HAL;
export namespace GUI
{

	class NinePatch
	{
		using Vertex = Table::vertex_input;
		

		static HAL::IndexBuffer index_buffer;
		std::vector<Vertex> vertexes;

		std::vector<HLSL::Texture2D<float4>> textures_handles;

		HAL::PipelineState::ptr current_state;
	public:
		int counter = 0;
		using ptr = s_ptr<NinePatch>;
		NinePatch();
		void draw(base::Context& c, HAL::PipelineState::ptr pipeline_state, rect r);
		void draw(base::Context& c, GUI::Texture& item, rect r);
		void draw(base::Context& c, GUI::Texture& item, rect r, HAL::PipelineState::ptr pipeline_state);
		void flush(base::Context& c);
	};

	class SimpleRect
	{
		struct Vertex
		{
			vec2 pos;

		};


	public:
		using ptr = s_ptr<SimpleRect>;
		SimpleRect();



		void draw(base::Context& c, float4 color, rect r);
	};

	class Renderer
	{

		NinePatch nine_patch;
		SimpleRect simple_rect;

		Texture area_tex;
		Texture virtual_tex;
		Texture container_tex;


	public:

		//     HAL::RootSignature::ptr root_signature;

		using ptr = s_ptr<Renderer>;
		using wptr = w_ptr<Renderer>;


		void draw_area(base::ptr obj, base::Context& c);
		void draw_container(base::ptr obj, base::Context& c);
		void draw_virtual(base::ptr obj, base::Context& c);
		void draw_color(base::Context& c, float4 color, rect r);
		void flush(base::Context& c);

		void draw(base::Context& c, GUI::Texture& item, rect r);
		void draw(base::Context& c, HAL::PipelineState::ptr state, rect r);
		Renderer();

		void start()
		{
			nine_patch.counter = 0;
		}
		void set(base::Context& c);
	};
}

