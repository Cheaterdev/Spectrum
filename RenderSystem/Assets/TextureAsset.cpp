#include "pch.h"

BOOST_CLASS_EXPORT(TextureAsset);
BOOST_CLASS_EXPORT_IMPLEMENT(AssetReference<TextureAsset>);


template void AssetReference<TextureAsset>::serialize(serialization_oarchive& arch, const unsigned int version);
template void AssetReference<TextureAsset>::serialize(serialization_iarchive& arch, const unsigned int version);

TextureAssetRenderer::TextureAssetRenderer()
{
    Render::PipelineStateDesc state_desc;
    state_desc.root_signature = get_Signature(Layouts::DefaultLayout);
    state_desc.pixel = Render::pixel_shader::get_resource({ "shaders\\texture_drawer.hlsl", "PS", 0, {} });
    state_desc.vertex = Render::vertex_shader::get_resource({ "shaders\\texture_drawer.hlsl", "VS", 0, {} });
    state_desc.rtv.rtv_formats = { DXGI_FORMAT_R16G16B16A16_FLOAT };
    state.reset(new Render::PipelineState(state_desc));
  
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
	//c->set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().get_samplers());
	c->transition(target, Render::ResourceState::RENDER_TARGET);

  //  target->change_state(c, Render::ResourceState::PIXEL_SHADER_RESOURCE, Render::ResourceState::RENDER_TARGET);
    c->get_native_list()->OMSetRenderTargets(1, &target->texture_2d()->get_rtv().cpu, FALSE, nullptr);
    c->get_graphics().set_pipeline(state);
	Render::Viewport vps;

    vps.Width = static_cast<float>(target->get_desc().Width);
    vps.Height = static_cast<float>(target->get_desc().Height);
    c->get_graphics().set_pipeline(state);

    c->get_graphics().set_viewports({ vps });
    sizer_long s = { 0, 0, vps.Width , vps.Height };
    c->get_graphics().set_scissors({ s });

    Slots::TextureRenderer data;
    data.GetTexture() = asset->get_texture()->texture_2d()->get_srv();

    data.set(c->get_graphics());
    c->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    c->get_graphics().use_dynamic = false;
    c->get_graphics().draw(4, 0);
    MipMapGenerator::get().generate(c->get_compute(), target);

	c->transition(target, Render::ResourceState::PIXEL_SHADER_RESOURCE);
    c->get_graphics().use_dynamic = true;

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
