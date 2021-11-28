#pragma once

#include "Assets/MaterialAsset.h"

class ShaderMaterial: public materials::material, public AssetHolder
{
  
    materials::Pipeline::ptr pipeline;
    public:
        using ptr = s_ptr<ShaderMaterial>;
        ShaderMaterial() = default;
		ShaderMaterial(std::string file_name, std::string entry);

		virtual void on_asset_change(std::shared_ptr<Asset> asset) override;
		materials::Pipeline::ptr get_pipeline() { return pipeline; }

        virtual void set(MESH_TYPE type, MeshRenderContext::ptr&) override;
		virtual void set(RENDER_TYPE render_type, MESH_TYPE type, Render::PipelineStateDesc &pipeline) override;
    private:
        SERIALIZE() {
            SAVE_PARENT(materials::material);
            SAVE_PARENT(AssetHolder);
        }

};

// CEREAL_REGISTER_TYPE(ShaderMaterial);