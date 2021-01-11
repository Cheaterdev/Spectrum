#pragma once
#include "Asset.h"
#include "Context/Context.h"

class MaterialAsset;
namespace materials
{

	class Pipeline
	{
		UINT id;
	public:
		using ptr = std::shared_ptr<Pipeline>;
        _Big_uint128 hash;
		Pipeline(UINT id) :id(id)
		{

		}
        Pipeline() = default;
		virtual void set(RENDER_TYPE render_type, MESH_TYPE type, DX12::PipelineStateDesc& pipeline) = 0;

		UINT get_id() { return id; }
	private:

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int file_version)
		{
			ar& NVP(hash);
            ar& NVP(id);
		}
    
    };


    class material
    {
        protected:
            size_t pipeline_id = 0;
        public:
            using ptr = s_ptr<material>;
            virtual void set(MESH_TYPE type,MeshRenderContext::ptr&) = 0;
			virtual void set(RENDER_TYPE render_type, MESH_TYPE type, DX12::PipelineStateDesc &pipeline) = 0;
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
                ar& NVP(pipeline_id);
            }

    };

}

BOOST_CLASS_EXPORT_KEY(materials::material);
class MaterialAsset : public Asset, public materials::material
{
        LEAK_TEST(MaterialAsset)

    public:
        using ptr = s_ptr<MaterialAsset>;
        using ref = AssetReference<MaterialAsset>;

        MaterialAsset(materials::material::ptr);

        virtual Asset_Type get_type() override;
        virtual void update_preview(DX12::Texture::ptr preview);
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
