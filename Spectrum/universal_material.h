#pragma once
namespace materials
{
    class universal_material : public material, public AssetHolder
    {
            Render::HandleTable texture_handles;
            std::vector<TextureAsset::ptr> textures;
            Render::pixel_shader::ptr shader;
            MaterialGraph::ptr graph;

            MaterialContext::ptr context;


            virtual void compile() override;
            universal_material() {};
        public:
            using ptr = s_ptr<universal_material>;

            universal_material(MaterialGraph::ptr graph)
            {
                this->graph = graph;
            }

            MaterialGraph::ptr get_graph()
            {
                return graph;
            }


            void on_graph_changed()
            {
                if (!context)
                    context.reset(new MaterialContext);

                context->start(graph.get());
                textures = context->get_textures();
                auto res_shader = Render::pixel_shader::create_from_memory(context->get_result(), "PS", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES);

                if (res_shader)
                    shader = res_shader;

                compile();
            }

            virtual void set(MeshRenderContext::ptr) override;
        private:
            friend class boost::serialization::access;

            template<class Archive>
            void save(Archive& ar, const unsigned int) const;

            template<class Archive>
            void load(Archive& ar, const unsigned int);
            template<class Archive>
            void serialize(Archive& ar, const unsigned int file_version);

    };
}

BOOST_CLASS_EXPORT_KEY(materials::universal_material);