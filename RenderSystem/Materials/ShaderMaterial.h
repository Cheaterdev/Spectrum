class ShaderMaterial: public materials::material, public AssetHolder
{
        //    BinaryAsset::ref file;
        Render::pixel_shader::ptr shader;

    public:
        using ptr = s_ptr<ShaderMaterial>;
		ShaderMaterial();
		ShaderMaterial(std::string file_name, std::string entry);

		virtual void on_asset_change(std::shared_ptr<Asset> asset) override;

        virtual void set(MESH_TYPE type, MeshRenderContext::ptr&) override;
		virtual void set(RENDER_TYPE render_type, MESH_TYPE type, Render::PipelineStateDesc &pipeline) override;
    private:
        friend class boost::serialization::access;

        template<class Archive>
        void save(Archive& ar, const unsigned int) const;

        template<class Archive>
        void load(Archive& ar, const unsigned int);
        template<class Archive>
        void serialize(Archive& ar, const unsigned int file_version);

};

BOOST_CLASS_EXPORT_KEY(ShaderMaterial);