module Graphics:TextureAsset;
import RenderSystem;


import Core;
import :MipMapGenerator;


REGISTER_TYPE(TextureAsset);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Asset, TextureAsset);
CEREAL_REGISTER_DYNAMIC_INIT(TextureAsset);
CEREAL_FORCE_REGISTER(TextureAsset);
CEREAL_FORCE_REGISTER_RELATION(Asset, TextureAsset);

TextureAssetRenderer::TextureAssetRenderer()
{

}
TextureAssetRenderer::~TextureAssetRenderer() {}

void TextureAssetRenderer::render(TextureAsset* asset, HAL::Texture::ptr target, HAL::CommandList::ptr c)
{
	if (!asset->get_texture()->texture_2d()) return;
	MipMapGenerator::get().copy_texture_2d_slow(c->get_graphics(), target, asset->get_texture()->texture_2d());
	MipMapGenerator::get().generate(c->get_compute(), target->texture_2d());
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

TextureAsset::TextureAsset(HAL::texture_data::ptr data, std::wstring name)
{
	auto task = TaskInfoManager::get().create_task(name);

	if (data)
		texture = HAL::Texture::create(RenderSystem::get().device(), data);

	if (!texture)
		texture = HAL::Texture::null;

	this->name = name;
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
		preview.reset(new HAL::Texture(RenderSystem::get().device(), HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { 256, 256 }, 1, 6, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess)));

	auto list = (RenderSystem::get().device().get_frame_manager().begin_frame()->start_list(L"TextureAsset", HAL::CommandListType::DIRECT, true));

	TextureAssetRenderer::get().render(this, preview, list);
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
