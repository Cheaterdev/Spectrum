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
	std::lock_guard<std::mutex> g(texture_mutex);

	// Loaded as tiles: the whole texture is made from the texels the file keeps beside them.
	if ((!texture || texture == HAL::Texture::null) && tiles && !tiles->path.empty() && RenderSystem::is_good())
	{
		FileDataStorage storage(tiles->path);
		auto data = std::make_shared<HAL::texture_data>();
		if (storage.has("texdata") && storage.get("texdata", *data))
		{
			texture = HAL::Texture::create(RenderSystem::get().device(), data);
			if (texture && texture->resource)
				texture->resource->set_name(convert(name));
		}
		if (!texture)
			texture = HAL::Texture::null;
	}

	return texture;
}

bool TextureAsset::has_texture_2d()
{
	if (streamed)
		return true;

	if (tiles)
		return tiles->array_size == 1 && tiles->depth <= 1;

	auto whole = get_texture();
	return whole && whole->texture_2d();
}

void TextureAsset::reset_streamed()
{
	streamed.reset();
	stream_checked = false;
	// The texture changed, so what the file holds is not it any more.
	tiles.reset();
}

void TextureAsset::set_streamable(bool value)
{
	if (streamable == value)
		return;

	get_texture();   // the whole texture has to exist before the streamed one goes
	streamable = value;
	streamed.reset();
	stream_checked = false;
	mark_changed();
}

// Below this a texture has too few tiles to be worth streaming.
static constexpr uint MinStreamedSize = 1024;

bool TextureAsset::can_stream(const HAL::Texture::ptr& candidate) const
{
	if (!streamable || !TextureStreaming::Manager::supported() || !candidate || candidate == HAL::Texture::null || !candidate->resource)
		return false;

	const auto& desc = candidate->get_desc();
	if (!desc.is_texture() || desc.is_virtual() || check(desc.Flags & HAL::ResFlags::RenderTarget))
		return false;

	const auto texture_desc = desc.as_texture();
	return texture_desc.is2D() && texture_desc.ArraySize == 1 && std::max(texture_desc.Dimensions.x, texture_desc.Dimensions.y) >= MinStreamedSize
		&& std::max(texture_desc.Dimensions.x, texture_desc.Dimensions.y) > static_cast<uint>(std::max(TextureStreaming::Manager::resident_size(), 0));
}

HAL::Texture::ptr TextureAsset::get_render_texture()
{
	if (!streamable)
		return get_texture();

	if (streamed)
		return streamed->texture;

	if (!stream_checked)
	{
		stream_checked = true;

		auto whole = get_texture();
		if (can_stream(whole))
		{
			auto data = whole->get_data();
			auto candidate = TextureStreaming::Manager::get().create(data, name);

			// Worth it only if some standard mip is left to stream above the always-resident ones.
			if (candidate->resident_floor_mip() >= 1)
			{
				streamed = candidate;
				const auto texture_desc = whole->get_desc().as_texture();
				Log::get() << L"Streaming texture " << name << L" (" << texture_desc.Dimensions.x << L"x" << texture_desc.Dimensions.y
					<< L", " << candidate->standard_mips() << L" streamed mips)" << Log::endl;
			}
		}
	}

	return streamed ? streamed->texture : get_texture();
}

void TextureAsset::before_save()
{
	saving_tiled = false;
	saving_data.reset();

	if (!streamable)
	{
		get_texture();   // saved whole, even if the asset was loaded as tiles
		return;
	}

	std::lock_guard<std::mutex> g(texture_mutex);

	if (tiles && !tiles->path.empty())
	{
		// Loaded from a file that is about to be written over: its tiles and texels are read first.
		saving_data = std::make_shared<HAL::texture_data>();
		FileDataStorage storage(tiles->path);
		if (!tiles->read_all() || !storage.has("texdata") || !storage.get("texdata", *saving_data))
			throw std::runtime_error("texture asset " + convert(name) + ": cannot read its tiles back to save them");
		saving_tiled = true;
	}
	else if (can_stream(texture))
	{
		saving_data = texture->get_data();
		tiles = TextureStreaming::Manager::get().bake(saving_data);
		saving_tiled = true;
	}
}

void TextureAsset::save_partitions(FileDataStorage& storage)
{
	Flags flags;
	flags.streamable = streamable;
	flags.tiled = saving_tiled;
	storage.put("texflags", flags);

	if (saving_tiled)
	{
		storage.put("textiles", *tiles);
		storage.put("texdata", *saving_data);
	}
}

void TextureAsset::after_save(FileDataStorage& storage)
{
	std::lock_guard<std::mutex> g(texture_mutex);

	if (saving_tiled)
	{
		// From here the tiles are ranges of the file just written, not bytes in memory.
		auto fresh = std::make_shared<TextureStreaming::TileFile>();
		if (storage.get("textiles", *fresh))
		{
			if (!tiles->path.empty())
				tiles->relocate(*fresh);   // the streamed texture reads through this very object
			else
				tiles = fresh;
		}
		tiles->tile_bytes.clear();
		tiles->tile_bytes.shrink_to_fit();
		tiles->packed_bytes.clear();
		tiles->packed_bytes.shrink_to_fit();
	}
	else
	{
		tiles.reset();
	}

	saving_data.reset();
	saving_tiled = false;
}

void TextureAsset::load_partitions(FileDataStorage& storage, UniversalContext* context)
{
	if (!storage.has("texflags"))
		return;

	Flags flags;
	storage.get("texflags", flags, context);
	streamable = flags.streamable;

	if (flags.tiled && storage.has("textiles"))
	{
		auto loaded = std::make_shared<TextureStreaming::TileFile>();
		if (!storage.get("textiles", *loaded, context))
			return;

		tiles = loaded;

		// Only the roots go to memory; a device without tiled textures makes the whole texture on demand (get_texture).
		if (streamable && TextureStreaming::Manager::supported())
		{
			streamed = TextureStreaming::Manager::get().create(tiles, name);
			stream_checked = true;
		}
	}
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
	reset_streamed();
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
	auto new_texture = get_texture()->compress();

	if (new_texture)
	{
		texture = new_texture;
		reset_streamed();
		mark_contents_changed();
	}

	return true;
}
