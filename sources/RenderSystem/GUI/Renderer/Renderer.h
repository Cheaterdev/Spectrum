#pragma once
#include "GUI/GUI.h"
import Graphics;
namespace GUI
{

	class NinePatch
	{
		using Vertex = Table::vertex_input;

		HAL::HandleTable sampler_table;

		HAL::IndexBuffer::ptr index_buffer;
		std::vector<Vertex> vertexes;

		std::vector<HLSL::Texture2D<float4>> textures_handles;
		std::vector<HAL::Handle> samplers_handles;

		std::vector<D3D12_VERTEX_BUFFER_VIEW> vblist;
		HAL::PipelineState::ptr current_state;
	public:
		int counter = 0;
		using ptr = s_ptr<NinePatch>;
		NinePatch();
		void draw(Graphics::context& c, HAL::PipelineState::ptr pipeline_state, rect r);
		void draw(Graphics::context& c, GUI::Texture& item, rect r);
		void draw(Graphics::context& c, GUI::Texture& item, rect r, HAL::PipelineState::ptr pipeline_state);
		void flush(Graphics::context& c);
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



		void draw(Graphics::context& c, float4 color, rect r);
	};

	class Renderer
	{

		NinePatch::ptr nine_patch;
		SimpleRect::ptr simple_rect;

		Texture area_tex;
		Texture virtual_tex;
		Texture container_tex;


	public:

		//     HAL::RootSignature::ptr root_signature;

		using ptr = s_ptr<Renderer>;
		using wptr = w_ptr<Renderer>;


		void draw_area(base::ptr obj, Graphics::context& c);
		void draw_container(base::ptr obj, Graphics::context& c);
		void draw_virtual(base::ptr obj, Graphics::context& c);
		void draw_color(Graphics::context& c, float4 color, rect r);
		void flush(Graphics::context& c);

		void draw(Graphics::context& c, GUI::Texture& item, rect r);
		void draw(Graphics::context& c, HAL::PipelineState::ptr state, rect r);
		Renderer();

		void start()
		{
			nine_patch->counter = 0;
		}
		void set(Graphics::context& c);
	};
}

