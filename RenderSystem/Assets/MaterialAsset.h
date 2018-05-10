
class MaterialAsset;
namespace materials
{

    class material//: public AssetHoldable<MaterialAsset>
    {
        protected:
            size_t pipeline_id = 0;
        public:
            //      std::string name;
            using ptr = s_ptr<material>;
            virtual void set(MESH_TYPE type,MeshRenderContext::ptr&) = 0;
			virtual void set(RENDER_TYPE render_type, MESH_TYPE type, Render::PipelineStateDesc &pipeline) = 0;
            virtual ~material() = default;
            virtual void compile() {};
            friend class boost::serialization::access;
            size_t get_pipeline_id()
            {
                return pipeline_id;
            }
            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                //      ar& NVP(name);
                ar& NVP(pipeline_id);
            }

    };

}

BOOST_CLASS_EXPORT_KEY(materials::material);
class MaterialAsset : public Asset, public materials::material
{
        //     materials::material::ptr mat;
        LEAK_TEST(MaterialAsset)

    public:
        using ptr = s_ptr<MaterialAsset>;
        using ref = AssetReference<MaterialAsset>;

        MaterialAsset(materials::material::ptr);

        //   materials::material::ptr get_material();

        virtual Asset_Type get_type() override;
        virtual void update_preview(Render::Texture::ptr preview);
        /*     virtual void set(MeshRenderContext::ptr& c)
             {
                 mat->set(c);
             };

        	*/
        virtual ~MaterialAsset() = default;
    protected:
        MaterialAsset() = default;
    private:


        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<Asset>(*this));
            ar& NVP(boost::serialization::base_object<materials::material>(*this));
        }

};


BOOST_CLASS_EXPORT_KEY(AssetReference<MaterialAsset>);
