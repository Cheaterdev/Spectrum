namespace GUI
{
    /*
    #define ELEMENTS_LIST\
    X(check_box)\
    X(scroll_bar)\
    X(list_element)\
    X(menu_list_element)\
    X(combo_box)\
    X(image)\
    X(edit_text)\
    X(edit_cursor)
    */
#define ELEMENTS_LIST_FLOW

    /*\
    X(component_window)\
    X(canvas)\
    X(link_item)\
    X(comment)

    */
    /*  struct SimpleData2 : public RenderData
      {
          struct v_c_b
          {
              vec2 p1;
              vec2 p2;
          };

          struct p_c_b
          {
              vec4 size;
          };

          BUILD_BUFFERS(SimpleData2);
      };

    */
    class NinePatch
    {
            struct Vertex
            {
                vec2 pos;
                vec2 tc;
            };
			Render::HandleTable sampler_table;

           Render::IndexBuffer::ptr index_buffer;
           Render::PipelineState::ptr state;
		   std::vector<Vertex> vertexes;

		  // std::vector<std::uint32_t> textures_offsets;
		   std::vector<Render::Handle> textures_handles;

		   std::vector<D3D12_VERTEX_BUFFER_VIEW> vblist;
		   Render::PipelineState::ptr current_state;
        public:
            using ptr = s_ptr<NinePatch>;
            NinePatch(Render::RootSignature::ptr root);
            void draw(Render::context& c, Render::PipelineState::ptr& pipeline_state, rect r);
            void draw(Render::context& c, GUI::Texture& item, rect r);
            void draw(Render::context& c, GUI::Texture& item, rect r, Render::PipelineState::ptr& pipeline_state);
			void flush(Render::context& c);
    };

    class SimpleRect
    {
            struct Vertex
            {
                vec2 pos;

            };

            //     Render::VertexBuffer<Vertex>::ptr vertex_buffer;

            Render::PipelineState::ptr state;
            std::vector<D3D12_VERTEX_BUFFER_VIEW> vblist;
            std::vector<Vertex> vertexes;

        public:
            using ptr = s_ptr<SimpleRect>;
            SimpleRect(Render::RootSignature::ptr root);



            void draw(Render::context& c, float4 color, rect r);
    };

    class Renderer //: public renderer_base<base, SimpleData>
    {




            //     Render::PipelineState::ptr color_state;
            Render::HandleTable sampler_table;

            /*

            #define X(x) GUI::Elements::x::renderer::ptr x##_renderer;
            ELEMENTS_LIST
            #undef X
            	*/
#define X(x) GUI::Elements::FlowGraph::x::renderer::ptr flow_##x##_renderer;
            ELEMENTS_LIST_FLOW
#undef X
            NinePatch::ptr nine_patch;
            SimpleRect::ptr simple_rect;

            Texture area_tex;
            Texture virtual_tex;
            Texture container_tex;


        public:

            Render::RootSignature::ptr root_signature;

            using ptr = s_ptr<Renderer>;
            using wptr = w_ptr<Renderer>;


            void draw_area(base::ptr obj, Render::context& c);
            void draw_container(base::ptr obj, Render::context& c);
            void draw_virtual(base::ptr obj, Render::context& c);
            void draw_color(Render::context& c, float4 color, rect r);
			void flush(Render::context& c);
            /*
            #define X(x) 	void draw(Elements::x * obj, Render::context &c) {x##_renderer->draw(obj, c);}
                        ELEMENTS_LIST
            #undef X
            */
#define X(x) 	void draw(Elements::FlowGraph::x * obj, Render::context &c) {flow_##x##_renderer->draw(obj, c);}
            ELEMENTS_LIST_FLOW
#undef X

            void draw(Render::context& c, GUI::Texture& item, rect r);
            void draw(Render::context& c, Render::PipelineState::ptr& state, rect r);
            Renderer();


            void set(Render::context& c);
    };
}

