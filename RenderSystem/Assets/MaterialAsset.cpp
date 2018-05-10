#include "pch.h"
BOOST_CLASS_EXPORT_IMPLEMENT(AssetReference<MaterialAsset>);


BOOST_CLASS_EXPORT(MaterialAsset);

BOOST_CLASS_EXPORT_IMPLEMENT(materials::material);


template void AssetReference<MaterialAsset>::serialize(serialization_oarchive& arch, const unsigned int version);
template void AssetReference<MaterialAsset>::serialize(serialization_iarchive& arch, const unsigned int version);

/*

class TextureAssetRenderer : public Singleton<TextureAssetRenderer>
{
	friend class Singleton<TextureAssetRenderer>;

	DX11::pipeline_state::ptr state;
	DX11::sampler_state::ptr sampler;
	std::vector<D3D11_VIEWPORT> vps;

	TextureAssetRenderer()
	{
		state = DX11::pipeline_state::create();
		state->p_shader = DX11::pixel_shader::get_resource({ "shaders\\texture_drawer.hlsl", "PS", 0, {} });
		state->v_shader = DX11::vertex_shader::get_resource({ "shaders\\texture_drawer.hlsl", "VS", 0, {} });
		D3D11_SAMPLER_DESC sampDesc;
		sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.MipLODBias = 0.0f;
		sampDesc.MaxAnisotropy = 16;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		sampler = DX11::sampler_state::get_resource(sampDesc);
		D3D11_BLEND_DESC b_desc;
		ZeroMemory(&b_desc, sizeof(b_desc));

		for (int i = 0; i < 4; ++i)
		{
			b_desc.RenderTarget[i].BlendEnable = FALSE;
			b_desc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			b_desc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			b_desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			b_desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
			b_desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			b_desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			b_desc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}

		state->blend = DX11::blend_state::get_resource(b_desc);
		D3D11_DEPTH_STENCIL_DESC depth_desc;
		depth_desc.DepthEnable = false;
		depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depth_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depth_desc.StencilEnable = false;
		depth_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		depth_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		depth_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depth_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		depth_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depth_desc.BackFace = depth_desc.FrontFace;
		state->depth_stencil = DX11::depth_stencil_state::get_resource(depth_desc);
		state->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		D3D11_RASTERIZER_DESC raster_desc;
		ZeroMemory(&raster_desc, sizeof(raster_desc));
		raster_desc.CullMode = D3D11_CULL_NONE;
		raster_desc.FillMode = D3D11_FILL_SOLID;
		raster_desc.ScissorEnable = false;
		state->rasterizer = DX11::rasterizer_state::get_resource(raster_desc);
		vps.resize(1);
		vps[0].TopLeftX = 0;
		vps[0].TopLeftY = 0;
		vps[0].MinDepth = 0.0f;
		vps[0].MaxDepth = 1.0f;
	}
	virtual ~TextureAssetRenderer() {}
public:
	void render(TextureAsset* asset, DX11::Texture2D::ptr target, DX11::Context &c)
	{
		auto orig = c.get_rts();
		auto orig_vps = c.get_viewports();
		DX11::RenderTargetState my_state;
		vps[0].Width = target->get_desc().Width;
		vps[0].Height = target->get_desc().Height;
		c.clear_rtv(target->get_render_target(), { 0, 0, 0, 0 });
		my_state[0] = target->get_render_target();
		c.get_rts() = my_state;
		c.get_shader_state< DX11::pixel_shader>().sampler_state[0] = sampler;
		c.get_shader_state< DX11::pixel_shader>().shader_resource_state[0] = asset->get_texture()->get_shader_view();
		c.set(state);
		c.set_viewports(vps);
		c.draw(4, 0);
		c.get_rts() = orig;
		c.set_viewports(orig_vps);
		c.flush();
	}
};

*/
//////////////////////////////////////////////////////////////////////////
Asset_Type MaterialAsset::get_type()
{
    return Asset_Type::MATERIAL;
}
/*
materials::material::ptr MaterialAsset::get_material()
{
    return mat;
}
*/
MaterialAsset::MaterialAsset(materials::material::ptr m)
{
    // mat = m;
    // name = convert(m->name);
    //  name = "i'm a material yo";
    // mat->asset = this;
    mark_changed();
}

void MaterialAsset::update_preview(Render::Texture::ptr preview)
{
    AssetRenderer::get().draw(get_ptr<MaterialAsset>(), preview);
    //   c.generate_mipmaps(preview->get_shader_view());
    mark_changed();
}

