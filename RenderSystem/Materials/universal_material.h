#pragma once
namespace materials
{

	struct render_pass
	{
		Render::pixel_shader::ptr ps_shader;
		Render::domain_shader::ptr ds_shader;
		Render::hull_shader::ptr hs_shader;

		void init_pipeline_id()
		{
			pipeline_id = 2166136261U;
			pipeline_id = HashIterate(ps_shader ? ps_shader->id : 0, pipeline_id);
			pipeline_id = HashIterate(ds_shader ? ds_shader->id : 0, pipeline_id);
			pipeline_id = HashIterate(hs_shader ? hs_shader->id : 0, pipeline_id);
		}


		D3D_PRIMITIVE_TOPOLOGY get_topology();


		size_t get_pipeline_id();

	private:
		size_t pipeline_id = 0;


		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int file_version)
		{
			ar& NVP(ps_shader);
			ar& NVP(ds_shader);
			ar& NVP(hs_shader);
		//	ar& NVP(pipeline_id);
			if (Archive::is_loading::value)
				init_pipeline_id();
		}
	};

	/*struct TextureSRVParamsRef
	{
		Asset::ref asset;
		bool to_linear = false;
		TextureSRVParamsRef():asset(nullptr)
		{

		}
		TextureSRVParamsRef(Asset::ref asset, bool to_linear):asset(asset), to_linear(to_linear)
		{}
	private:


		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int file_version)
		{
			ar& NVP(asset);
			ar& NVP(to_linear);
		}
	};*/
    class universal_material : public MaterialAsset, ::FlowGraph::graph_listener
    {
            LEAK_TEST(universal_material)
            /*----------------------------------------------------------*/
            virtual	void on_register(::FlowGraph::window*)override;
            virtual	void on_remove(::FlowGraph::window*)override;

            virtual	void on_add_input(::FlowGraph::parameter*)override;
            virtual	void on_remove_input(::FlowGraph::parameter*)override;

            virtual	void on_add_output(::FlowGraph::parameter*)override;
            virtual	void on_remove_output(::FlowGraph::parameter*)override;

            virtual	void on_link(::FlowGraph::parameter*, ::FlowGraph::parameter*)override;
            virtual	void on_unlink(::FlowGraph::parameter*, ::FlowGraph::parameter*) override;
            /*----------------------------------------------------------*/


            std::vector<Render::Handle> texture_handles;
            Render::HandleTable uav_handles;


            std::vector<TextureSRVParams::ptr> textures;
            //   std::vector<TiledTexture::ref> tiled_textures;


			Render::HandleTable texture_table;
			std::vector<render_pass> passes;

			enum PASS_TYPE
			{
				DEFERRED,
				DEPTH,
				VOXEL_DYNAMIC,
				VOXEL_STATIC,
				COUNTER
			};

           std::vector<Uniform::ptr> tess_uniforms;

            std::vector<Events::prop_handler> handlers;
            Render::GPUBuffer::ptr pixel_buffer;
            Render::GPUBuffer::ptr tess_buffer;

            BinaryData<MaterialGraph> graph;

            MaterialContext::ptr context;
            BinaryAsset::ref include_file;

            bool need_update_uniforms = false;

            std::mutex m;
            virtual void on_asset_change(std::shared_ptr<Asset> asset) override;
            virtual void compile() override;
            universal_material();
            bool need_regenerate_material = false;

			void generate_texture_handles();
        public:
            using ptr = s_ptr<universal_material>;
			std::vector<Uniform::ptr> ps_uniforms;
			Events::Event<void> on_change;

            universal_material(MaterialGraph::ptr graph);

			void test();

            void update(MeshRenderContext::ptr&);
        
			void place_textures(std::vector<Render::Handle>& handles);

			std::vector<Render::Handle> get_texture_handles()
			{
				return texture_handles;
			}
			Render::HandleTable get_texture_srvs()
			{
				return texture_table;
			}

			render_pass & get_normal_pass();
			size_t  get_id();
			render_pass & get_pass(RENDER_TYPE render_type, MESH_TYPE type);
			render_pass & get_pass(MESH_TYPE type, MeshRenderContext::ptr& context);
            void set_pipeline_states(MESH_TYPE type, MeshRenderContext::ptr& context,Render::PipelineStateDesc& desc);
            MaterialGraph::ptr get_graph();
			Render::GPUBuffer::ptr get_pixel_buffer();
            /*   Render::HandleTable get_texture_handles()
               {
                   return texture_handles;
               }*/
            void on_graph_changed();
            void generate_material();

            virtual void set(MESH_TYPE type, MeshRenderContext::ptr&) override;
			virtual void set(RENDER_TYPE render_type, MESH_TYPE type, Render::PipelineStateDesc &pipeline) override;
        private:
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int file_version);

    };
	
}

BOOST_CLASS_EXPORT_KEY(materials::universal_material);