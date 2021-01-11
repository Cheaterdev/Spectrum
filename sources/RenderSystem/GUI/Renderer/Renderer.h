#pragma once
#include <slots.h>
#include "DX12/Buffer.h"
#include "DX12/PipelineState.h"
#include "../GUI.h"
namespace GUI
{

    class NinePatch
    {
        using Vertex = Table::vertex_input::CB;

			DX12::HandleTable sampler_table;

           DX12::IndexBuffer::ptr index_buffer;
		   std::vector<Vertex> vertexes;

		   std::vector<DX12::Handle> textures_handles;
		   std::vector<DX12::Handle> samplers_handles;

		   std::vector<D3D12_VERTEX_BUFFER_VIEW> vblist;
		   DX12::PipelineState::ptr current_state;
        public:
            int counter = 0;
            using ptr = s_ptr<NinePatch>;
            NinePatch();
            void draw(DX12::context& c, DX12::PipelineState::ptr pipeline_state, rect r);
            void draw(DX12::context& c, GUI::Texture& item, rect r);
            void draw(DX12::context& c, GUI::Texture& item, rect r, DX12::PipelineState::ptr pipeline_state);
			void flush(DX12::context& c);
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



            void draw(DX12::context& c, float4 color, rect r);
    };

    class Renderer 
    {
       
          //  DX12::HandleTable sampler_table;

            NinePatch::ptr nine_patch;
            SimpleRect::ptr simple_rect;

            Texture area_tex;
            Texture virtual_tex;
            Texture container_tex;


        public:

       //     DX12::RootSignature::ptr root_signature;

            using ptr = s_ptr<Renderer>;
            using wptr = w_ptr<Renderer>;


            void draw_area(base::ptr obj, DX12::context& c);
            void draw_container(base::ptr obj, DX12::context& c);
            void draw_virtual(base::ptr obj, DX12::context& c);
            void draw_color(DX12::context& c, float4 color, rect r);
			void flush(DX12::context& c);

            void draw(DX12::context& c, GUI::Texture& item, rect r);
            void draw(DX12::context& c, DX12::PipelineState::ptr state, rect r);
            Renderer();

            void start()
            {
                nine_patch->counter = 0;
            }
            void set(DX12::context& c);
    };
}

