export module Graphics:MaterialAsset;
import <RenderSystem.h>;

import Core;
import HAL;

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
		Pipeline(uint id) :id(id)
		{

		}
        Pipeline() = default;
        virtual ~Pipeline() = default;
		
		virtual void set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics) = 0;

        uint get_id() { return id; }
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
            virtual void set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics) {};
            virtual ~material() = default;
            virtual void compile() {};
            virtual void update() {};

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
export class MaterialAsset : public Asset, public materials::material
{
        LEAK_TEST(MaterialAsset)

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