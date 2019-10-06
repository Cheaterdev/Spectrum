#include "pch.h"


/*   { Render::DescriptorRange::SRV, Render::ShaderVisibility::PIXEL, 0, 1 },
		{ Render::DescriptorRange::SAMPLER, Render::ShaderVisibility::PIXEL, 0, 1 },
*/

template <class T>
struct TextureRenderer : public T
{
	using T::T;

	typename T::template Table			<0, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 1>	texture = this;
	typename T::template Table			<2, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SAMPLER, 0, 1>	sampler = this;

};





BOOST_CLASS_EXPORT(TextureAsset);
BOOST_CLASS_EXPORT_IMPLEMENT(AssetReference<TextureAsset>);


template void AssetReference<TextureAsset>::serialize(serialization_oarchive& arch, const unsigned int version);
template void AssetReference<TextureAsset>::serialize(serialization_iarchive& arch, const unsigned int version);

TextureAssetRenderer::TextureAssetRenderer()
{
    Render::PipelineStateDesc state_desc;
	state_desc.root_signature = TextureRenderer<SignatureCreator>().create_root();
    state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\texture_drawer.hlsl", "PS", 0, {} });
    state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\texture_drawer.hlsl", "VS", 0, {} });
    state_desc.rtv.rtv_formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
    state.reset(new Render::PipelineState(state_desc));
    /*	state = DX11::pipeline_state::create();
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
    */
    sampler_table = Render::DescriptorHeapManager::get().get_samplers()->create_table(1);
    D3D12_SAMPLER_DESC wrapSamplerDesc = {};
    wrapSamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
    wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    wrapSamplerDesc.MinLOD = 0;
    wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    wrapSamplerDesc.MipLODBias = 0.0f;
    wrapSamplerDesc.MaxAnisotropy = 1;
    wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] = wrapSamplerDesc.BorderColor[3] = 0;
    Render::Device::get().create_sampler(wrapSamplerDesc, sampler_table[0].cpu);
    vps.TopLeftX = 0;
    vps.TopLeftY = 0;
    vps.MinDepth = 0.0f;
    vps.MaxDepth = 1.0f;
}
TextureAssetRenderer::~TextureAssetRenderer() {}

void TextureAssetRenderer::render(TextureAsset* asset, Render::Texture::ptr target, Render::CommandList::ptr c)
{
    if (!asset->get_texture()->texture_2d()) return;

  //  c->get_graphics().set_heaps(Render::DescriptorHeapManager::get().get_csu(), Render::DescriptorHeapManager::get().get_samplers
	c->set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().get_samplers());
	c->transition(target, Render::ResourceState::RENDER_TARGET);

  //  target->change_state(c, Render::ResourceState::PIXEL_SHADER_RESOURCE, Render::ResourceState::RENDER_TARGET);
    c->get_native_list()->OMSetRenderTargets(1, &target->texture_2d()->get_rtv().cpu, FALSE, nullptr);
    c->get_graphics().set_pipeline(state);
	Render::Viewport vps;

    vps.Width = static_cast<float>(target->get_desc().Width);
    vps.Height = static_cast<float>(target->get_desc().Height);
    c->get_graphics().set_pipeline(state);

	TextureRenderer<Signature> shader_data(&c->get_graphics());
    c->get_graphics().set_viewports({ vps });
    sizer_long s = { 0, 0, vps.Width , vps.Height };
    c->get_graphics().set_scissors({ s });
	shader_data.texture[0] = asset->get_texture()->texture_2d()->get_srv();
	shader_data.sampler[0] = sampler_table;
    c->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    c->get_graphics().draw(4, 0);
    MipMapGenerator::get().generate(c->get_compute(), target);

	c->transition(target, Render::ResourceState::PIXEL_SHADER_RESOURCE);
    /*auto orig = c.get_rts();
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
    c.flush();*/
}


//////////////////////////////////////////////////////////////////////////
Asset_Type TextureAsset::get_type()
{
    return Asset_Type::TEXTURE;
}

Render::Texture::ptr TextureAsset::get_texture()
{
    return texture;
}

TextureAsset::TextureAsset(boost::filesystem::path file_name)
{
    auto task = TaskInfoManager::get().create_task(file_name.generic_wstring());
    texture = Render::Texture::get_resource(Render::texure_header(file_name, true));

    if (!texture)
        texture = Render::Texture::null;

    name = file_name.filename().wstring();
    mark_changed();
}
void TextureAsset::try_register()
{
    if ((texture && texture != Render::Texture::null))
        Asset::try_register();
}

inline void TextureAsset::reload_resource()
{
	texture->reload_resource();
}

TextureAsset::TextureAsset()
{
}

void TextureAsset::update_preview(Render::Texture::ptr preview)
{
    if (!preview || !preview->is_rt())
        preview.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 256, 256, 1, 6, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));

    Render::CommandList::ptr list(new Render::CommandList(Render::CommandListType::DIRECT));
    list->begin("TextureAsset");
    TextureAssetRenderer::get().render(this, preview, list);
    list->end();
    list->execute();
    //
//   c.generate_mipmaps(preview->get_shader_view());
    mark_changed();
}

bool TextureAsset::compress()
{

	auto task = TaskInfoManager::get().create_task(std::wstring(L"compressing") + name);
	Log::get() << std::wstring(L"compressing") + name << Log::endl;
	auto new_texture = texture->compress();

	if (new_texture)
	{
		texture = new_texture;
		mark_contents_changed();
	}

	return true;
}
