export module Graphics:MaterialAsset;


import Core;
import HAL;
import :Texture;

import :Context;
import :Asset;

export class MaterialAsset;   // defined below as exported; every declaration must agree (Clang)
export namespace materials
{

	// A material always builds its mesh-shader pipelines; the vertex-stage GBuffer pipeline (for GPUs without
	// mesh shaders) is built where mesh shaders are missing, and also when an offline tool asks for it so
	// that the asset it saves carries it for such a device.
	struct PipelineOptions
	{
		static bool& vertex_front_end() { static bool v = false; return v; }
	};

	class Pipeline
	{
		uint id;
	public:
		using ptr = std::shared_ptr<Pipeline>;
        unsigned int hash;
		Pipeline(uint id);
        Pipeline() = default;
        virtual ~Pipeline() = default;

		// capture_meshlets: the GBuffer draw records per-meshlet cull results for
		// the debug view (GBufferDraw's CaptureMeshlets permutation).
		// cluster_lod: the GBuffer draw walks the cluster LOD hierarchy; the
		// gather must have handed out lod_draw_commands (GBufferDraw ClusterLod).
		virtual void set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion, bool capture_meshlets, bool cluster_lod) = 0;

		// Per pipeline, not per material: the GPU gather routes draws by
		// pipeline id, so this is what decides which passes a mesh reaches
		// (Masked -> VSM's cutout depth PSO, Translucent -> no raster pass at
		// all). Default Opaque so non-material pipelines (PipelineSimple, used
		// for UI/preview-style draws) don't need to care.
		virtual Meshes::TransparencyMode get_transparency_mode() const { return Meshes::TransparencyMode::Opaque; }

		// Non-null only for a Masked PipelinePasses (see its constructor).
		virtual PSOS::Shadows::VSM::VSMDepthDrawMaterial::ptr get_vsm_depth_draw() const { return nullptr; }

		// False while one of its shaders is still compiling: draws with it are skipped.
		virtual bool is_built() const { return true; }

		// Binds the GBuffer pipeline whose geometry stage is a vertex shader over a direct indexed draw (see
		// PipelineOptions). Only pipelines that built one say so.
		virtual void set_vertex_front_end(HAL::GraphicsContext& graphics) {}
		virtual bool has_vertex_front_end() const { return false; }

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

            // Whether a draw with this material would render now, not be skipped
            // because its pipeline is still compiling.
            virtual bool is_ready_to_draw() const { return true; }

            // Whether the pipeline exists yet (a loaded material makes it on its first update),
            // so a caller knows waiting for is_ready_to_draw() can end.
            virtual bool has_pipeline() const { return true; }

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