namespace GUI
{

	template <class T>
	struct UISignature : public T
	{
		using T::T;

		typename T::template ConstBuffer	<0, Render::ShaderVisibility::VERTEX, 0>									vertex_const_buffer = this;
		typename T::template ConstBuffer	<1, Render::ShaderVisibility::PIXEL, 0>										pixel_const_buffer = this;
		typename T::template Table			<2, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 5>	pixel_strange = this;
		typename T::template Table			<3, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SAMPLER, 0, 3>pixel_samples = this;
		typename T::template ConstBuffer	<4, Render::ShaderVisibility::GEOMETRY, 0>									geometry_data = this;
		typename T::template SRV			<5, Render::ShaderVisibility::VERTEX, 0, 1>									vertex_buffer = this;
		typename T::template SRV			<6, Render::ShaderVisibility::VERTEX, 1, 1>									instance_buffer = this;
		typename T::template Table			<7, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 1, 1>	pixel_textures = this;
	};


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


            Render::PipelineState::ptr state;
            std::vector<D3D12_VERTEX_BUFFER_VIEW> vblist;
            std::vector<Vertex> vertexes;

        public:
            using ptr = s_ptr<SimpleRect>;
            SimpleRect(Render::RootSignature::ptr root);



            void draw(Render::context& c, float4 color, rect r);
    };

    class Renderer 
    {
       
            Render::HandleTable sampler_table;

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

            void draw(Render::context& c, GUI::Texture& item, rect r);
            void draw(Render::context& c, Render::PipelineState::ptr& state, rect r);
            Renderer();


            void set(Render::context& c);
    };
}

