module Graphics:BinaryAsset;
import RenderSystem;



import :MipMapGenerator;
import TextEngine;
import Core;
void removeme2() // TODO: VS issue - make dummy unused func to compile entire cpp =[
{
	auto res = Serializer::deserialize<std::string>("");
}

REGISTER_TYPE(BinaryAsset);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Asset, BinaryAsset);
CEREAL_REGISTER_DYNAMIC_INIT(BinaryAsset);
CEREAL_FORCE_REGISTER(BinaryAsset);
CEREAL_FORCE_REGISTER_RELATION(Asset, BinaryAsset);

Asset_Type BinaryAsset::get_type()
{
	return Asset_Type::BINARY;
}

std::string BinaryAsset::get_data()
{
	return data;
}

BinaryAsset::BinaryAsset(std::wstring file_name)
{
	tester = std::make_shared<bool>();
	auto file = FileSystem::get().get_file(to_path(file_name));

	if (file)
	{
		data = file->load_all();
		file_depends.add_depend(file);
	}

	this->file_name = file_name;
	name = file_name.substr(file_name.find_last_of(L"\\") + 1);
	mark_changed();
	FileSystem::get().get_provider<native_file_provider>()->on_change(to_path(file_name), [this]()
		{
			reload_resource();
		});
}
void BinaryAsset::try_register()
{
	if (data.size())
		Asset::try_register();
}

void BinaryAsset::reload_resource()
{
	Log::get() << "reloading " << file_name << Log::endl;

	if (!file_depends.need_update())
		return;

	file_depends.clear();
	auto file = FileSystem::get().get_file(to_path(file_name));

	if (file)
	{
		data = file->load_all();
		file_depends.add_depend((file));
		mark_changed();
	}

	mark_contents_changed(); // bumps version + notifies consumers
}

BinaryAsset::BinaryAsset()
{
	tester = std::make_shared<bool>();

}

BinaryAsset::~BinaryAsset()
{
}

void BinaryAsset::update_preview(HAL::Texture::ptr preview)
{
	if (!preview || !preview->is_rt())
		preview.reset(new HAL::Texture(RenderSystem::get().device(), HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { 256, 256 }, 1, 6, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess)));

	auto list = (RenderSystem::get().device().get_frame_manager().begin_frame()->start_list(L"BinaryAsset", HAL::CommandListType::DIRECT, true));

	// The glyphs are uploaded in this same list, ahead of the draw. Text is
	// laid out unwrapped and clipped to the preview.
	Text::Layout text;
	Text::Engine::get().build(data, { 14, Text::Weight::Light }, text);
	Text::Engine::get().upload(list);

	{
				RT::Frame::SingleColor rt;
				rt.GetColor() =preview->texture_2d().renderTarget;
				list->get_graphics().set_rtv(rt, HAL::RTOptions::Default| HAL::RTOptions::ClearColor);
	}

	// raw_output: an R8G8B8A8_UNORM preview texture, not the scRGB swapchain.
	Text::Engine::get().draw(list, text, { 0, 0 }, float4(1, 1, 1, 1), sizer{ 0, 0, 256, 256 }, { 256, 256 }, true);


	MipMapGenerator::get().generate(list->get_compute(), preview->texture_2d());
	//TextureAssetRenderer::get().render(this, preview, list);
	list->execute();
	//
//   c.generate_mipmaps(preview->get_shader_view());
	mark_changed();
}

