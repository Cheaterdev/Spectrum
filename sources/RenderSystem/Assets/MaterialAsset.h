#pragma once
#include "Assets/TextureAsset.h"
#include "Context/Context.h"

class MaterialAsset;
namespace materials
{

	class Pipeline
	{
		UINT id;
	public:
		using ptr = std::shared_ptr<Pipeline>;
        unsigned int hash;
		Pipeline(UINT id) :id(id)
		{

		}
        Pipeline() = default;
        virtual ~Pipeline() = default;
		
		virtual void set(RENDER_TYPE render_type, MESH_TYPE type, Graphics::PipelineStateDesc& pipeline) = 0;

		UINT get_id() { return id; }
	private:

        SERIALIZE()
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
            virtual void set(MESH_TYPE type, MeshRenderContext::ptr&) {};
            virtual void set(RENDER_TYPE render_type, MESH_TYPE type, Graphics::PipelineStateDesc& pipeline) {};
            virtual ~material() = default;
            virtual void compile() {};

            size_t get_pipeline_id()
            {
                return pipeline_id;
            }
            SERIALIZE()
            {
                ar& NVP(pipeline_id);
            }

    };

}

// REGISTER_TYPE(materials::material);
class MaterialAsset : public Asset, public materials::material
{
        LEAK_TEST(MaterialAsset)

    public:
        using ptr = s_ptr<MaterialAsset>;
        using ref = AssetReference<MaterialAsset>;

        MaterialAsset(materials::material::ptr);

        virtual Asset_Type get_type() override;
        virtual void update_preview(Graphics::Texture::ptr preview);
        virtual ~MaterialAsset() = default;
    protected:
        MaterialAsset() = default;
    private:

        SERIALIZE()
        {
            SAVE_PARENT(Asset);
            SAVE_PARENT(materials::material);
        }

};

//CEREAL_REGISTER_TYPE2(MaterialAsset);