export module Graphics:MaterialAsset;


import Core;
import HAL;
import :Texture;

import :Context;
import :Asset;

class MaterialAsset;
export namespace materials
{

	class Pipeline
	{
		uint id;
	public:
		using ptr = std::shared_ptr<Pipeline>;
        unsigned int hash;
		Pipeline(uint id);
        Pipeline() = default;
        virtual ~Pipeline() = default;

		virtual void set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion) = 0;

		// Whether this pipeline's material graph actually drives opacity (see
		// universal_material::is_transparent()). VSM's alpha-cutout depth pass
		// only needs to know this per pipeline, not the material graph itself
		// -- default false so non-material pipelines (PipelineSimple, used for
		// UI/preview-style draws) don't need to care.
		virtual bool is_transparent() const { return false; }

		// Non-null only for a PipelinePasses whose is_transparent() is true
		// (see PipelinePasses' constructor) -- a virtual accessor here, not a
		// cast at the VSM.cpp call site, since only pipelines that already
		// pass is_transparent() ever get asked for this.
		virtual PSOS::VSMDepthDrawMaterial::ptr get_vsm_depth_draw() const { return nullptr; }

        uint get_id();
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
            virtual void set(MESH_TYPE type, MeshRenderContext::ptr&);
            virtual void set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion);
            virtual ~material() = default;
            virtual void compile();
            virtual void update();

            size_t get_pipeline_id();
            SERIALIZE()
            {
                ar& NVP(pipeline_id);
            }

    };

}

// REGISTER_TYPE(materials::material);
export class MaterialAsset : public Asset, public materials::material
{
    public:
        using ptr = s_ptr<MaterialAsset>;
        using ref = AssetReference<MaterialAsset>;

        MaterialAsset(materials::material::ptr);

        virtual Asset_Type get_type() override;
        virtual void update_preview(HAL::Texture::ptr preview);
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