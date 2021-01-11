#include "pch.h"
#include "TextureAsset.h"
#include "Log/Tasks.h"
#include "Helpers/MipMapGeneration.h"

BOOST_CLASS_EXPORT(TextureAsset);
BOOST_CLASS_EXPORT_IMPLEMENT(AssetReference<TextureAsset>);


template void AssetReference<TextureAsset>::serialize(serialization_oarchive& arch, const unsigned int version);
template void AssetReference<TextureAsset>::serialize(serialization_iarchive& arch, const unsigned int version);

TextureAssetRenderer::TextureAssetRenderer()
{

}
TextureAssetRenderer::~TextureAssetRenderer() {}

void TextureAssetRenderer::render(TextureAsset* asset, DX12::Texture::ptr target, DX12::CommandList::ptr c)
{
    if (!asset->get_texture()->texture_2d()) return;
    MipMapGenerator::get().copy_texture_2d_slow(c->get_graphics(), target, asset->get_texture());
    MipMapGenerator::get().generate(c->get_compute(), target);
}


//////////////////////////////////////////////////////////////////////////
Asset_Type TextureAsset::get_type()
{
    return Asset_Type::TEXTURE;
}

DX12::Texture::ptr TextureAsset::get_texture()
{
    return texture;
}

TextureAsset::TextureAsset(std::filesystem::path file_name)
{
    auto task = TaskInfoManager::get().create_task(file_name.generic_wstring());
    texture = DX12::Texture::get_resource(DX12::texure_header(file_name, true));

    if (!texture)
        texture = DX12::Texture::null;

    name = file_name.filename().wstring();
    mark_changed();
}
void TextureAsset::try_register()
{
    if ((texture && texture != DX12::Texture::null))
        Asset::try_register();
}

inline void TextureAsset::reload_resource()
{
	texture->reload_resource();
}

TextureAsset::TextureAsset()
{
}

void TextureAsset::update_preview(DX12::Texture::ptr preview)
{
    if (!preview || !preview->is_rt())
        preview.reset(new DX12::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 256, 256, 1, 6, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));

    auto list =  FrameResourceManager::get().begin_frame()->start_list("TextureAsset");

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
