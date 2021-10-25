#pragma once
#include "GUI/GUI.h"
import Buffer;
namespace GUI
{

    class NinePatch
    {
        using Vertex = Table::vertex_input::CB;

			Render::HandleTable sampler_table;

           Render::IndexBuffer::ptr index_buffer;
		   std::vector<Vertex> vertexes;

		   std::vector<Render::Handle> textures_handles;
		   std::vector<Render::Handle> samplers_handles;

		   std::vector<D3D12_VERTEX_BUFFER_VIEW> vblist;
		   Render::PipelineState::ptr current_state;
        public:
            int counter = 0;
            using ptr = s_ptr<NinePatch>;
            NinePatch();
            void draw(Render::context& c, Render::PipelineState::ptr pipeline_state, rect r);
            void draw(Render::context& c, GUI::Texture& item, rect r);
            void draw(Render::context& c, GUI::Texture& item, rect r, Render::PipelineState::ptr pipeline_state);
			void flush(Render::context& c);
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



            void draw(Render::context& c, float4 color, rect r);
    };

    class Renderer 
    {
       
          //  Render::HandleTable sampler_table;

            NinePatch::ptr nine_patch;
            SimpleRect::ptr simple_rect;

            Texture area_tex;
            Texture virtual_tex;
            Texture container_tex;


        public:

       //     Render::RootSignature::ptr root_signature;

            using ptr = s_ptr<Renderer>;
            using wptr = w_ptr<Renderer>;


            void draw_area(base::ptr obj, Render::context& c);
            void draw_container(base::ptr obj, Render::context& c);
            void draw_virtual(base::ptr obj, Render::context& c);
            void draw_color(Render::context& c, float4 color, rect r);
			void flush(Render::context& c);

            void draw(Render::context& c, GUI::Texture& item, rect r);
            void draw(Render::context& c, Render::PipelineState::ptr state, rect r);
            Renderer();

            void start()
            {
                nine_patch->counter = 0;
            }
            void set(Render::context& c);
    };
}

