#include "pch_render.h"
#include "TextureAsset.h"
#include "Helpers/MipMapGeneration.h"
import Tasks;
#include "Context/Context.h"


REGISTER_TYPE(TextureAsset);

TextureAssetRenderer::TextureAssetRenderer()
{

}
TextureAssetRenderer::~TextureAssetRenderer() {}

void TextureAssetRenderer::render(TextureAsset* asset, HAL::Texture::ptr target, HAL::CommandList::ptr c)
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

HAL::Texture::ptr TextureAsset::get_texture()
{
	return texture;
}

TextureAsset::TextureAsset(std::filesystem::path file_name)
{
	auto task = TaskInfoManager::get().create_task(file_name.generic_wstring());
	texture = HAL::Texture::get_resource(HAL::texure_header(file_name, true));

	if (!texture)
		texture = HAL::Texture::null;

	name = file_name.filename().wstring();
	mark_changed();
}
void TextureAsset::try_register()
{
	if ((texture && texture != HAL::Texture::null))
		Asset::try_register();
}

inline void TextureAsset::reload_resource()
{
	texture->reload_resource();
}

TextureAsset::TextureAsset()
{
}

void TextureAsset::update_preview(HAL::Texture::ptr preview)
{
	if (!preview || !preview->is_rt())
		preview.reset(new HAL::Texture(HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { 256, 256 }, 1, 6, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess)));

	auto list = (HAL::FrameResourceManager::get().begin_frame()->start_list("TextureAsset"));

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
